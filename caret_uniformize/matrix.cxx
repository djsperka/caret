/*****************************************************************************
   Major portions of this software are copyrighted by the Medical College
   of Wisconsin, 1994-2000, and are released under the Gnu General Public
   License, Version 2.  See the file README.Copyright for details.
******************************************************************************/

/*!
  This file contains matrix and vector arithmetic routines.

  File:     matrix.c
  Author:   B. Douglas Ward
  Date:     23 April 1997

  Mod:      Changed print format for functions matrix_print and vector_print.
  Date:     05 August 1997

  Mod:      Changed initialization in function vector_create.
  Date:     04 November 1997

  Mod:      Added routines matrix_file_write and matrix_file_read.
  Date:     02 July 1999

  Mod:      Added routine for calculating square root of matrix.
  Date:     30 September 1999

  Mod:      Added routines matrix_sprint and vector_sprint.
  Date:     04 October 1999

  Mod:      Modified matrix_file_read to use mri_read_ascii routine.
  Date:     12 January 2000

  Mod:      Changed return type of vector_dot from float to double.
  Date:     13 April 2000

  Mod:      Added functions column_to_vector and matrix_extract_rows.
  Date:     21 April 2000

  Mod:      Added test for missing matrix file name.
  Date:     08 May 2000

  Mod:      Added "register" declarations and a few other things to speed
            up calculations (including vector_create_noinit) -- RWCox.
  Date:     28 Dec 2001

  Mod:      Allow matrices and vectors with zero rows and columns.
  Date:     26 February 2002

  Mod:      Corrected errors in vector_multiply and vector_multiply_subtract
            routines that would produce segmentation fault for certain input
	    matrices.
  Date:     18 March 2003

  Mod:      UNROLL_VECMUL defined to allow unrolling by 2 of vector-multiply
            dot product loops.

  Mod:      'ipr' added to matrix_print() function.
  Date:     03 Aug 2004 - RWCox

  Mod:      Added USE_SCSLBLAS stuff for SGI Altix, and USE_SUNPERF for Solaris.
  Date:     01 Mar 2005
*/

/*---------------------------------------------------------------------*/
/** Vectorization macros:
   - DOTP(n,x,y,z) computes the n-long dot product of vectors
       x and y and puts the result into the place pointed to by z.
   - VSUB(n,x,y,z) computes vector x-y into vector z.
   - These are intended to be the fast method for doing these things. **/
/*---------------------------------------------------------------------*/

#include <cmath>
#include <cstdio>    // CARET
#include <cstdlib>   // CARET
#include <cstring>   // CARET
#include "matrix.h"  // CARET


#undef SETUP_BLAS1  /* define this to use BLAS-1 functions */
#undef SETUP_BLAS2  /* define this to use BLAS-2 functions */
#undef DOTP
#undef VSUB
#undef MATVEC
#undef SUBMATVEC

#if defined(USE_SCSLBLAS)                            /** SGI Altix **/
#  include <scsl_blas.h>
#  define SETUP_BLAS1
#  undef  SETUP_BLAS2     /* don't use this */
#  define TRANSA "T"
#elif defined(USE_SUNPERF)                           /** Sun Solaris **/
#  include <sunperf.h>
#  define SETUP_BLAS1
#  undef SETUP_BLAS2
#  define TRANSA 'T'
#elif defined(USE_ACML)
#  ifndef _ACML_COMPLEX
#  define _ACML_COMPLEX
 /*    typedef struct { float real, imag; } complex; */
 /*    typedef struct { double real, imag; } doublecomplex; */
#  endif
#  include <acml.h>
#  define SETUP_BLAS1
#endif

/* double precision BLAS-1 functions */

#ifdef SETUP_BLAS1
# define DOTP(n,x,y,z) *(z)=ddot(n,x,1,y,1)
# define VSUB(n,x,y,z) (memcpy(z,x,sizeof(double)*n),daxpy(n,-1.0,y,1,z,1))
#endif

/*.......................................................................
   BLAS-2 function operate on matrix-vector structs defined in matrix.h:

   MATVEC(m,v,z):    [z] = [m][v] where m=matrix, z,v = matrices
   SUBMATVEC(m,v,z): [z] = [z] - [m][v]
.........................................................................*/

#ifdef DONT_USE_MATRIX_MAT
# undef SETUP_BLAS2
#endif

#ifdef SETUP_BLAS2  /* doesn't seem to help much */

# define MATVEC(m,v,z) dgemv( TRANSA , (m).cols , (m).rows ,       \
                              1.0 , (m).mat , (m).cols ,           \
                              (v).elts , 1 , 0.0 , (z).elts , 1 )

# define SUBMATVEC(m,v,z) dgemv( TRANSA , (m).cols , (m).rows ,      \
                                 -1.0 , (m).mat , (m).cols ,         \
                                 (v).elts , 1 , 1.0 , (z).elts , 1 )
#endif

/*---------------------------------------------------------------------------*/
static double flops=0.0 ;
double get_matrix_flops(void){ return flops; }

static double dotnum=0.0 , dotsum=0.0 ;
double get_matrix_dotlen(void){ return (dotnum > 0.0) ? dotsum/dotnum : 0.0 ; }

