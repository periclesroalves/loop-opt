//Copyright (c) 2006-2009 Princeton University
//All rights reserved.

//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met:
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//    * Neither the name of Princeton University nor the
//      names of its contributors may be used to endorse or promote products
//      derived from this software without specific prior written permission.

//THIS SOFTWARE IS PROVIDED BY PRINCETON UNIVERSITY ``AS IS'' AND ANY
//EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL PRINCETON UNIVERSITY BE LIABLE FOR ANY
//DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.




#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <sys/resource.h>
#include <limits.h>

#ifdef ENABLE_PARSEC_HOOKS
#include <hooks.h>
#endif

using namespace std;

#define MAXNAMESIZE 1024 // max filename length
#define SEED 1
/* increase this to reduce probability of random error */
/* increasing it also ups running time of "speedy" part of the code */
/* SP = 1 seems to be fine */
#define SP 1 // number of repetitions of speedy must be >=1

/* higher ITER --> more likely to get correct # of centers */
/* higher ITER also scales the running time almost linearly */
#define ITER 3 // iterate ITER* k log k times; ITER >= 1

//#define PRINTINFO //comment this out to disable output
//#define PROFILE // comment this out to disable instrumentation code
//#define ENABLE_THREADS  // comment this out to disable threads
//#define INSERT_WASTE //uncomment this to insert waste computation into dist function

#define CACHE_LINE 512 // cache line in byte

/* this structure represents a point */
/* these will be passed around to avoid copying coordinates */
typedef struct {
  float weight;
  float *coord;
  long assign;  /* number of point where this one is assigned */
  float cost;  /* cost of that assignment, weight*distance */
} Point;

/* this is the array of points */
typedef struct {
  long num; /* number of points; may not be N if this is a sample */
  int dim;  /* dimensionality */
  Point *p; /* the array itself */
} Points;

static bool *switch_membership; //whether to switch membership in pgain
static bool* is_center; //whether a point is a center
static int* center_table; //index table of centers

static int nproc; //# of threads

// instrumentation code
#ifdef PROFILE
double time_local_search;
double time_speedy;
double time_select_feasible;
double time_gain;
double time_shuffle;
double time_gain_dist;
double time_gain_init;
#endif 

double gettime() {
  struct timeval t;
  gettimeofday(&t,NULL);
  return (double)t.tv_sec+t.tv_usec*1e-6;
}

int isIdentical(float *i, float *j, int D)
// tells whether two points of D dimensions are identical
{
  int a = 0;
  int equal = 1;

  while (equal && a < D) {
    if (i[a] != j[a]) equal = 0;
    else a++;
  }
  if (equal) return 1;
  else return 0;

}

/* comparator for floating point numbers */
static int floatcomp(const void *i, const void *j)
{
  float a, b;
  a = *(float *)(i);
  b = *(float *)(j);
  if (a > b) return (1);
  if (a < b) return (-1);
  return(0);
}

/* shuffle points into random order */
void shuffle(Points *points)
{
#ifdef PROFILE
  double t1 = gettime();
#endif
  long i, j;
  Point temp;
  for (i=0;i<points->num-1;i++) {
    j=(lrand48()%(points->num - i)) + i;
    temp = points->p[i];
    points->p[i] = points->p[j];
    points->p[j] = temp;
  }
#ifdef PROFILE
  double t2 = gettime();
  time_shuffle += t2-t1;
#endif
}

/* shuffle an array of integers */
void intshuffle(int *intarray, int length)
{
#ifdef PROFILE
  double t1 = gettime();
#endif
  long i, j;
  int temp;
  for (i=0;i<length;i++) {
    j=(lrand48()%(length - i))+i;
    temp = intarray[i];
    intarray[i]=intarray[j];
    intarray[j]=temp;
  }
#ifdef PROFILE
  double t2 = gettime();
  time_shuffle += t2-t1;
#endif
}

