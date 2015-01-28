/*
 * ft.c
 *
 * The author of this software is Alain K\"{a}gi.
 *
 * Copyright (c) 1993 by Alain K\"{a}gi and the University of Wisconsin
 * Board of Trustees.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHOR NOR THE UNIVERSITY OF
 * WISCONSIN MAKE ANY REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING
 * THE MERCHANTABILITY OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR
 * PURPOSE.
 *
 * ------------------------------------------------------------------------
 *
 * This is the first algorithm presented in
 *
 *   , by Michael L. Fredman and Robert Endre Tarjan, in
 *   Journal of Association for Computing Machinery, Vol. 34, No. 3,
 *   July 1987, Pages 596-615,
 *
 * for find the minimum spanning tree.
 *
 * ------------------------------------------------------------------------
 *
 * $Id: ft.c,v 1.3 1993/03/15 04:30:47 alain Exp alain $
 *
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <sys/time.h>
#include <sys/resource.h>

#include "Fheap.h"
#include "graph.h"

#define MINUS_INFINITY		INT_MIN
#define PLUS_INFINITY		INT_MAX

#define DEFAULT_N_VERTEX	10
#define DEFAULT_N_EDGE		9

/*
 * Local functions.
 */
void      PrintMST(Vertices * graph);
Vertices * MST(Vertices * graph);
void      PrintRUsage(struct rusage * rUBuf1, struct rusage * rUBuf2);

/*
 * Local variables.
 */
int debug = 1;

void
main(int argc, char *argv[])
{
  int            nVertex;
  int            nEdge;
  Vertices *  graph;
  struct rusage * rUBuf1;
  struct rusage * rUBuf2;

  nVertex = DEFAULT_N_VERTEX;
  nEdge = DEFAULT_N_EDGE;

  if(argc > 1)
  {
    nVertex = atoi(argv[1]);
    if(argc > 2)
    {
      nEdge = atoi(argv[2]);
      if(argc > 3)
      {
        srandom(atoi(argv[3]));
      }
    }
  }

  rUBuf1 = (struct rusage *)malloc(sizeof(struct rusage));
  assert(rUBuf1 != NULL);
  rUBuf2 = (struct rusage *)malloc(sizeof(struct rusage));
  assert(rUBuf1 != NULL);

  if(debug)
  {
    printf("Generating a connected graph ... ");
  }

  graph = GenGraph(nVertex, nEdge);

  if(debug)
  {
    printf("done\nFinding the mininmum spanning tree ... ");
  }

  getrusage(RUSAGE_SELF, rUBuf1);

  graph = MST(graph);

  getrusage(RUSAGE_SELF, rUBuf2);

  if(debug)
  {
    printf("done\nThe graph:\n");
    PrintGraph(graph);
    printf("The minimum spanning tree:\n");
    PrintMST(graph);
  }

  if(debug)
  {
    printf("Time spent in finding the mininum spanning tree:\n");
  }
  PrintRUsage(rUBuf1, rUBuf2);
#ifdef PLUS_STATS
  PrintDerefStats(stderr);
  PrintHeapSize(stderr);
#endif /* PLUS_STATS */
  exit(0);
}

Vertices *
MST(Vertices * graph)
{
  HeapP * heap;
  Vertices * vertex;
  Edges * edge;
  ;

  InitFHeap();

  /*
   * key(s) = 0;
   * key(v) = infty for v != s;
   * init heap;
   * make a heap;
   * put s in heap;
   */
  vertex = graph;
  KEY(vertex) = 0;
  heap = MakeHeap();
  (void)Insert(&heap, (Item *)vertex);

  vertex = NEXT_VERTEX(vertex);
  while(vertex != graph)
  {
    KEY(vertex) = PLUS_INFINITY;
    vertex = NEXT_VERTEX(vertex);
  }
  while(vertex != graph);

  vertex = FindMin(heap);
  while(vertex != NULL_VERTEX)
  {
    heap = DeleteMin(heap);
    KEY(vertex) = MINUS_INFINITY;
    edge = EDGES(vertex);
    while(edge != NULL_EDGE)
    {
      if(WEIGHT(edge) < KEY(VERTEX(edge)))
      {
        KEY(VERTEX(edge)) = WEIGHT(edge);
        CHOSEN_EDGE(VERTEX(edge)) = edge;
        (void)Insert(&heap, VERTEX(edge));
      }
      edge = NEXT_EDGE(edge);
    }
    vertex = FindMin(heap);
  }
  ;
  return(graph);
}

void
PrintMST(Vertices * graph)
{
  Vertices * vertex;

  assert(graph != NULL_VERTEX);

  vertex = NEXT_VERTEX(graph);

  while(vertex != graph)
  {
    printf("vertex %d to %d\n", ID(vertex), ID(SOURCE(CHOSEN_EDGE(vertex))));
    vertex = NEXT_VERTEX(vertex);
  }

}

void
PrintRUsage(struct rusage * rUBuf1, struct rusage * rUBuf2)
{
  long sec;
  long usec;

  sec = (*rUBuf2).ru_utime.tv_sec - (*rUBuf1).ru_utime.tv_sec;
  usec = (*rUBuf2).ru_utime.tv_usec - (*rUBuf1).ru_utime.tv_usec;
  if(usec < 0)
  {
    sec--;
    usec += 1000000;
  }
  assert(sec >= 0);
  assert(usec >= 0);

  fprintf(stderr, "User time: %d%06d usec\n", sec, usec); 

  sec = (*rUBuf2).ru_stime.tv_sec - (*rUBuf1).ru_stime.tv_sec;
  usec = (*rUBuf2).ru_stime.tv_usec - (*rUBuf1).ru_stime.tv_usec;

  if(usec < 0)
  {
    sec--;
    usec += 1000000;
  }
  assert(sec >= 0);
  assert(usec >= 0);

  fprintf(stderr, "Sys time:  %d%06d usec\n", sec, usec); 
}