#define ENABLE_FLOPS
/*---------------------------------------------------------------------------*/
/*!
  Routine to print and error message and stop.
*/

void matrix_error (char * message)
{ 
  printf ("Matrix error: %s \n", message);
  exit (1);
}


/*---------------------------------------------------------------------------*/
/*!
  Initialize matrix data structure.
*/

void matrix_initialize (matrix * m)
{
  m->rows = 0;
  m->cols = 0;
  m->elts = NULL;
#ifndef DONT_USE_MATRIX_MAT
  m->mat  = NULL ;  /* 04 Mar 2005 */
#endif
}


/*---------------------------------------------------------------------------*/
/*!
  Destroy matrix data structure by deallocating memory.
*/

void matrix_destroy (matrix * m)
{
  if (m->elts != NULL){
#ifdef DONT_USE_MATRIX_MAT
    int i ;
    for( i=0 ; i < m->rows ; i++ )
      if( m->elts[i] != NULL ) free(m->elts[i]) ;
#endif
    free(m->elts) ;
  }
#ifndef DONT_USE_MATRIX_MAT
  if( m->mat  != NULL) free (m->mat ) ;
#endif
  matrix_initialize (m);
}

/*---------------------------------------------------------------------------*/
/*!
  Create matrix data structure by allocating memory and initializing values.

*/

void matrix_create (int rows, int cols, matrix * m)
{
  register int i ;

  matrix_destroy (m);

  if ((rows < 0) || (cols < 0))
    matrix_error ("Illegal dimensions for new matrix");

  m->rows = rows;
  m->cols = cols;
  if ((rows < 1) || (cols < 1))  return;

  m->elts = (double **) malloc (sizeof(double *) * rows);
  if (m->elts == NULL)
    matrix_error ("Memory allocation error");

#ifdef DONT_USE_MATRIX_MAT
  for (i = 0;  i < rows;  i++){
    m->elts[i] = (double *) calloc (sizeof(double) , cols);
    if (m->elts[i] == NULL) matrix_error ("Memory allocation error");
  }
#else
  m->mat  = (double *) calloc( sizeof(double) , rows*cols ) ;
  if( m->mat == NULL )
    matrix_error ("Memory allocation error");
  for (i = 0;  i < rows;  i++)
     m->elts[i] = m->mat + (i*cols) ;   /* 04 Mar 2005: offsets into mat */
#endif
}


/*---------------------------------------------------------------------------*/
/*!
  Print contents of matrix m.
*/

void matrix_print (matrix m)
{
  int i=0, j=0;
  int rows, cols;
  double val ;
  int ipr ;

  rows = m.rows;
  cols = m.cols;

  for( i=0 ; i < rows ; i++ ){
    for( j=0 ; j < cols ; j++ ){
      val = (int)m.elts[i][j] ;
      if( val != m.elts[i][j] || fabs(val) > 9.0 ) goto zork ; 
    }
  }
zork:
  ipr = (i==rows && j==cols) ;

  for (i = 0;  i < rows;  i++)
    {
      for (j = 0;  j < cols;  j++)
        if( ipr ) printf (" %2d"   , (int)m.elts[i][j]);
        else      printf (" %10.4g", m.elts[i][j]);
      printf (" \n");
    }
  printf (" \n"); fflush(stdout) ;
}


/*---------------------------------------------------------------------------*/
/*!
  Print label and contents of matrix m.
*/

void matrix_sprint (char * s, matrix m)
{
  printf ("%s \n", s);
  matrix_print (m);
}


/*---------------------------------------------------------------------------*/
/*!
  Print contents of matrix m to specified file.
*/

void matrix_file_write (char * filename, matrix m)
{
  int i, j;
  int rows, cols;
  FILE * outfile = NULL;


  /*----- First, check for empty file name -----*/
  if (filename == NULL)  matrix_error ("Missing matrix file name");


  outfile = fopen (filename, "w");

  rows = m.rows;
  cols = m.cols;

  for (i = 0;  i < rows;  i++)
    {
      for (j = 0;  j < cols;  j++)
        fprintf (outfile, "  %g", m.elts[i][j]);
      fprintf (outfile, " \n");
    }
  fprintf (outfile, " \n");

  fclose (outfile);
}

/*---------------------------------------------------------------------------*/
/*!
  Manual entry of matrix data.
*/

void matrix_enter (matrix * m)
{
  int rows, cols;
  int i, j;
  float fval;

  printf ("Enter number of rows: "); fflush(stdout) ;
  scanf ("%d", &rows);
  printf ("Enter number of cols: "); fflush(stdout) ;
  scanf ("%d", &cols);

  matrix_create (rows, cols, m);

  for (i = 0;  i < rows;  i++)
    for (j = 0;  j < cols;  j++)
      {
	     printf ("elts[%d][%d] = ", i, j); fflush(stdout);
	     scanf ("%f", &fval);
	     m->elts[i][j] = fval;
      }
}