#ifdef INSERT_WASTE
double waste(double s )
{
  for( int i =0 ; i< 4; i++ ) {
    s += pow(s,0.78);
  }
  return s;
}
#endif

/* compute Euclidean distance squared between two points */
float dist(Point p1, Point p2, int dim)
{
  int i;
  float result=0.0;
  for (i=0;i<dim;i++)
    result += (p1.coord[i] - p2.coord[i])*(p1.coord[i] - p2.coord[i]);
#ifdef INSERT_WASTE
  double s = waste(result);
  result += s;
  result -= s;
#endif
  return(result);
}

/* run speedy on the points, return total cost of solution */
float pspeedy(Points *points, float z, long *kcenter, int pid)
{
#ifdef PROFILE
  double t1 = gettime();
#endif

  //my block
  long bsize = points->num/nproc;
  long k1 = bsize * pid;
  long k2 = k1 + bsize;
  if( pid == nproc-1 ) k2 = points->num;

  static double totalcost;

  static bool open = false;
  static double* costs; //cost for each thread. 
  static int i;

#ifdef PRINTINFO
  if( pid == 0 ){
  }
#endif

  /* create center at first point, send it to itself */
  for( int k = k1; k < k2; k++ )    {
    float distance = dist(points->p[k],points->p[0],points->dim);
    points->p[k].cost = distance * points->p[k].weight;
    points->p[k].assign=0;
  }

  if( pid==0 )   {
    *kcenter = 1;
    costs = (double*)malloc(sizeof(double)*nproc);
  }
    
  if( pid != 0 ) { // we are not the master threads. we wait until a center is opened.
    while(1) {
      if( i >= points->num ) break;
      for( int k = k1; k < k2; k++ )
	{
	  float distance = dist(points->p[i],points->p[k],points->dim);
	  if( distance*points->p[k].weight < points->p[k].cost )
	    {
	      points->p[k].cost = distance * points->p[k].weight;
	      points->p[k].assign=i;
	    }
	}
    } 
  }
  else  { // I am the master thread. I decide whether to open a center and notify others if so. 
    for(i = 1; i < points->num; i++ )  {
      bool to_open = ((float)lrand48()/(float)INT_MAX)<(points->p[i].cost/z);
      if( to_open )  {
	(*kcenter)++;
	open = true;
	for( int k = k1; k < k2; k++ )  {
	  float distance = dist(points->p[i],points->p[k],points->dim);
	  if( distance*points->p[k].weight < points->p[k].cost )  {
	    points->p[k].cost = distance * points->p[k].weight;
	    points->p[k].assign=i;
	  }
	}
	open = false;
      }
    }
    open = true;
  }
  open = false;
  double mytotal = 0;
  for( int k = k1; k < k2; k++ )  {
    mytotal += points->p[k].cost;
  }
  costs[pid] = mytotal;
  // aggregate costs from each thread
  if( pid == 0 )
    {
      totalcost=z*(*kcenter);
      for( int i = 0; i < nproc; i++ )
	{
	  totalcost += costs[i];
	} 
      free(costs);
    }

#ifdef PRINTINFO
  if( pid == 0 )
    {
    }
#endif

#ifdef PROFILE
  double t2 = gettime();
  if( pid== 0 ) {
    time_speedy += t2 -t1;
  }
#endif
  return(totalcost);
}


/* For a given point x, find the cost of the following operation:
 * -- open a facility at x if there isn't already one there,
 * -- for points y such that the assignment distance of y exceeds dist(y, x),
 *    make y a member of x,
 * -- for facilities y such that reassigning y and all its members to x 
 *    would save cost, realize this closing and reassignment.
 * 
 * If the cost of this operation is negative (i.e., if this entire operation
 * saves cost), perform this operation and return the amount of cost saved;
 * otherwise, do nothing.
 */

/* numcenters will be updated to reflect the new number of centers */
/* z is the facility cost, x is the number of this point in the array 
   points */

