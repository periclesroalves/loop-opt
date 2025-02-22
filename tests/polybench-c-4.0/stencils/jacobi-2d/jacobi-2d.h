/**
 * This version is stamped on Mar. 3, 2015
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* jacobi-2d.h: this file is part of PolyBench/C */

#ifndef _JACOBI_2D_H
# define _JACOBI_2D_H

/* Default to LARGE_DATASET. */
# if !defined(MINI_DATASET) && !defined(SMALL_DATASET) && !defined(MEDIUM_DATASET) && !defined(LARGE_DATASET) && !defined(EXTRALARGE_DATASET)
#  define LARGE_DATASET
# endif

# if !defined(TSTEPS) && !defined(N)
/* Define sample dataset sizes. */
#  ifdef MINI_DATASET
#   define TSTEPS 20
#   define N 30
#  endif 

#  ifdef SMALL_DATASET
#   define TSTEPS 40
#   define N 90
#  endif 

#  ifdef MEDIUM_DATASET
#   define TSTEPS 100
#   define N 250
#  endif 

#  ifdef LARGE_DATASET
#   define TSTEPS 500
#   define N 1300
#  endif 

#  ifdef EXTRALARGE_DATASET
#   define TSTEPS 1000
#   define N 2800
#  endif 


#endif /* !(TSTEPS N) */

# define _PB_TSTEPS POLYBENCH_LOOP_BOUND(TSTEPS,tsteps)
# define _PB_N POLYBENCH_LOOP_BOUND(N,n)


# ifndef DATA_TYPE
#  define DATA_TYPE double
#  define DATA_PRINTF_MODIFIER "%0.2lf "
#  define SCALAR_VAL(x) x
#  define SQRT_FUN(x) sqrt(x)
#  define EXP_FUN(x) exp(x)
#  define POW_FUN(x,y) pow(x,y)
# endif



#endif /* !_JACOBI_2D_H */