/*---------------------------------------------------------------------------*/
/*!
  Read contents of matrix m from specified file.
  If unable to read matrix from file, or matrix has wrong dimensions:
     If error_exit flag is set, then print error message and exit.
     Otherwise, return null matrix.
*/

// matrix_file_read REMOVED CARET


/*---------------------------------------------------------------------------*/
/*!
  Convert simple array to matrix structure.
*/

void array_to_matrix (int rows, int cols, float ** f, matrix * m)
{
  register int i, j;

  matrix_create (rows, cols, m);

  for (i = 0;  i < rows;  i++)
    for (j = 0;  j < cols;  j++)
      m->elts[i][j] = f[i][j];
}


/*---------------------------------------------------------------------------*/
/*!
  Make a copy of the first matrix, return copy as the second matrix.
*/

void matrix_equate (matrix a, matrix * b)
{
  register int i; //, j;
  register int rows, cols;

  rows = a.rows;
  cols = a.cols;

  matrix_create (rows, cols, b);

  for (i = 0;  i < rows;  i++){
#if 0
    for (j = 0;  j < cols;  j++)
      b->elts[i][j] = a.elts[i][j];
#else
    if( cols > 0 )
      memcpy( b->elts[i] , a.elts[i] , sizeof(double)*cols ) ;  /* RWCox */
#endif
  }
}


/*---------------------------------------------------------------------------*/
/*!
  Extract p columns (specified by list) from matrix a.  Result is matrix b.
*/

void matrix_extract (matrix a, int p, int *list, matrix * b)
{
  register int i, j;
  register int rows, cols;

  rows = a.rows;
  cols = p;

  matrix_create (rows, cols, b);

  for (i = 0;  i < rows;  i++)
    for (j = 0;  j < cols;  j++)
      b->elts[i][j] = a.elts[i][list[j]];
}


/*---------------------------------------------------------------------------*/
/*!
  Extract p rows (specified by list) from matrix a.  Result is matrix b.
*/

void matrix_extract_rows (matrix a, int p, int * list, matrix * b)
{
  register int i, j;
  register int rows, cols;

  rows = p;
  cols = a.cols;

  matrix_create (rows, cols, b);

  for (i = 0;  i < rows;  i++)
    for (j = 0;  j < cols;  j++)
      b->elts[i][j] = a.elts[list[i]][j];
}


/*---------------------------------------------------------------------------*/
/*!
  Create n x n identity matrix.
*/

void matrix_identity (int n, matrix * m)
{
  register int i, j;

  if (n < 0)
    matrix_error ("Illegal dimensions for identity matrix");

  matrix_create (n, n, m);

  for (i = 0;  i < n;  i++)
    for (j = 0;  j < n;  j++)
      if (i == j)
	m->elts[i][j] = 1.0;
      else
	m->elts[i][j] = 0.0;
}


/*---------------------------------------------------------------------------*/
/*!
  Add matrix a to matrix b.  Result is matrix c.
*/

void matrix_add (matrix a, matrix b, matrix * c)
{
  register int rows, cols;
  register int i, j;

  if ((a.rows != b.rows) || (a.cols != b.cols))
    matrix_error ("Incompatible dimensions for matrix addition");

  rows = a.rows;
  cols = a.cols;

  matrix_create (rows, cols, c);

  for (i = 0;  i < rows;  i++)
    for (j = 0;  j < cols;  j++)
      c->elts[i][j] = a.elts[i][j] + b.elts[i][j];

#ifdef ENABLE_FLOPS
  flops += rows*cols ;
#endif
}


/*---------------------------------------------------------------------------*/
/*!
  Subtract matrix b from matrix a.  Result is matrix c.
*/

void matrix_subtract (matrix a, matrix b, matrix * c)
{
  register int rows, cols;
  register int i, j;

  if ((a.rows != b.rows) || (a.cols != b.cols))
    matrix_error ("Incompatible dimensions for matrix subtraction");

  rows = a.rows;
  cols = a.cols;

  matrix_create (rows, cols, c);

  for (i = 0;  i < rows;  i++)
    for (j = 0;  j < cols;  j++)
      c->elts[i][j] = a.elts[i][j] - b.elts[i][j];

#ifdef ENABLE_FLOPS
  flops += rows*cols ;
#endif
}


/*---------------------------------------------------------------------------*/
/*!
  Multiply matrix a by matrix b.  Result is matrix c.
*/

void matrix_multiply (matrix a, matrix b, matrix * c)
{
  int rows, cols;
  register int i, j, k;
  register double sum ;

  if (a.cols != b.rows)
    matrix_error ("Incompatible dimensions for matrix multiplication");

  rows = a.rows;
  cols = b.cols;

  matrix_create (rows, cols, c);

  for (i = 0;  i < rows;  i++)
    for (j = 0;  j < cols;  j++)
      {
        sum = 0.0 ;
	for (k = 0;  k < a.cols;  k++)
	  sum += a.elts[i][k] * b.elts[k][j];
        c->elts[i][j] = sum;
      }

#ifdef ENABLE_FLOPS
  flops += 2.0*rows*cols*cols ;
#endif
}


/*---------------------------------------------------------------------------*/
/*!
  Multiply matrix a by scalar constant k.  Result is matrix c.
*/