double pgain(long x, Points *points, double z, long int *numcenters, int pid)
{
  //  printf("pgain pthread %d begin\n",pid);
#ifdef PROFILE
  double t0 = gettime();
#endif

  //my block
  long bsize = points->num/nproc;
  long k1 = bsize * pid;
  long k2 = k1 + bsize;
  if( pid == nproc-1 ) k2 = points->num;

  int i;
  int number_of_centers_to_close = 0;

  static double *work_mem;
  static double gl_cost_of_opening_x;
  static int gl_number_of_centers_to_close;

  //each thread takes a block of working_mem.
  int stride = *numcenters+2;
  //make stride a multiple of CACHE_LINE
  int cl = CACHE_LINE/sizeof(double);
  if( stride % cl != 0 ) { 
    stride = cl * ( stride / cl + 1);
  }
  int K = stride -2 ; // K==*numcenters
  
  //my own cost of opening x
  double cost_of_opening_x = 0;

  if( pid==0 )    { 
    work_mem = (double*) malloc(stride*(nproc+1)*sizeof(double));
    gl_cost_of_opening_x = 0;
    gl_number_of_centers_to_close = 0;
  }

  /*For each center, we have a *lower* field that indicates 
    how much we will save by closing the center. 
    Each thread has its own copy of the *lower* fields as an array.
    We first build a table to index the positions of the *lower* fields. 
  */

  int count = 0;
  for( int i = k1; i < k2; i++ ) {
    if( is_center[i] ) {
      center_table[i] = count++;
    }
  }
  work_mem[pid*stride] = count;


  if( pid == 0 ) {
    int accum = 0;
    for( int p = 0; p < nproc; p++ ) {
      int tmp = (int)work_mem[p*stride];
      work_mem[p*stride] = accum;
      accum += tmp;
    }
  }


  for( int i = k1; i < k2; i++ ) {
    if( is_center[i] ) {
      center_table[i] += (int)work_mem[pid*stride];
    }
  }

  //now we finish building the table. clear the working memory.
  memset(switch_membership + k1, 0, (k2-k1)*sizeof(bool));
  memset(work_mem+pid*stride, 0, stride*sizeof(double));
  if( pid== 0 ) memset(work_mem+nproc*stride,0,stride*sizeof(double));

#ifdef PROFILE
  double t1 = gettime();
  if( pid == 0 ) time_gain_init += t1-t0;
#endif
  //my *lower* fields
  double* lower = &work_mem[pid*stride];
  //global *lower* fields
  double* gl_lower = &work_mem[nproc*stride];

  for ( i = k1; i < k2; i++ ) {
    float x_cost = dist(points->p[i], points->p[x], points->dim) 
      * points->p[i].weight;
    float current_cost = points->p[i].cost;

    if ( x_cost < current_cost ) {

      // point i would save cost just by switching to x
      // (note that i cannot be a median, 
      // or else dist(p[i], p[x]) would be 0)
      
      switch_membership[i] = 1;
      cost_of_opening_x += x_cost - current_cost;

    } else {

      // cost of assigning i to x is at least current assignment cost of i

      // consider the savings that i's **current** median would realize
      // if we reassigned that median and all its members to x;
      // note we've already accounted for the fact that the median
      // would save z by closing; now we have to subtract from the savings
      // the extra cost of reassigning that median and its members 
      int assign = points->p[i].assign;
      lower[center_table[assign]] += current_cost - x_cost;
    }
  }


#ifdef PROFILE
  double t2 = gettime();
  if( pid==0){
    time_gain_dist += t2 - t1;
  }
#endif

  // at this time, we can calculate the cost of opening a center
  // at x; if it is negative, we'll go through with opening it

  for ( int i = k1; i < k2; i++ ) {
    if( is_center[i] ) {
      double low = z;
      //aggregate from all threads
      for( int p = 0; p < nproc; p++ ) {
	low += work_mem[center_table[i]+p*stride];
      }
      gl_lower[center_table[i]] = low;
      if ( low > 0 ) {
	// i is a median, and
	// if we were to open x (which we still may not) we'd close i

	// note, we'll ignore the following quantity unless we do open x
	++number_of_centers_to_close;  
	cost_of_opening_x -= low;
      }
    }
  }
  //use the rest of working memory to store the following
  work_mem[pid*stride + K] = number_of_centers_to_close;
  work_mem[pid*stride + K+1] = cost_of_opening_x;

  //  printf("thread %d cost complete\n",pid); 

  if( pid==0 ) {
    gl_cost_of_opening_x = z;
    //aggregate
    for( int p = 0; p < nproc; p++ ) {
      gl_number_of_centers_to_close += (int)work_mem[p*stride + K];
      gl_cost_of_opening_x += work_mem[p*stride+K+1];
    }
  }
  // Now, check whether opening x would save cost; if so, do it, and
  // otherwise do nothing

  if ( gl_cost_of_opening_x < 0 ) {
    //  we'd save money by opening x; we'll do it
    for ( int i = k1; i < k2; i++ ) {
      bool close_center = gl_lower[center_table[points->p[i].assign]] > 0 ;
      if ( switch_membership[i] || close_center ) {
	// Either i's median (which may be i itself) is closing,
	// or i is closer to x than to its current median
	points->p[i].cost = points->p[i].weight *
	  dist(points->p[i], points->p[x], points->dim);
	points->p[i].assign = x;
      }
    }
    for( int i = k1; i < k2; i++ ) {
      if( is_center[i] && gl_lower[center_table[i]] > 0 ) {
	is_center[i] = false;
      }
    }
    if( x >= k1 && x < k2 ) {
      is_center[x] = true;
    }
    //    pthread_barrier_wait(barrier);

    if( pid==0 ) {
      *numcenters = *numcenters + 1 - gl_number_of_centers_to_close;
    }
  }
  else {
    if( pid==0 )
      gl_cost_of_opening_x = 0;  // the value we'll return
  }
  if( pid == 0 ) {
    free(work_mem);
    //    free(is_center);
    //    free(switch_membership);
    //    free(proc_cost_of_opening_x);
    //    free(proc_number_of_centers_to_close);
  }

#ifdef PROFILE
  double t3 = gettime();
  if( pid==0 )
  time_gain += t3-t0;
#endif
  return -gl_cost_of_opening_x;
}