void matrix_scale (double k, matrix a, matrix * c)
{
  register int rows, cols;
  register int i, j;

  rows = a.rows;
  cols = a.cols;

  matrix_create (rows, cols, c);

  for (i = 0;  i < rows;  i++)
    for (j = 0;  j < cols;  j++)
      c->elts[i][j] = k * a.elts[i][j];

#ifdef ENABLE_FLOPS
  flops += rows*cols ;
#endif
}


/*---------------------------------------------------------------------------*/
/*!
  Take transpose of matrix a.  Result is matrix t.
*/

void matrix_transpose (matrix a, matrix * t)
{
  register int rows, cols;
  register int i, j;

  rows = a.cols;
  cols = a.rows;

  matrix_create (rows, cols, t);
  for (i = 0;  i < rows;  i++)
    for (j = 0;  j < cols;  j++)
      t->elts[i][j] = a.elts[j][i];
}

/*---------------------------------------------------------------------------*/
/*!
  Use Gaussian elimination to calculate inverse of matrix a.  Result is
  matrix ainv.
*/

int matrix_inverse (matrix a, matrix * ainv)
{
  const double epsilon = 1.0e-10;
  matrix tmp;
  register int i, j, ii, n;
  register double fval;
  register double fmax;
  register double * p;

  matrix_initialize (&tmp);


  if (a.rows != a.cols)
    matrix_error ("Illegal dimensions for matrix inversion");

#if 0
matrix_sprint("matrix_inverse:",a) ;
#endif

  n = a.rows;
  matrix_identity (n, ainv);
  matrix_equate (a, &tmp);

  for (i = 0;  i < n;  i++)
    {
      fmax = fabs(tmp.elts[i][i]);  
      for (j = i+1;  j < n;  j++)
	if (fabs(tmp.elts[j][i]) > fmax) 
	  {
	    fmax = fabs(tmp.elts[j][i]);
	    p = tmp.elts[i];
	    tmp.elts[i] = tmp.elts[j];
	    tmp.elts[j] = p;
	    p = ainv->elts[i];
	    ainv->elts[i] = ainv->elts[j];
	    ainv->elts[j] = p;
	  }

      if (fmax < epsilon)
	{
	  matrix_destroy (&tmp);
	  return (0);
	}
	

      fval = 1.0 / tmp.elts[i][i];   /* RWCox: change division by this to */
      for (j = 0;  j < n;  j++)      /*        multiplication by 1.0/this */
	{
	  tmp.elts[i][j]   *= fval;
	  ainv->elts[i][j] *= fval;
	}
      for (ii = 0;  ii < n;  ii++)
	if (ii != i)
	  {
	    fval = tmp.elts[ii][i];
	    for (j = 0;  j < n;  j++)
	      {
		tmp.elts[ii][j] -= fval*tmp.elts[i][j];
		ainv->elts[ii][j] -= fval*ainv->elts[i][j];
	      }
	  }
	
    }

  matrix_destroy (&tmp);

#ifdef ENABLE_FLOPS
  flops += 3.0*n*n*n ;
#endif
  return (1);
}


/*---------------------------------------------------------------------------*/
/*!
  Use Gaussian elimination to calculate inverse of matrix a, with diagonal
  scaling applied for stability.  Result is matrix ainv.
*/

int matrix_inverse_dsc (matrix a, matrix * ainv)  /* 15 Jul 2004 - RWCox */
{
  matrix atmp;
  register int i, j, n;
  register double *diag ;
  int mir ;

  if (a.rows != a.cols)
    matrix_error ("Illegal dimensions for matrix inversion");

  matrix_initialize (&atmp);

  n = a.rows;
  matrix_equate (a, &atmp);
  diag = (double *)malloc( sizeof(double)*n ) ;
  for( i=0 ; i < n ; i++ ){
    diag[i] = fabs(atmp.elts[i][i]) ;
    if( diag[i] == 0.0 ) diag[i] = 1.0 ;  /* shouldn't happen? */
    diag[i] = 1.0 / sqrt(diag[i]) ;
  }

  for( i=0 ; i < n ; i++ )                /* scale a */
   for( j=0 ; j < n ; j++ )
    atmp.elts[i][j] *= diag[i]*diag[j] ;

  mir = matrix_inverse( atmp , ainv ) ;   /* invert */

  for( i=0 ; i < n ; i++ )                /* scale inverse */
   for( j=0 ; j < n ; j++ )
    ainv->elts[i][j] *= diag[i]*diag[j] ;

  matrix_destroy (&atmp); free((void *)diag) ;
#ifdef ENABLE_FLOPS
  flops += 4.0*n*n + 4.0*n ;
#endif
  return (mir);
}


/*---------------------------------------------------------------------------*/
/*!
  Calculate square root of symmetric positive definite matrix a.
  Result is matrix s.
*/