/* facility location on the points using local search */
/* z is the facility cost, returns the total cost and # of centers */
/* assumes we are seeded with a reasonable solution */
/* cost should represent this solution's cost */
/* halt if there is < e improvement after iter calls to gain */
/* feasible is an array of numfeasible points which may be centers */

float pFL(Points *points, int *feasible, int numfeasible,
	  float z, long *k, double cost, long iter, float e, 
	  int pid)
{
  long i;
  long x;
  double change;
  long numberOfPoints;

  change = cost;
  /* continue until we run iter iterations without improvement */
  /* stop instead if improvement is less than e */
  while (change/cost > 1.0*e) {
    change = 0.0;
    numberOfPoints = points->num;
    /* randomize order in which centers are considered */

    if( pid == 0 ) {
      intshuffle(feasible, numfeasible);
    }
    for (i=0;i<iter;i++) {
      x = i%numfeasible;
      change += pgain(feasible[x], points, z, k, pid);
    }

    cost -= change;
  }
  return(cost);
}

int selectfeasible_fast(Points *points, int **feasible, int kmin, int pid)
{
#ifdef PROFILE
  double t1 = gettime();
#endif

  int numfeasible = points->num;
  if (numfeasible > (ITER*kmin*log((double)kmin)))
    numfeasible = (int)(ITER*kmin*log((double)kmin));
  *feasible = (int *)malloc(numfeasible*sizeof(int));
  
  float* accumweight;
  float totalweight;

  /* 
     Calcuate my block. 
     For now this routine does not seem to be the bottleneck, so it is not parallelized. 
     When necessary, this can be parallelized by setting k1 and k2 to 
     proper values and calling this routine from all threads ( it is called only
     by thread 0 for now ). 
     Note that when parallelized, the randomization might not be the same and it might
     not be difficult to measure the parallel speed-up for the whole program. 
   */
  //  long bsize = numfeasible;
  long k1 = 0;
  long k2 = numfeasible;

  float w;
  int l,r,k;

  /* not many points, all will be feasible */
  if (numfeasible == points->num) {
    for (int i=k1;i<k2;i++)
      (*feasible)[i] = i;
    return numfeasible;
  }

  accumweight= (float*)malloc(sizeof(float)*points->num);
  accumweight[0] = points->p[0].weight;
  totalweight=0;
  for( int i = 1; i < points->num; i++ ) {
    accumweight[i] = accumweight[i-1] + points->p[i].weight;
  }
  totalweight=accumweight[points->num-1];

  for(int i=k1; i<k2; i++ ) {
    w = (lrand48()/(float)INT_MAX)*totalweight;
    //binary search
    l=0;
    r=points->num-1;
    if( accumweight[0] > w )  { 
      (*feasible)[i]=0; 
      continue;
    }
    while( l+1 < r ) {
      k = (l+r)/2;
      if( accumweight[k] > w ) {
	r = k;
      } 
      else {
	l=k;
      }
    }
    (*feasible)[i]=r;
  }

  free(accumweight); 

#ifdef PROFILE
  double t2 = gettime();
  time_select_feasible += t2-t1;
#endif
  return numfeasible;
}

/* compute approximate kmedian on the points */
float pkmedian(Points *points, long kmin, long kmax, long* kfinal,
	       int pid)
{
  int i;
  double cost;
  double lastcost;
  double hiz, loz, z;

  static long k;
  static int *feasible;
  static int numfeasible;
  static double* hizs;

  if( pid==0 ) hizs = (double*)calloc(nproc,sizeof(double));
  hiz = loz = 0.0;
  long numberOfPoints = points->num;
  long ptDimension = points->dim;

  //my block
  long bsize = points->num/nproc;
  long k1 = bsize * pid;
  long k2 = k1 + bsize;
  if( pid == nproc-1 ) k2 = points->num;

#ifdef PRINTINFO
  if( pid == 0 )
    {
    }
#endif


  double myhiz = 0;
  for (long kk=k1;kk < k2; kk++ ) {
    myhiz += dist(points->p[kk], points->p[0],
		      ptDimension)*points->p[kk].weight;
  }
  hizs[pid] = myhiz;

  for( int i = 0; i < nproc; i++ )   {
    hiz += hizs[i];
  }

  loz=0.0; z = (hiz+loz)/2.0;
  /* NEW: Check whether more centers than points! */
  if (points->num <= kmax) {
    /* just return all points as facilities */
    for (long kk=k1;kk<k2;kk++) {
      points->p[kk].assign = kk;
      points->p[kk].cost = 0;
    }
    cost = 0;
    if( pid== 0 ) {
      free(hizs); 
      *kfinal = k;
    }
    return cost;
  }

  if( pid == 0 ) shuffle(points);
  cost = pspeedy(points, z, &k, pid);

#ifdef PRINTINFO
  if( pid == 0 )
#endif
  i=0;
  /* give speedy SP chances to get at least kmin/2 facilities */
  while ((k < kmin)&&(i<SP)) {
    cost = pspeedy(points, z, &k, pid);
    i++;
  }

#ifdef PRINTINFO
  if( pid==0)
#endif 
  /* if still not enough facilities, assume z is too high */
  while (k < kmin) {
#ifdef PRINTINFO
    if( pid == 0 ) {
    }
#endif
    if (i >= SP) {hiz=z; z=(hiz+loz)/2.0; i=0;}
    if( pid == 0 ) shuffle(points);
    cost = pspeedy(points, z, &k, pid);
    i++;
  }

  /* now we begin the binary search for real */
  /* must designate some points as feasible centers */
  /* this creates more consistancy between FL runs */
  /* helps to guarantee correct # of centers at the end */
  
  if( pid == 0 )
    {
      numfeasible = selectfeasible_fast(points,&feasible,kmin,pid);
      for( int i = 0; i< points->num; i++ ) {
	is_center[points->p[i].assign]= true;
      }
    }


  while(1) {
#ifdef PRINTINFO
    if( pid==0 )
      {
      }
#endif
    /* first get a rough estimate on the FL solution */
    //    pthread_barrier_wait(barrier);

    lastcost = cost;
    cost = pFL(points, feasible, numfeasible,
	       z, &k, cost, (long)(ITER*kmax*log((double)kmax)), 0.1, pid);

    /* if number of centers seems good, try a more accurate FL */
    if (((k <= (1.1)*kmax)&&(k >= (0.9)*kmin))||
	((k <= kmax+2)&&(k >= kmin-2))) {

#ifdef PRINTINFO
      if( pid== 0)
	{
	}
#endif
      /* may need to run a little longer here before halting without
	 improvement */
      cost = pFL(points, feasible, numfeasible,
		 z, &k, cost, (long)(ITER*kmax*log((double)kmax)), 0.001, pid);
    }

    if (k > kmax) {
      /* facilities too cheap */
      /* increase facility cost and up the cost accordingly */
      loz = z; z = (hiz+loz)/2.0;
      cost += (z-loz)*k;
    }
    if (k < kmin) {
      /* facilities too expensive */
      /* decrease facility cost and reduce the cost accordingly */
      hiz = z; z = (hiz+loz)/2.0;
      cost += (z-hiz)*k;
    }

    /* if k is good, return the result */
    /* if we're stuck, just give up and return what we have */
    if (((k <= kmax)&&(k >= kmin))||((loz >= (0.999)*hiz)) )
      { 
	break;
      }
  }

  //clean up...
  if( pid==0 ) {
    free(feasible); 
    free(hizs);
    *kfinal = k;
  }

  return cost;
}