int matrix_sqrt (matrix a, matrix * s)
{
  const int MAX_ITER = 100;
  int n;
  int ok;
  int iter;
  register float sse, psse;
  register int i, j;
  matrix x, xinv, axinv, xtemp, error;

  matrix_initialize (&x);
  matrix_initialize (&xinv);
  matrix_initialize (&axinv);
  matrix_initialize (&xtemp);
  matrix_initialize (&error);


  if (a.rows != a.cols)
    matrix_error ("Illegal dimensions for matrix square root");


  n = a.rows;
  matrix_identity (n, &x);


  psse = 1.0e+30;
  for (iter = 0;  iter < MAX_ITER;  iter++)
    {
      ok = matrix_inverse (x, &xinv);
      if (! ok)  return (0);
      matrix_multiply (a, xinv, &axinv);
      matrix_add (x, axinv, &xtemp);
      matrix_scale (0.5, xtemp, &x);

      matrix_multiply (x, x, &xtemp);
      matrix_subtract (a, xtemp, &error);
      sse = 0.0;
      for (i = 0;  i < n;  i++)
	for (j = 0;  j < n;  j++)
	  sse += error.elts[i][j] * error.elts[i][j] ;

      if (sse >= psse) break;

      psse = sse;
    }

  if (iter == MAX_ITER)  return (0);

  matrix_equate (x, s);

  matrix_destroy (&x);
  matrix_destroy (&xinv);
  matrix_destroy (&axinv);
  matrix_destroy (&xtemp);

  return (1);

}


/*---------------------------------------------------------------------------*/
/*!
  Initialize vector data structure.
*/

void vector_initialize (vector * v)
{
  v->dim = 0;
  v->elts = NULL;
}


/*---------------------------------------------------------------------------*/
/*!
  Destroy vector data structure by deallocating memory.
*/

void vector_destroy (vector * v)
{
  if (v->elts != NULL)  free (v->elts);
  vector_initialize (v);
}


/*---------------------------------------------------------------------------*/
/*!
  Create vector v by allocating memory and initializing values.
*/

void vector_create (int dim, vector * v)
{
  //register int i;

  vector_destroy (v);

  if (dim < 0)  matrix_error ("Illegal dimensions for new vector");

  v->dim = dim;
  if (dim < 1)  return;

  v->elts = (double *) calloc (sizeof(double) , dim);
  if (v->elts == NULL)
    matrix_error ("Memory allocation error");

}

/*---------------------------------------------------------------------------*/
static void vector_create_noinit(int dim, vector * v)  /* 28 Dec 2001: RWCox */
{
  //register int i;

  vector_destroy (v);

  if (dim < 0)  matrix_error ("Illegal dimensions for new vector");

  v->dim = dim;
  if (dim < 1)  return;

  v->elts = (double *) malloc (sizeof(double) * dim);
  if (v->elts == NULL)
    matrix_error ("Memory allocation error");
}

/*---------------------------------------------------------------------------*/
/*!
  Print contents of vector v.
*/

void vector_print (vector v)
{
  int i;

  for (i = 0;  i < v.dim;  i++)
    printf ("  %10.4g \n", v.elts[i]);
  printf (" \n"); fflush(stdout);

}


/*---------------------------------------------------------------------------*/
/*!
  Print label and contents of vector v.
*/

void vector_sprint (char * s, vector v)
{
  printf ("%s \n", s);

  vector_print (v);
}


/*---------------------------------------------------------------------------*/
/*!
  Copy vector a.  Result is vector b.
*/

void vector_equate (vector a, vector * b)
{
  //register int i, 
  register int dim;

  dim = a.dim;

  vector_create_noinit (dim, b);

#if 0
  for (i = 0;  i < dim;  i++)
    b->elts[i] = a.elts[i];
#else
  if( dim > 0 )
    memcpy( b->elts , a.elts , sizeof(double)*dim ) ;  /* RWCox */
#endif
}


/*---------------------------------------------------------------------------*/
/*!
  Convert simple array f into vector v.
*/

void array_to_vector (int dim, float * f, vector * v)
{
  register int i;

  vector_create_noinit (dim, v);

  for (i = 0;  i < dim;  i++)
    v->elts[i] = f[i];
}



/*---------------------------------------------------------------------------*/
/*!
  Convert column c of matrix m into vector v.
*/

void column_to_vector (matrix m, int c, vector * v)
{
  register int i;
  register int dim;

  dim = m.rows;
  vector_create_noinit (dim, v);

  for (i = 0;  i < dim;  i++)
    v->elts[i] = m.elts[i][c];
}



/*---------------------------------------------------------------------------*/
/*!
  Convert vector v into array f.
*/

void vector_to_array (vector v, float * f)
{
  register int i;

  for (i = 0;  i < v.dim;  i++)
    f[i] = v.elts[i];
}


/*---------------------------------------------------------------------------*/
/*!
  Add vector a to vector b.  Result is vector c.
*/

void vector_add (vector a, vector b, vector * c)
{
  register int i, dim;

  if (a.dim != b.dim)
    matrix_error ("Incompatible dimensions for vector addition");

  dim = a.dim;

  vector_create_noinit (dim, c);

  for (i = 0;  i < dim;  i++)
    c->elts[i] = a.elts[i] + b.elts[i];

#ifdef ENABLE_FLOPS
  flops += dim ;
#endif
}