/* compute the means for the k clusters */
int contcenters(Points *points)
{
  long i, ii;
  float relweight;

  for (i=0;i<points->num;i++) {
    /* compute relative weight of this point to the cluster */
    if (points->p[i].assign != i) {
      relweight=points->p[points->p[i].assign].weight + points->p[i].weight;
      relweight = points->p[i].weight/relweight;
      for (ii=0;ii<points->dim;ii++) {
	points->p[points->p[i].assign].coord[ii]*=1.0-relweight;
	points->p[points->p[i].assign].coord[ii]+=
	  points->p[i].coord[ii]*relweight;
      }
      points->p[points->p[i].assign].weight += points->p[i].weight;
    }
  }
  
  return 0;
}

/* copy centers from points to centers */
void copycenters(Points *points, Points* centers, long* centerIDs, long offset)
{
  long i;
  long k;

  bool *is_a_median = (bool *) calloc(points->num, sizeof(bool));

  /* mark the centers */
  for ( i = 0; i < points->num; i++ ) {
    is_a_median[points->p[i].assign] = 1;
  }

  k=centers->num;

  /* count how many  */
  for ( i = 0; i < points->num; i++ ) {
    if ( is_a_median[i] ) {
      memcpy( centers->p[k].coord, points->p[i].coord, points->dim * sizeof(float));
      centers->p[k].weight = points->p[i].weight;
      centerIDs[k] = i + offset;
      k++;
    }
  }

  centers->num = k;

  free(is_a_median);
}

struct pkmedian_arg_t
{
  Points* points;
  long kmin;
  long kmax;
  long* kfinal;
  int pid;
};

void* localSearchSub(void* arg_) {

  pkmedian_arg_t* arg= (pkmedian_arg_t*)arg_;
  pkmedian(arg->points,arg->kmin,arg->kmax,arg->kfinal,arg->pid);

  return NULL;
}