/*---------------------------------------------------------------------------*/
/*!
  Subtract vector b from vector a.  Result is vector c.
*/

void vector_subtract (vector a, vector b, vector * c)
{
  register int i, dim;
  register double *aa,*bb,*cc ;

  if (a.dim != b.dim)
    matrix_error ("Incompatible dimensions for vector subtraction");

  dim = a.dim;

  vector_create_noinit (dim, c);

  aa = a.elts ; bb = b.elts ; cc = c->elts ;
  for (i = 0;  i < dim;  i++)
#if 0
    c->elts[i] = a.elts[i] - b.elts[i];
#else
    cc[i] = aa[i] - bb[i] ;
#endif

#ifdef ENABLE_FLOPS
  flops += dim ;
#endif
}

/*** for vector-matrix multiply routines below ***/

#define UNROLL_VECMUL  /* RWCox */

/*---------------------------------------------------------------------------*/
/*!
  Right multiply matrix a by vector b.  Result is vector c.
*/
void vector_multiply (matrix a, vector b, vector * c)
{
  register int rows, cols;
  register int i, j;
  register double *bb ;
  register double sum ;
#ifdef DOTP
  register double **aa , *cc ;
#else
  register double *aa ;
#endif


  if (a.cols != b.dim){
    char str[444] ;
    sprintf(str,
            "Incompatible dimensions for vector multiplication: %dx%d X %d",
            a.rows,a.cols,b.dim ) ;
    matrix_error(str) ;
  }

  rows = a.rows;
  cols = a.cols;

  vector_create_noinit (rows, c);

  if( cols <= 0 ){
    for( i=0 ; i < rows ; i++ ) c->elts[i] = 0.0 ;
    return ;
  }

  bb = b.elts ;

#ifdef MATVEC
  MATVEC( a , b , *c ) ;          /* 04 Mar 2005 */

#elif defined(DOTP)               /* vectorized */
  aa = a.elts ; cc = c->elts ;
  i = rows%2 ;
  if( i == 1 ) DOTP(cols,aa[0],bb,cc) ;
  for( ; i < rows ; i+=2 ){
    DOTP(cols,aa[i]  ,bb,cc+i    ) ;
    DOTP(cols,aa[i+1],bb,cc+(i+1)) ;
  }
#else

#ifdef UNROLL_VECMUL
  switch( cols%4 ){
    case 0:
     for (i = 0;  i < rows;  i++){
        sum = 0.0 ; aa = a.elts[i] ;
        for (j = 0;  j < cols;  j+=4 )
          sum += aa[j]*bb[j]+aa[j+1]*bb[j+1]+aa[j+2]*bb[j+2]+aa[j+3]*bb[j+3];
        c->elts[i] = sum ;
     }
    break ;
    case 1:
     for (i = 0;  i < rows;  i++){
        aa = a.elts[i] ; sum = aa[0]*bb[0] ;
        for (j = 1;  j < cols;  j+=4 )
          sum += aa[j]*bb[j]+aa[j+1]*bb[j+1]+aa[j+2]*bb[j+2]+aa[j+3]*bb[j+3];
        c->elts[i] = sum ;
     }
    break ;
    case 2:
     for (i = 0;  i < rows;  i++){
        aa = a.elts[i] ; sum = aa[0]*bb[0]+aa[1]*bb[1] ;
        for (j = 2;  j < cols;  j+=4 )
          sum += aa[j]*bb[j]+aa[j+1]*bb[j+1]+aa[j+2]*bb[j+2]+aa[j+3]*bb[j+3];
        c->elts[i] = sum ;
     }
    break ;
    case 3:
     for (i = 0;  i < rows;  i++){
        aa = a.elts[i] ; sum = aa[0]*bb[0]+aa[1]*bb[1]+aa[2]*bb[2] ;
        for (j = 3;  j < cols;  j+=4 )
          sum += aa[j]*bb[j]+aa[j+1]*bb[j+1]+aa[j+2]*bb[j+2]+aa[j+3]*bb[j+3];
        c->elts[i] = sum ;
     }
    break ;
  }
#else
    for (i = 0;  i < rows;  i++){
        sum = 0.0 ; aa = a.elts[i] ;
        for (j = 0;  j < cols;  j++ ) sum += aa[j]*bb[j] ;
        c->elts[i] = sum ;
    }
#endif /* UNROLL_VECMUL */

#endif /* MATVEC, DOTP */

#ifdef ENABLE_FLOPS
    flops += 2.0*rows*cols ;
    dotsum += rows*cols ; dotnum += rows ;
#endif
    return ;
}

/*---------------------------------------------------------------------------*/
/*!
  Compute d = c-a*b: a is a matrix; b,c,d are vectors -- RWCox
  26 Feb 2002: return value is sum of squares of d vector
*/