void localSearch( Points* points, long kmin, long kmax, long* kfinal ) {
#ifdef PROFILE
  double t1 = gettime();
#endif

    pkmedian_arg_t* arg = new pkmedian_arg_t[nproc];


    for( int i = 0; i < nproc; i++ ) {
      arg[i].points = points;
      arg[i].kmin = kmin;
      arg[i].kmax = kmax;
      arg[i].pid = i;
      arg[i].kfinal = kfinal;

      localSearchSub(&arg[0]);
    }

    for ( int i = 0; i < nproc; i++) {
    }

    delete[] arg;

#ifdef PROFILE
  double t2 = gettime();
  time_local_search += t2-t1;
#endif
 
}

class PStream {
public:
  virtual size_t read( float* dest, int dim, int num ) = 0;
  virtual int ferror() = 0;
  virtual int feof() = 0;
  virtual ~PStream() {
  }
};

//synthetic stream
class SimStream : public PStream {
public:
  SimStream(long n_ ) {
    n = n_;
  }
  size_t read( float* dest, int dim, int num ) {
    size_t count = 0;
    for( int i = 0; i < num && n > 0; i++ ) {
      for( int k = 0; k < dim; k++ ) {
	dest[i*dim + k] = lrand48()/(float)INT_MAX;
      }
      n--;
      count++;
    }
    return count;
  }
  int ferror() {
    return 0;
  }
  int feof() {
    return n <= 0;
  }
  ~SimStream() { 
  }
private:
  long n;
};

class FileStream : public PStream {
public:
  FileStream(char* filename) {
    fp = fopen( filename, "rb");
    if( fp == NULL ) {
      exit(1);
    }
  }
  size_t read( float* dest, int dim, int num ) {
    return std::fread(dest, sizeof(float)*dim, num, fp); 
  }
  int ferror() {
    return std::ferror(fp);
  }
  int feof() {
    return std::feof(fp);
  }
  ~FileStream() {
    fclose(fp);
  }
private:
  FILE* fp;
};

void outcenterIDs( Points* centers, long* centerIDs, char* outfile ) {
  FILE* fp = fopen(outfile, "w");
  if( fp==NULL ) {
    exit(1);
  }
  int* is_a_median = (int*)calloc( sizeof(int), centers->num );
  for( int i =0 ; i< centers->num; i++ ) {
    is_a_median[centers->p[i].assign] = 1;
  }

  for( int i = 0; i < centers->num; i++ ) {
    if( is_a_median[i] ) {
      for( int k = 0; k < centers->dim; k++ ) {
      }
    }
  }
  fclose(fp);
}