double vector_multiply_subtract (matrix a, vector b, vector c, vector * d)
{
  register int rows, cols;
  register int i, j;
  register double *bb ;
#ifdef DOTP
  double qsum,sum , **aa , *dd,*cc,*ee ;
#else
  register double qsum,sum, *aa ;
#endif


  if (a.cols != b.dim || a.rows != c.dim )
    matrix_error ("Incompatible dimensions for vector multiplication-subtraction");

  rows = a.rows;
  cols = a.cols;

  vector_create_noinit (rows, d);

  if( cols <= 0 ){
    qsum = 0.0 ;
    for( i=0 ; i < rows ; i++ ){
      d->elts[i] = c.elts[i] ;
      qsum += d->elts[i] * d->elts[i] ;
    }
    return qsum ;
  }

  qsum = 0.0 ; bb = b.elts ;

#ifdef DOTP                                      /* vectorized */
  aa = a.elts ; dd = d->elts ; cc = c.elts ;
  ee = (double *)malloc(sizeof(double)*rows) ;
  i  = rows%2 ;
  if( i == 1 ) DOTP(cols,aa[0],bb,ee) ;
  for( ; i < rows ; i+=2 ){
    DOTP(cols,aa[i]  ,bb,ee+i    ) ;
    DOTP(cols,aa[i+1],bb,ee+(i+1)) ;
  }
  VSUB(rows,cc,ee,dd) ;
  DOTP(rows,dd,dd,&qsum) ;
  free((void *)ee) ;
#else

#ifdef UNROLL_VECMUL
  switch( cols%4 ){
    case 0:
     for (i = 0;  i < rows;  i++){
        aa = a.elts[i] ; sum = c.elts[i] ;
        for (j = 0;  j < cols;  j+=4 )
          sum -= aa[j]*bb[j]+aa[j+1]*bb[j+1]+aa[j+2]*bb[j+2]+aa[j+3]*bb[j+3];
        d->elts[i] = sum ; qsum += sum*sum ;
     }
    break ;
    case 1:
     for (i = 0;  i < rows;  i++){
        aa = a.elts[i] ; sum = c.elts[i]-aa[0]*bb[0] ;
        for (j = 1;  j < cols;  j+=4 )
          sum -= aa[j]*bb[j]+aa[j+1]*bb[j+1]+aa[j+2]*bb[j+2]+aa[j+3]*bb[j+3];
        d->elts[i] = sum ; qsum += sum*sum ;
     }
    break ;
    case 2:
     for (i = 0;  i < rows;  i++){
        aa = a.elts[i] ; sum = c.elts[i]-aa[0]*bb[0]-aa[1]*bb[1] ;
        for (j = 2;  j < cols;  j+=4 )
          sum -= aa[j]*bb[j]+aa[j+1]*bb[j+1]+aa[j+2]*bb[j+2]+aa[j+3]*bb[j+3];
        d->elts[i] = sum ; qsum += sum*sum ;
     }
    break ;
    case 3:
     for (i = 0;  i < rows;  i++){
        aa = a.elts[i] ; sum = c.elts[i]-aa[0]*bb[0]-aa[1]*bb[1]-aa[2]*bb[2] ;
        for (j = 3;  j < cols;  j+=4 )
          sum -= aa[j]*bb[j]+aa[j+1]*bb[j+1]+aa[j+2]*bb[j+2]+aa[j+3]*bb[j+3];
        d->elts[i] = sum ; qsum += sum*sum ;
     }
    break ;
  }
#else
  for (i = 0;  i < rows;  i++){
    aa = a.elts[i] ; sum = c.elts[i] ;
    for (j = 0;  j < cols;  j++) sum -= aa[j] * bb[j] ;
    d->elts[i] = sum ; qsum += sum*sum ;
  }
#endif /* UNROLL_VECMUL */

#endif /* DOTP */

#ifdef ENABLE_FLOPS
  flops += 2.0*rows*(cols+1) ;
  dotsum += rows*cols ; dotnum += rows ;
#endif

  return qsum ;  /* 26 Feb 2003 */
}

/*---------------------------------------------------------------------------*/
/*!
  Calculate dot product of vector a with vector b.
*/

double vector_dot (vector a, vector b)
{
  register int i, dim;
  register double sum;
  register double *aa , *bb ;

  if (a.dim != b.dim)
    matrix_error ("Incompatible dimensions for vector dot product");

  dim = a.dim;

  sum = 0.0;
  aa = a.elts ; bb = b.elts ;
  for (i = 0;  i < dim;  i++)
#if 0
    sum += a.elts[i] * b.elts[i];
#else
    sum += aa[i] * bb[i] ;
#endif

#ifdef ENABLE_FLOPS
  flops += 2.0*dim ;
#endif
  return (sum);
}

/*--------------------------------------------------------------------------*/
/*!
  Calculate dot product of vector a with itself -- 28 Dec 2001, RWCox.
*/

double vector_dotself( vector a )
{
  register int i, dim;
  register double sum;
  register double *aa ;

  dim = a.dim;
  sum = 0.0;
  aa = a.elts ;
  for (i = 0;  i < dim;  i++)
#if 0
    sum += a.elts[i] * a.elts[i];
#else
    sum += aa[i] * aa[i] ;
#endif

#ifdef ENABLE_FLOPS
  flops += 2.0*dim ;
#endif
  return (sum);
}

/*---------------------------------------------------------------------------*/
/*!
  Compute the L_infinity norm of a matrix: the max absolute row sum.
*/

double matrix_norm( matrix a )
{
   int i,j , rows=a.rows, cols=a.cols ;
   double sum , smax=0.0 ;

   for (i = 0;  i < rows;  i++){
     sum = 0.0 ;
     for (j = 0;  j < cols;  j++) sum += fabs(a.elts[i][j]) ;
     if( sum > smax ) smax = sum ;
   }
#ifdef ENABLE_FLOPS
   flops += 2.0*rows*cols ;
#endif
   return smax ;
}

/*---------------------------------------------------------------------------*/
/*! Search a matrix for nearly identical column pairs, where "nearly identical"
    means they are correlated closer than 1-eps.

    Return is a pointer to an int array of the form
      [ i1 j1 i2 j2 ... -1 -1 ]
    where columns (i1,j1) are nearly the same, (i2,j2) also, etc.
    In addition:
     - A pair (i,-1) indicates that column #i is all zeros.
     - The array is terminated with the pair (-1,-1).
     - If there are no bad column pairs or all-zero columns, NULL is returned.
     - Pairs of all-zero columns are NOT reported.
     - The array should be free()-ed when you are done with it.
-----------------------------------------------------------------------------*/

int * matrix_check_columns( matrix a , double eps )  /* 14 Jul 2004 */
{
   int i,j,k , rows=a.rows , cols=a.cols ;
   int *iar=NULL , nar=0 ;
   double sumi,sumj,sumd ;

   if( eps <= 0.0 ) eps = 1.e-5 ;

   for( i=0 ; i < cols ; i++ ){
     sumi = 0.0 ;
     for( k=0 ; k < rows ; k++ ) sumi += a.elts[k][i] * a.elts[k][i] ;
     if( sumi <= 0.0 ){
       iar = (int *)realloc( (void *)iar , sizeof(int)*2*(nar+1) ) ;
       iar[2*nar] = i ; iar[2*nar+1] = -1 ; nar++ ;
       continue ;                           /* skip to next column i */
     }
     for( j=i+1 ; j < cols ; j++ ){
       sumj = sumd = 0.0 ;
       for( k=0 ; k < rows ; k++ ){
         sumj += a.elts[k][j] * a.elts[k][j] ;
         sumd += a.elts[k][j] * a.elts[k][i] ;
       }
       if( sumj > 0.0 ){
         sumd = fabs(sumd) / sqrt(sumi*sumj) ;
         if( sumd >= 1.0-eps ){
           iar = (int *)realloc( (void *)iar , sizeof(int)*2*(nar+1) ) ;
           iar[2*nar] = i ; iar[2*nar+1] = j ; nar++ ;
         }
       }
     }
   }

   if( iar != NULL ){
     iar = (int *)realloc( (void *)iar , sizeof(int)*2*(nar+1) ) ;
     iar[2*nar] = iar[2*nar+1] = -1 ;
   }

   return iar ;
}

/*---------------------------------------------------------------------------*/
/*! Return the eigenvalues of matrix X-transpose X, scaled to diagonal 1.
    The output points to a vector of doubles, of length X.cols.  This
    should be free()-ed when you are done with it.
-----------------------------------------------------------------------------*/

/* CARET
double * matrix_singvals( matrix X ) 
{
   int i,j,k , M=X.rows , N=X.cols ;
   double *a , *e , sum ;

   a = (double *) malloc( sizeof(double)*N*N ) ;
   e = (double *) malloc( sizeof(double)*N   ) ;

   for( i=0 ; i < N ; i++ ){
     for( j=0 ; j <= i ; j++ ){
       sum = 0.0 ;
       for( k=0 ; k < M ; k++ ) sum += X.elts[k][i] * X.elts[k][j] ;
       a[j+N*i] = sum ;
       if( j < i ) a[i+N*j] = sum ;
     }
   }

   for( i=0 ; i < N ; i++ ){
     if( a[i+N*i] > 0.0 ) e[i] = 1.0 / sqrt(a[i+N*i]) ;
     else                 e[i] = 1.0 ;
   }
   for( i=0 ; i < N ; i++ ){
     for( j=0 ; j < N ; j++ ) a[j+N*i] *= e[i]*e[j] ;
   }

   symeigval_double( N , a , e ) ;
   free( (void *)a ) ;
#ifdef ENABLE_FLOPS
   flops += (M+N+2.0)*N*N ;
#endif
   return e ;
}
CARET */

/*---------------------------------------------------------------------------*/

//CARETextern void svd_double( int, int, double *, double *, double *, double * ) ;


static double pseps = 1.e-16 ;

void matrix_psinv_seteps( double eps )
{
   pseps = (eps > 0.0) ? eps : 1.e-16 ;
}

/*---------------------------------------------------------------------------*/
/*! Given MxN matrix X, return the NxN matrix

       [  T  ]-1                       [  T  ]-1 T
       [ X X ]     and the NxM matrix  [ X X ]  X
-----------------------------------------------------------------------------*/

// CARET REMOVEDvoid matrix_psinvXX( matrix X , matrix *XtXinv , matrix *XtXinvXt )