void streamCluster( PStream* stream, 
		    long kmin, long kmax, int dim,
		    long chunksize, long centersize, char* outfile )
{
  float* block = (float*)malloc( chunksize*dim*sizeof(float) );
  float* centerBlock = (float*)malloc(centersize*dim*sizeof(float) );
  long* centerIDs = (long*)malloc(centersize*dim*sizeof(long));

  if( block == NULL ) { 
    exit(1);
  }

  Points points;
  points.dim = dim;
  points.num = chunksize;
  points.p = (Point *)malloc(chunksize*sizeof(Point));
  for( int i = 0; i < chunksize; i++ ) {
    points.p[i].coord = &block[i*dim];
  }

  Points centers;
  centers.dim = dim;
  centers.p = (Point *)malloc(centersize*sizeof(Point));
  centers.num = 0;

  for( int i = 0; i< centersize; i++ ) {
    centers.p[i].coord = &centerBlock[i*dim];
    centers.p[i].weight = 1.0;
  }

  long IDoffset = 0;
  long kfinal;
  while(1) {

    size_t numRead  = stream->read(block, dim, chunksize ); 

    if( stream->ferror() || numRead < (unsigned int)chunksize && !stream->feof() ) {
      exit(1);
    }

    points.num = numRead;
    for( int i = 0; i < points.num; i++ ) {
      points.p[i].weight = 1.0;
    }

    switch_membership = (bool*)malloc(points.num*sizeof(bool));
    is_center = (bool*)calloc(points.num,sizeof(bool));
    center_table = (int*)malloc(points.num*sizeof(int));

    localSearch(&points,kmin, kmax,&kfinal);

    contcenters(&points);
    if( kfinal + centers.num > centersize ) {
      //here we don't handle the situation where # of centers gets too large. 
      exit(1);
    }


    copycenters(&points, &centers, centerIDs, IDoffset);
    IDoffset += numRead;


    free(is_center);
    free(switch_membership);
    free(center_table);

    if( stream->feof() ) {
      break;
    }
  }

  //finally cluster all temp centers
  switch_membership = (bool*)malloc(centers.num*sizeof(bool));
  is_center = (bool*)calloc(centers.num,sizeof(bool));
  center_table = (int*)malloc(centers.num*sizeof(int));

  localSearch( &centers, kmin, kmax ,&kfinal );
  contcenters(&centers);
  outcenterIDs( &centers, centerIDs, outfile);
}

int main(int argc, char **argv)
{
  char *outfilename = new char[MAXNAMESIZE];
  char *infilename = new char[MAXNAMESIZE];
  long kmin, kmax, n, chunksize, clustersize;
  int dim;

#ifdef PARSEC_VERSION
#define __PARSEC_STRING(x) #x
#define __PARSEC_XSTRING(x) __PARSEC_STRING(x)
	fflush(NULL);
#else
	fflush(NULL);
#endif //PARSEC_VERSION
#ifdef ENABLE_PARSEC_HOOKS
  __parsec_bench_begin(__parsec_streamcluster);
#endif

  if (argc<10) {
    fprintf(stderr,"usage: %s k1 k2 d n chunksize clustersize infile outfile nproc\n",
	    argv[0]);
    fprintf(stderr,"  k1:          Min. number of centers allowed\n");
    fprintf(stderr,"  k2:          Max. number of centers allowed\n");
    fprintf(stderr,"  d:           Dimension of each data point\n");
    fprintf(stderr,"  n:           Number of data points\n");
    fprintf(stderr,"  chunksize:   Number of data points to handle per step\n");
    fprintf(stderr,"  clustersize: Maximum number of intermediate centers\n");
    fprintf(stderr,"  infile:      Input file (if n<=0)\n");
    fprintf(stderr,"  outfile:     Output file\n");
    fprintf(stderr,"  nproc:       Number of threads to use\n");
    fprintf(stderr,"\n");
    fprintf(stderr, "if n > 0, points will be randomly generated instead of reading from infile.\n");
    exit(1);
  }
  kmin = atoi(argv[1]);
  kmax = atoi(argv[2]);
  dim = atoi(argv[3]);
  n = atoi(argv[4]);
  chunksize = atoi(argv[5]);
  clustersize = atoi(argv[6]);
  strcpy(infilename, argv[7]);
  strcpy(outfilename, argv[8]);
  nproc = atoi(argv[9]);

  srand48(SEED);
  PStream* stream;
  if( n > 0 ) {
    stream = new SimStream(n);
  }
  else {
    stream = new FileStream(infilename);
  }



#ifdef ENABLE_PARSEC_HOOKS
  __parsec_roi_begin();
#endif
  double t1 = gettime();
  streamCluster(stream, kmin, kmax, dim, chunksize, clustersize, outfilename );
  double t2 = gettime();
#ifdef ENABLE_PARSEC_HOOKS
  __parsec_roi_end();
#endif

printf("%lf\n", t2-t1);

  delete stream;
#ifdef PROFILE
 #endif
  
#ifdef ENABLE_PARSEC_HOOKS
  __parsec_bench_end();
#endif
  
  return 0;
}
