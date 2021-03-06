/*LICENSE_START*/
/*
 *  Copyright 1995-2002 Washington University School of Medicine
 *
 *  http://brainmap.wustl.edu
 *
 *  This file is part of CARET.
 *
 *  CARET is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  CARET is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CARET; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*LICENSE_END*/
/*****===================================================================*****/
/*****     Sample functions to deal with NIFTI-1 and ANALYZE files       *****/
/*****...................................................................*****/
/*****            This code is released to the public domain.            *****/
/*****...................................................................*****/
/*****  Author: Robert W Cox, SSCC/DIRP/NIMH/NIH/DHHS/USA/EARTH          *****/
/*****  Date:   August 2003                                              *****/
/*****...................................................................*****/
/*****  Neither the National Institutes of Health (NIH), nor any of its  *****/
/*****  employees imply any warranty of usefulness of this software for  *****/
/*****  any purpose, and do not assume any liability for damages,        *****/
/*****  incidental or otherwise, caused by any use of this document.     *****/
/*****===================================================================*****/

#include <cstdio>  // used for NULL
#include <cmath>
#include <sstream>

#include <QTextStream>

#include "ByteSwapping.h"
#include "NiftiHelper.h"

/**
 * Methods below are caret unique.
 */
void 
NiftiHelper::mat44ToCaretOrientation(mat44 R, 
                                     VolumeFile::ORIENTATION& xOrient,   
                                     VolumeFile::ORIENTATION& yOrient,   
                                     VolumeFile::ORIENTATION& zOrient)
{
   int x, y, z;
   nifti_mat44_to_orientation(R, &x, &y, &z);
   
   xOrient = niftiOrientationToCaretOrientation(x);
   yOrient = niftiOrientationToCaretOrientation(y);
   zOrient = niftiOrientationToCaretOrientation(z);
}

/**
 * convert NIFTI orientation to Caret orientation.
 */
VolumeFile::ORIENTATION 
NiftiHelper::niftiOrientationToCaretOrientation(const int niftiOrientation)
{
  VolumeFile::ORIENTATION orient = VolumeFile::ORIENTATION_UNKNOWN;
  
  switch(niftiOrientation) {
     case NIFTI_L2R:
        orient = VolumeFile::ORIENTATION_LEFT_TO_RIGHT;
        break;
     case NIFTI_R2L:
        orient = VolumeFile::ORIENTATION_RIGHT_TO_LEFT;
        break;
     case NIFTI_P2A:
        orient = VolumeFile::ORIENTATION_POSTERIOR_TO_ANTERIOR;
        break;
     case NIFTI_A2P:
        orient = VolumeFile::ORIENTATION_ANTERIOR_TO_POSTERIOR;
        break;
     case NIFTI_I2S:
        orient = VolumeFile::ORIENTATION_INFERIOR_TO_SUPERIOR;
        break;
     case NIFTI_S2I:
        orient = VolumeFile::ORIENTATION_SUPERIOR_TO_INFERIOR;
        break;
  }
  
  return orient;
}

void 
NiftiHelper::nifti_mat44_to_orientation( mat44 R , int *icod, int *jcod, int *kcod )
{
   float xi,xj,xk , yi,yj,yk , zi,zj,zk , val,detQ,detP ;
   mat33 P , Q , M ;
   int i,j,k=0,p,q,r , ibest,jbest,kbest,pbest,qbest,rbest ;
   float vbest ;

   if( icod == NULL || jcod == NULL || kcod == NULL ) return ; /* bad */

   *icod = *jcod = *kcod = 0 ; /* error returns, if sh*t happens */

   /* load column vectors for each (i,j,k) direction from matrix */

   /*-- i axis --*/ /*-- j axis --*/ /*-- k axis --*/

   xi = R.m[0][0] ; xj = R.m[0][1] ; xk = R.m[0][2] ;
   yi = R.m[1][0] ; yj = R.m[1][1] ; yk = R.m[1][2] ;
   zi = R.m[2][0] ; zj = R.m[2][1] ; zk = R.m[2][2] ;

   /* normalize column vectors to get unit vectors along each ijk-axis */

   /* normalize i axis */

   val = std::sqrt( xi*xi + yi*yi + zi*zi ) ;
   if( val == 0.0 ) return ;                 /* stupid input */
   xi /= val ; yi /= val ; zi /= val ;

   /* normalize j axis */

   val = std::sqrt( xj*xj + yj*yj + zj*zj ) ;
   if( val == 0.0 ) return ;                 /* stupid input */
   xj /= val ; yj /= val ; zj /= val ;

   /* orthogonalize j axis to i axis, if needed */

   val = xi*xj + yi*yj + zi*zj ;    /* dot product between i and j */
   if( std::fabs(val) > 1.e-4 ){
     xj -= val*xi ; yj -= val*yi ; zj -= val*zi ;
     val = std::sqrt( xj*xj + yj*yj + zj*zj ) ;  /* must renormalize */
     if( val == 0.0 ) return ;              /* j was parallel to i? */
     xj /= val ; yj /= val ; zj /= val ;
   }

   /* normalize k axis; if it is zero, make it the cross product i x j */

   val = std::sqrt( xk*xk + yk*yk + zk*zk ) ;
   if( val == 0.0 ){ xk = yi*zj-zi*yj; yk = zi*xj-zj*xi ; zk=xi*yj-yi*xj ; }
   else            { xk /= val ; yk /= val ; zk /= val ; }

   /* orthogonalize k to i */

   val = xi*xk + yi*yk + zi*zk ;    /* dot product between i and k */
   if( std::fabs(val) > 1.e-4 ){
     xk -= val*xi ; yk -= val*yi ; zk -= val*zi ;
     val = std::sqrt( xk*xk + yk*yk + zk*zk ) ;
     if( val == 0.0 ) return ;      /* bad */
     xk /= val ; yk /= val ; zk /= val ;
   }

   /* orthogonalize k to j */

   val = xj*xk + yj*yk + zj*zk ;    /* dot product between j and k */
   if( std::fabs(val) > 1.e-4 ){
     xk -= val*xj ; yk -= val*yj ; zk -= val*zj ;
     val = std::sqrt( xk*xk + yk*yk + zk*zk ) ;
     if( val == 0.0 ) return ;      /* bad */
     xk /= val ; yk /= val ; zk /= val ;
   }

   Q.m[0][0] = xi ; Q.m[0][1] = xj ; Q.m[0][2] = xk ;
   Q.m[1][0] = yi ; Q.m[1][1] = yj ; Q.m[1][2] = yk ;
   Q.m[2][0] = zi ; Q.m[2][1] = zj ; Q.m[2][2] = zk ;

   /* at this point, Q is the rotation matrix from the (i,j,k) to (x,y,z) axes */

   detQ = nifti_mat33_determ( Q ) ;
   if( detQ == 0.0 ) return ; /* shouldn't happen unless user is a DUFIS */

   /* Build and test all possible +1/-1 coordinate permutation matrices P;
      then find the P such that the rotation matrix M=PQ is closest to the
      identity, in the sense of M having the smallest total rotation angle. */

   /* Despite the formidable looking 6 nested loops, there are
      only 3*3*3*2*2*2 = 216 passes, which will run very quickly. */

   vbest = -666.0 ; ibest=pbest=qbest=rbest=1 ; jbest=2 ; kbest=3 ;
   for( i=1 ; i <= 3 ; i++ ){     /* i = column number to use for row #1 */
    for( j=1 ; j <= 3 ; j++ ){    /* j = column number to use for row #2 */
     if( i == j ) continue ;
      for( k=1 ; k <= 3 ; k++ ){  /* k = column number to use for row #3 */
       if( i == k || j == k ) continue ;
       P.m[0][0] = P.m[0][1] = P.m[0][2] =
        P.m[1][0] = P.m[1][1] = P.m[1][2] =
         P.m[2][0] = P.m[2][1] = P.m[2][2] = 0.0 ;
       for( p=-1 ; p <= 1 ; p+=2 ){    /* p,q,r are -1 or +1      */
        for( q=-1 ; q <= 1 ; q+=2 ){   /* and go into rows #1,2,3 */
         for( r=-1 ; r <= 1 ; r+=2 ){
           P.m[0][i-1] = p ; P.m[1][j-1] = q ; P.m[2][k-1] = r ;
           detP = nifti_mat33_determ(P) ;           /* sign of permutation */
           if( detP * detQ <= 0.0 ) continue ;  /* doesn't match sign of Q */
           M = nifti_mat33_mul(P,Q) ;

           /* angle of M rotation = 2.0*acos(0.5*sqrt(1.0+trace(M)))       */
           /* we want largest trace(M) == smallest angle == M nearest to I */

           val = M.m[0][0] + M.m[1][1] + M.m[2][2] ; /* trace */
           if( val > vbest ){
             vbest = val ;
             ibest = i ; jbest = j ; kbest = k ;
             pbest = p ; qbest = q ; rbest = r ;
           }
   }}}}}}

   /* At this point ibest is 1 or 2 or 3; pbest is -1 or +1; etc.

      The matrix P that corresponds is the best permutation approximation
      to Q-inverse; that is, P (approximately) takes (x,y,z) coordinates
      to the (i,j,k) axes.

      For example, the first row of P (which contains pbest in column ibest)
      determines the way the i axis points relative to the anatomical
      (x,y,z) axes.  If ibest is 2, then the i axis is along the y axis,
      which is direction P2A (if pbest > 0) or A2P (if pbest < 0).

      So, using ibest and pbest, we can assign the output code for
      the i axis.  Mutatis mutandis for the j and k axes, of course. */

   switch( ibest*pbest ){
     case  1: i = NIFTI_L2R ; break ;
     case -1: i = NIFTI_R2L ; break ;
     case  2: i = NIFTI_P2A ; break ;
     case -2: i = NIFTI_A2P ; break ;
     case  3: i = NIFTI_I2S ; break ;
     case -3: i = NIFTI_S2I ; break ;
   }

   switch( jbest*qbest ){
     case  1: j = NIFTI_L2R ; break ;
     case -1: j = NIFTI_R2L ; break ;
     case  2: j = NIFTI_P2A ; break ;
     case -2: j = NIFTI_A2P ; break ;
     case  3: j = NIFTI_I2S ; break ;
     case -3: j = NIFTI_S2I ; break ;
   }

   switch( kbest*rbest ){
     case  1: k = NIFTI_L2R ; break ;
     case -1: k = NIFTI_R2L ; break ;
     case  2: k = NIFTI_P2A ; break ;
     case -2: k = NIFTI_A2P ; break ;
     case  3: k = NIFTI_I2S ; break ;
     case -3: k = NIFTI_S2I ; break ;
   }

   *icod = i ; *jcod = j ; *kcod = k ; return ;
}

/*----------------------------------------------------------------------*/
/*! compute the determinant of a 3x3 matrix
*//*--------------------------------------------------------------------*/
float 
NiftiHelper::nifti_mat33_determ( mat33 R )   /* determinant of 3x3 matrix */
{
   double r11,r12,r13,r21,r22,r23,r31,r32,r33 ;
                                                       /*  INPUT MATRIX:  */
   r11 = R.m[0][0]; r12 = R.m[0][1]; r13 = R.m[0][2];  /* [ r11 r12 r13 ] */
   r21 = R.m[1][0]; r22 = R.m[1][1]; r23 = R.m[1][2];  /* [ r21 r22 r23 ] */
   r31 = R.m[2][0]; r32 = R.m[2][1]; r33 = R.m[2][2];  /* [ r31 r32 r33 ] */

   return r11*r22*r33-r11*r32*r23-r21*r12*r33
         +r21*r32*r13+r31*r12*r23-r31*r22*r13 ;
}

/*----------------------------------------------------------------------*/
/*! multiply 2 3x3 matrices
*//*--------------------------------------------------------------------*/
NiftiHelper::mat33 
NiftiHelper::nifti_mat33_mul( mat33 A , mat33 B )  /* multiply 2 3x3 matrices */
{
   mat33 C ; int i,j ;
   for( i=0 ; i < 3 ; i++ )
    for( j=0 ; j < 3 ; j++ )
      C.m[i][j] =  A.m[i][0] * B.m[0][j]
                 + A.m[i][1] * B.m[1][j]
                 + A.m[i][2] * B.m[2][j] ;
   return C ;
}

NiftiHelper::mat44 
NiftiHelper::nifti_quatern_to_mat44( float qb, float qc, float qd,
                                     float qx, float qy, float qz,
                                     float dx, float dy, float dz, float qfac )
{
   mat44 R ;
   double a,b=qb,c=qc,d=qd , xd,yd,zd ;

   /* last row is always [ 0 0 0 1 ] */

   R.m[3][0]=R.m[3][1]=R.m[3][2] = 0.0 ; R.m[3][3]= 1.0 ;

   /* compute a parameter from b,c,d */

   a = 1.0l - (b*b + c*c + d*d) ;
   if( a < 1.e-7l ){                   /* special case */
     a = 1.0l / sqrt(b*b+c*c+d*d) ;
     b *= a ; c *= a ; d *= a ;        /* normalize (b,c,d) vector */
     a = 0.0l ;                        /* a = 0 ==> 180 degree rotation */
   } else{
     a = std::sqrt(a) ;                     /* angle = 2*arccos(a) */
   }

   /* load rotation matrix, including scaling factors for voxel sizes */

   xd = (dx > 0.0) ? dx : 1.0l ;       /* make sure are positive */
   yd = (dy > 0.0) ? dy : 1.0l ;
   zd = (dz > 0.0) ? dz : 1.0l ;

   if( qfac < 0.0 ) zd = -zd ;         /* left handedness? */

   R.m[0][0] =        (a*a+b*b-c*c-d*d) * xd ;
   R.m[0][1] = 2.0l * (b*c-a*d        ) * yd ;
   R.m[0][2] = 2.0l * (b*d+a*c        ) * zd ;
   R.m[1][0] = 2.0l * (b*c+a*d        ) * xd ;
   R.m[1][1] =        (a*a+c*c-b*b-d*d) * yd ;
   R.m[1][2] = 2.0l * (c*d-a*b        ) * zd ;
   R.m[2][0] = 2.0l * (b*d-a*c        ) * xd ;
   R.m[2][1] = 2.0l * (c*d+a*b        ) * yd ;
   R.m[2][2] =        (a*a+d*d-c*c-b*b) * zd ;

   /* load offsets */

   R.m[0][3] = qx ; R.m[1][3] = qy ; R.m[2][3] = qz ;

   return R ;
}

      
/**
 * get the NIFTI intention (such as statistical parameters) information.
 */
void
NiftiHelper::getNiftiIntentionInformation(const nifti_1_header& hdr,
                                          QString& niftiIntentCodeAndParam,
                                          QString& niftiIntentName)
{
   niftiIntentName = ("NIFTI_INTENT_NOT_RECOGNIZED_CODE_"
                      + QString::number(hdr.intent_code));
   
   QString intentDescription;
   QString p1;
   QString p2;
   QString p3;
   
   switch (hdr.intent_code) {
     case NIFTI_INTENT_NONE:
        niftiIntentName = "NIFTI_INTENT_NONE";
        intentDescription = "None" ;
        p1 = "";
        p2 = "";
        p3 = "";
        break;
     case NIFTI_INTENT_CORREL:     
        niftiIntentName = "NIFTI_INTENT_CORREL";
        intentDescription = "Correlation statistic" ;
        p1 = "DOF";
        p2 = "";
        p3 = "";
        break;
     case NIFTI_INTENT_TTEST:      
        niftiIntentName = "NIFTI_INTENT_TTEST";
        intentDescription = "T-statistic" ;
        p1 = "DOF";
        p2 = "";
        p3 = "";
        break;
     case NIFTI_INTENT_FTEST:      
        niftiIntentName = "NIFTI_INTENT_FTEST";
        intentDescription = "F-statistic" ;
        p1 = "Numerator DOF";
        p2 = "Denorminator DOF";
        p3 = "";
        break;
     case NIFTI_INTENT_ZSCORE:     
        niftiIntentName = "NIFTI_INTENT_ZSCORE";
        intentDescription = "Z-score"     ;
        p1 = "";
        p2 = "";
        p3 = "";
        break;
     case NIFTI_INTENT_CHISQ:      
        niftiIntentName = "NIFTI_INTENT_CHISQ";
        intentDescription = "Chi-squared distribution" ;
        p1 = "DOF";
        p2 = "";
        p3 = "";
        break;
     case NIFTI_INTENT_BETA:       
        niftiIntentName = "NIFTI_INTENT_BETA";
        intentDescription = "Beta distribution" ;
        p1 = "a";
        p2 = "b";
        p3 = "";
        break;
     case NIFTI_INTENT_BINOM:      
        niftiIntentName = "NIFTI_INTENT_BINOM";
        intentDescription = "Binomial distribution" ;
        p1 = "Number of Trials";
        p2 = "Probability per Trial";
        p3 = "";
        break;
     case NIFTI_INTENT_GAMMA:      
        niftiIntentName = "NIFTI_INTENT_GAMMA";
        intentDescription = "Gamma distribution" ;
        p1 = "Shape";
        p2 = "Scale";
        p3 = "";
        break;
     case NIFTI_INTENT_POISSON:    
        niftiIntentName = "NIFTI_INTENT_POISSON";
        intentDescription = "Poisson distribution" ;
        p1 = "Mean";
        p2 = "";
        p3 = "";
        break;
     case NIFTI_INTENT_NORMAL:     
        niftiIntentName = "NIFTI_INTENT_NORMAL";
        intentDescription = "Normal distribution" ;
        p1 = "Mean";
        p2 = "Standard Deviation";
        p3 = "";
        break;
     case NIFTI_INTENT_FTEST_NONC: 
        niftiIntentName = "NIFTI_INTENT_FTEST_NONC";
        intentDescription = "F-statistic noncentral" ;
        p1 = "Numerator DOF";
        p2 = "Denominator DOF";
        p3 = "Numerator Noncentrality Parameter";
        break;
     case NIFTI_INTENT_CHISQ_NONC: 
        niftiIntentName = "NIFTI_INTENT_CHISQ_NONC";
        intentDescription = "Chi-squared noncentral" ;
        p1 = "DOF";
        p2 = "Noncentrality Parameter";
        p3 = "";
        break;
     case NIFTI_INTENT_LOGISTIC:   
        niftiIntentName = "NIFTI_INTENT_LOGISTIC";
        intentDescription = "Logistic distribution" ;
        p1 = "Location";
        p2 = "Scale";
        p3 = "";
        break;
     case NIFTI_INTENT_LAPLACE:    
        niftiIntentName = "NIFTI_INTENT_LAPLACE";
        intentDescription = "Laplace distribution" ;
        p1 = "Location";
        p2 = "Scale";
        p3 = "";
        break;
     case NIFTI_INTENT_UNIFORM:    
        niftiIntentName = "NIFTI_INTENT_UNIFORM";
        intentDescription = "Uniform distribition" ;
        p1 = "Lower End";
        p2 = "Upper End";
        p3 = "";
        break;
     case NIFTI_INTENT_TTEST_NONC: 
        niftiIntentName = "NIFTI_INTENT_TTEST_NONC";
        intentDescription = "T-statistic noncentral" ;
        p1 = "DOF";
        p2 = "Noncentrality Parameter";
        p3 = "";
        break;
     case NIFTI_INTENT_WEIBULL:    
        niftiIntentName = "NIFTI_INTENT_WEIBULL";
        intentDescription = "Weibull distribution" ;
        p1 = "Location";
        p2 = "Scale";
        p3 = "Power";
        break;
     case NIFTI_INTENT_CHI:        
        niftiIntentName = "NIFTI_INTENT_CHI";
        intentDescription = "Chi distribution" ;
        p1 = "Half Normal Distribution";
        p2 = "Rayleigh Distritibution";
        p3 = "Maxwell-Boltzmann Distribution";
        break;
     case NIFTI_INTENT_INVGAUSS:   
        niftiIntentName = "NIFTI_INTENT_INVGAUSS";
        intentDescription = "Inverse Gaussian distribution" ;
        p1 = "MU";
        p2 = "Lambda";
        p3 = "";
        break;
     case NIFTI_INTENT_EXTVAL:     
        niftiIntentName = "NIFTI_INTENT_EXTVAL";
        intentDescription = "Extreme Value distribution" ;
        p1 = "Location";
        p2 = "Scale";
        p3 = "";
        break;
     case NIFTI_INTENT_PVAL:       
        niftiIntentName = "NIFTI_INTENT_PVAL";
        intentDescription = "P-value" ;
        p1 = "";
        p2 = "";
        p3 = "";
        break;
     case NIFTI_INTENT_LOGPVAL:    
        niftiIntentName = "NIFTI_INTENT_LOGPVAL";
        intentDescription = "Log P-value" ;
        p1 = "";
        p2 = "";
        p3 = "";
        break;
     case NIFTI_INTENT_LOG10PVAL:  
        niftiIntentName = "NIFTI_INTENT_LOG10PVAL";
        intentDescription = "Log10 P-value" ;
        p1 = "";
        p2 = "";
        p3 = "";
        break;
     case NIFTI_INTENT_ESTIMATE:   
        niftiIntentName = "NIFTI_INTENT_ESTIMATE";
        intentDescription = "Estimate" ;
        p1 = "";
        p2 = "";
        p3 = "";
        break;
     case NIFTI_INTENT_LABEL:      
        niftiIntentName = "NIFTI_INTENT_LABEL";
        intentDescription = "Label index" ;
        p1 = "";
        p2 = "";
        p3 = "";
        break;
     case NIFTI_INTENT_NEURONAME:  
        niftiIntentName = "NIFTI_INTENT_NEURONAME";
        intentDescription = "NeuroNames index" ;
        p1 = "";
        p2 = "";
        p3 = "";
        break;
     case NIFTI_INTENT_GENMATRIX:  
        niftiIntentName = "NIFTI_INTENT_GENMATRIX";
        intentDescription = "General matrix" ;
        p1 = "";
        p2 = "";
        p3 = "";
        break;
     case NIFTI_INTENT_SYMMATRIX:  
        niftiIntentName = "NIFTI_INTENT_SYMMATRIX";
        intentDescription = "Symmetric matrix" ;
        p1 = "";
        p2 = "";
        p3 = "";
        break;
     case NIFTI_INTENT_DISPVECT:   
        niftiIntentName = "NIFTI_INTENT_DISPVECT";
        intentDescription = "Displacement vector" ;
        p1 = "";
        p2 = "";
        p3 = "";
        break;
     case NIFTI_INTENT_VECTOR:     
        niftiIntentName = "NIFTI_INTENT_VECTOR";
        intentDescription = "Vector" ;
        p1 = "";
        p2 = "";
        p3 = "";
        break;
     case NIFTI_INTENT_POINTSET:   
        niftiIntentName = "NIFTI_INTENT_POINTSET";
        intentDescription = "Pointset" ;
        p1 = "";
        p2 = "";
        p3 = "";
        break;
     case NIFTI_INTENT_TRIANGLE:   
        niftiIntentName = "NIFTI_INTENT_TRIANGLE";
        intentDescription = "Triangle" ;
        p1 = "";
        p2 = "";
        p3 = "";
        break;
     case NIFTI_INTENT_QUATERNION: 
        niftiIntentName = "NIFTI_INTENT_QUATERNION";
        intentDescription = "Quaternion" ;
        p1 = "";
        p2 = "";
        p3 = "";
        break;
     case NIFTI_INTENT_DIMLESS:    
        niftiIntentName = "NIFTI_INTENT_DIMLESS";
        intentDescription = "Dimensionless number" ;
        p1 = "";
        p2 = "";
        p3 = "";
        break;
   }

   QString s;
   
   if (intentDescription.isEmpty() == false) {
      s += intentDescription;
      
      if (p1.isEmpty() == false) {
         s += ("  "
               + p1
               + "="
               + QString::number(hdr.intent_p1, 'f', 2));
      }
      
      if (p2.isEmpty() == false) {
         s += ("  "
               + p2
               + "="
               + QString::number(hdr.intent_p2, 'f', 2));
      }
      
      if (p3.isEmpty() == false) {
         s += ("  "
               + p3
               + "="
               + QString::number(hdr.intent_p3, 'f', 2));
      }
   }
   
   niftiIntentCodeAndParam = s;
}
      
/**
 * analyze HDR file is actually a NIFTI HDR file.
 */
bool 
NiftiHelper::hdrIsNiftiFile(const QString& hdrFileName)
{
   QFile file(hdrFileName);
   if (file.open(QIODevice::ReadOnly)) {
      //
      // read bytes 348
      //
      int numBytesToRead = 348;
      char bytes[numBytesToRead];
      QDataStream stream(&file);
      const bool errorFlag = (stream.readRawData(bytes, numBytesToRead) != numBytesToRead);
      file.close();
      if (errorFlag) {
         return false;
      }
      
      //
      // Is this the NIFTI code "ni1"?
      //
      if ((bytes[344] == 'n') &&
          (bytes[345] == 'i') &&
          (bytes[346] == '1')) {
         return true;
      }
   }
   
   return false;
}

/**
 * Get the information NIFTI header.
 */
void
NiftiHelper::getNiftiHeaderInformation(const QString& fileNameIn,
                                       nifti_1_header& niftiHeaderOut,
                                       float qformMatrixOut[4][4],
                                       float sformMatrixOut[4][4],
                                       QString& headerDescriptionOut,
                                       QString& errorMessageOut)
{
   headerDescriptionOut = "";
   errorMessageOut = "";

   gzFile dataFile = gzopen(fileNameIn.toAscii().constData(), "rb");
   if (dataFile == NULL) {
      errorMessageOut = "Unable to open " + fileNameIn + "with ZLIB for reading.";
   }

   //
   // Read the NIFTI header and close file after reading header
   //
   const unsigned long headerSize = sizeof(niftiHeaderOut);
   const unsigned long numBytesRead = gzread(dataFile, (voidp)&niftiHeaderOut, headerSize);
   if (numBytesRead != headerSize) {
      gzclose(dataFile);
      std::ostringstream str;
      str << "Tried to read "
          << headerSize
          << " bytes from header.\n"
          << "Only read "
          << numBytesRead
          << ".";
      errorMessageOut = str.str().c_str();
      return;
   }

   //
   // Make sure it is a NIFTI file
   //
   const int version = NIFTI_VERSION(niftiHeaderOut);
   switch (version) {
      case 0:
         gzclose(dataFile);
         errorMessageOut = fileNameIn + "Is not a NIFTI volume file.";
         return;
         break;
      case 1:
         break;
      default:
         {
            gzclose(dataFile);
            std::ostringstream str;
            str << fileNameIn.toAscii().constData()
                << " is an invalid NIFTI version: "
                << version
                << ".";
            errorMessageOut = str.str().c_str();
            return;
         }
         break;
   }

   //
   // Do bytes need to be swapped ?
   //
   bool byteSwapFlag = false;
   if (NIFTI_NEEDS_SWAP(niftiHeaderOut)) {
      byteSwapFlag = true;

      ByteSwapping::swapBytes(&niftiHeaderOut.sizeof_hdr, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.extents, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.session_error, 1);
      ByteSwapping::swapBytes(niftiHeaderOut.dim, 8);
      ByteSwapping::swapBytes(&niftiHeaderOut.intent_p1, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.intent_p2, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.intent_p3, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.intent_code, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.datatype, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.bitpix, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.slice_start, 1);
      ByteSwapping::swapBytes(niftiHeaderOut.pixdim, 8);
      ByteSwapping::swapBytes(&niftiHeaderOut.vox_offset, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.scl_slope, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.scl_inter, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.slice_end, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.cal_max, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.cal_min, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.slice_duration, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.toffset, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.glmax, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.glmin, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.qform_code, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.sform_code, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.quatern_b, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.quatern_c, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.quatern_d, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.qoffset_x, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.qoffset_y, 1);
      ByteSwapping::swapBytes(&niftiHeaderOut.qoffset_z, 1);
      ByteSwapping::swapBytes(niftiHeaderOut.srow_x, 4);
      ByteSwapping::swapBytes(niftiHeaderOut.srow_y, 4);
      ByteSwapping::swapBytes(niftiHeaderOut.srow_z, 4);
   }


   VolumeFile::ORIENTATION qformOrientation[3] = {
      VolumeFile::ORIENTATION_UNKNOWN,
      VolumeFile::ORIENTATION_UNKNOWN,
      VolumeFile::ORIENTATION_UNKNOWN
   };
   VolumeFile::ORIENTATION sformOrientation[3] = {
      VolumeFile::ORIENTATION_UNKNOWN,
      VolumeFile::ORIENTATION_UNKNOWN,
      VolumeFile::ORIENTATION_UNKNOWN
   };

   if (niftiHeaderOut.sform_code > 0) {
      //
      // NIFTI origin is in the center of the voxel
      // so move the origin to the corner of voxel
      //
      //volumeRead.origin[0] -= volumeRead.spacing[0] * 0.5;
      //volumeRead.origin[1] -= volumeRead.spacing[1] * 0.5;
      //volumeRead.origin[2] -= volumeRead.spacing[2] * 0.5;

      NiftiHelper::mat44 m;
      m.m[0][0] = niftiHeaderOut.srow_x[0];
      m.m[0][1] = niftiHeaderOut.srow_x[1];
      m.m[0][2] = niftiHeaderOut.srow_x[2];
      m.m[0][3] = niftiHeaderOut.srow_x[3];
      m.m[1][0] = niftiHeaderOut.srow_y[0];
      m.m[1][1] = niftiHeaderOut.srow_y[1];
      m.m[1][2] = niftiHeaderOut.srow_y[2];
      m.m[1][3] = niftiHeaderOut.srow_y[3];
      m.m[2][0] = niftiHeaderOut.srow_z[0];
      m.m[2][1] = niftiHeaderOut.srow_z[1];
      m.m[2][2] = niftiHeaderOut.srow_z[2];
      m.m[2][3] = niftiHeaderOut.srow_z[3];
      m.m[3][0] = 0.0;
      m.m[3][1] = 0.0;
      m.m[3][2] = 0.0;
      m.m[3][3] = 1.0;
      NiftiHelper::mat44ToCaretOrientation(m,
                                           sformOrientation[0],
                                           sformOrientation[1],
                                           sformOrientation[2]);

      for (int i = 0; i < 4; i++) {
         for (int j = 0; j < 4; j++) {
            sformMatrixOut[i][j] = m.m[i][j];
         }
      }
   }
   if (niftiHeaderOut.qform_code > 0) {
      float qfac = (niftiHeaderOut.pixdim[0] < 0.0) ? -1.0 : 1.0 ;  /* left-handedness? */
      NiftiHelper::mat44 m =
         NiftiHelper::nifti_quatern_to_mat44(niftiHeaderOut.quatern_b, niftiHeaderOut.quatern_c, niftiHeaderOut.quatern_c,
                                             0.0, 0.0, 0.0,
                                             1.0, 1.0, 1.0,
                                             qfac);
      for (int i = 0; i < 4; i++) {
         for (int j = 0; j < 4; j++) {
            qformMatrixOut[i][j] = m.m[i][j];
         }
      }

      NiftiHelper::mat44ToCaretOrientation(m,
                                           qformOrientation[0],
                                           qformOrientation[1],
                                           qformOrientation[2]);
   }

   int precision = 3;

   QStringList keys, values;

   keys << "sizeof_hdr";
   values << QString::number(niftiHeaderOut.sizeof_hdr);

   keys << "data_type";
   QString dataTypeString;
   for (int i = 0; i < 10; i++) {
      if (niftiHeaderOut.data_type[i] == '\0') {
         break;
      }
      dataTypeString += niftiHeaderOut.data_type[i];
   }
   values << dataTypeString;

   keys << "db_name";
   QString dbNameString;
   for (int i = 0; i < 18; i++) {
      if (niftiHeaderOut.db_name[i] == '\0') {
         break;
      }
      dbNameString += niftiHeaderOut.db_name[i];
   }
   values << dbNameString;

   keys << "extents";
   values << QString::number(niftiHeaderOut.extents);

   keys << "session_error";
   values << QString::number(niftiHeaderOut.session_error);

   keys << "regular";
   values << QChar(niftiHeaderOut.regular);

   keys << "dim_info";
   values << QString::number(niftiHeaderOut.dim_info);

   keys << "dim";
   QString dimString;
   for (int i = 0; i < 8; i++) {
      dimString += QString::number(niftiHeaderOut.dim[i]) + " ";
   }
   values << dimString;

   keys << "intent_p1";
   values << QString::number(niftiHeaderOut.intent_p1, 'f', precision);

   keys << "intent_p2";
   values << QString::number(niftiHeaderOut.intent_p2, 'f', precision);

   keys << "intent_p3";
   values << QString::number(niftiHeaderOut.intent_p3, 'f', precision);

   keys << "intent_code";
   values << QString::number(niftiHeaderOut.intent_code);

   keys << "datatype";
   values << QString::number(niftiHeaderOut.datatype);

   keys << "bitpix";
   values << QString::number(niftiHeaderOut.bitpix);

   keys << "slice_start";
   values << QString::number(niftiHeaderOut.slice_start);

   keys << "pixdim";
   QString pixDimString;
   for (int i = 0; i < 8; i++) {
      pixDimString += QString::number(niftiHeaderOut.pixdim[i], 'f', precision) + " ";
   }
   values << pixDimString;

   keys << "vox_offset";
   values << QString::number(niftiHeaderOut.vox_offset, 'f', precision);

   keys << "scl_slope";
   values << QString::number(niftiHeaderOut.scl_slope, 'f', precision);

   keys << "scl_inter";
   values << QString::number(niftiHeaderOut.scl_inter, 'f', precision);

   keys << "slice_end";
   values << QString::number(niftiHeaderOut.slice_end);

   keys << "slice_code";
   values << QString::number(niftiHeaderOut.slice_code);

   keys << "xyzt_units";
   values << QString::number(niftiHeaderOut.xyzt_units);

   keys << "cal_max";
   values << QString::number(niftiHeaderOut.cal_max, 'f', precision);

   keys << "cal_min";
   values << QString::number(niftiHeaderOut.cal_min, 'f', precision);

   keys << "slice_duration";
   values << QString::number(niftiHeaderOut.slice_duration, 'f', precision);

   keys << "toffset";
   values << QString::number(niftiHeaderOut.toffset, 'f', precision);

   keys << "glmax";
   values << QString::number(niftiHeaderOut.glmax);

   keys << "glmin";
   values << QString::number(niftiHeaderOut.glmin);

   QString description;
   for (int i = 0; i < 80; i++) {
      if (niftiHeaderOut.descrip[i] == '\0') {
         break;
      }
      description += niftiHeaderOut.descrip[i];
   }
   keys << "description";
   values << description;

   QString auxFile;
   for (int i = 0; i < 24; i++) {
      if (niftiHeaderOut.aux_file[i] == '\0') {
         break;
      }
      auxFile += niftiHeaderOut.aux_file[i];
   }
   keys << "aux_file";
   values << auxFile;

   keys << "qform_code";
   values << QString::number(niftiHeaderOut.qform_code);

   keys << "sform_code";
   values << QString::number(niftiHeaderOut.sform_code);

   keys << "quatern_b";
   values << QString::number(niftiHeaderOut.quatern_b, 'f', precision);

   keys << "quatern_c";
   values << QString::number(niftiHeaderOut.quatern_c, 'f', precision);

   keys << "quatern_d";
   values << QString::number(niftiHeaderOut.quatern_d, 'f', precision);

   keys << "qoffset_x";
   values << QString::number(niftiHeaderOut.qoffset_x, 'f', precision);

   keys << "qoffset_y";
   values << QString::number(niftiHeaderOut.qoffset_y, 'f', precision);

   keys << "qoffset_z";
   values << QString::number(niftiHeaderOut.qoffset_z, 'f', precision);

   QString sRowX;
   for (int i = 0; i < 4; i++) {
      sRowX += QString::number(niftiHeaderOut.srow_x[i], 'f', precision) + " ";
   }
   keys << "srow_x";
   values << sRowX;

   QString sRowY;
   for (int i = 0; i < 4; i++) {
      sRowY += QString::number(niftiHeaderOut.srow_y[i], 'f', precision) + " ";
   }
   keys << "srow_y";
   values << sRowY;

   QString sRowZ;
   for (int i = 0; i < 4; i++) {
      sRowZ += QString::number(niftiHeaderOut.srow_z[i], 'f', precision) + " ";
   }
   keys << "srow_z";
   values << sRowZ;

   QString intentName;
   for (int i = 0; i < 16; i++) {
      if (niftiHeaderOut.intent_name[i] == '\0') {
         break;
      }
      intentName += niftiHeaderOut.intent_name[i];
   }
   keys << "intent_name";
   values << intentName;

   QString magic;
   for (int i = 0; i < 3; i++) {
      if (niftiHeaderOut.magic[i] == '\0') {
         break;
      }
      magic += niftiHeaderOut.magic[i];
   }
   keys << "magic";
   values << magic;

   //
   // Get key maximum length
   //
   int maxLength = 0;
   for (int i = 0; i < keys.count(); i++) {
      maxLength = std::max(maxLength, keys.at(i).length());
   }

   //
   // Place data into header description
   //
   for (int i = 0; i < keys.count(); i++) {
      headerDescriptionOut +=
              QString("%1: %2\n").arg(keys.at(i), maxLength).arg(values.at(i));
   }
   headerDescriptionOut += "\n";

   //
   // Intent Information
   //
   QString niftiIntentCodeAndParam;
   QString niftiIntentName;
   NiftiHelper::getNiftiIntentionInformation(niftiHeaderOut,
                                             niftiIntentCodeAndParam,
                                             niftiIntentName);
   headerDescriptionOut += ("Intent Name:      " + niftiIntentName + "\n");
   headerDescriptionOut += ("Intent Parameters:" + niftiIntentCodeAndParam + "\n");
   headerDescriptionOut += "\n";

   //
   // qform matrix and orientation
   //
   headerDescriptionOut += ("QFORM: "
                            + NiftiHelper::getQSFormCodeAsString(niftiHeaderOut.qform_code)
                            + "\n");
   for (int i = 0; i < 4; i++) {
      headerDescriptionOut += "   ";
      for (int j = 0; j < 4; j++) {
         headerDescriptionOut +=
            QString::number(qformMatrixOut[i][j], 'f', precision).rightJustified(12, ' ');
      }
      headerDescriptionOut += "\n";
   }
   headerDescriptionOut += "QFORM Orientation\n";
   for (int i = 0; i < 3; i++) {
      headerDescriptionOut += ("   "
            + VolumeFile::getOrientationLabel(qformOrientation[i]) + "\n");
   }
   headerDescriptionOut += "\n";

   //
   // sform matrix and orientation
   //
   headerDescriptionOut += ("SFORM: "
                            + NiftiHelper::getQSFormCodeAsString(niftiHeaderOut.sform_code)
                            + "\n");
   for (int i = 0; i < 4; i++) {
      headerDescriptionOut += "   ";
      for (int j = 0; j < 4; j++) {
         headerDescriptionOut +=
            QString::number(sformMatrixOut[i][j], 'f', precision).rightJustified(12, ' ');
      }
      headerDescriptionOut += "\n";
   }
   headerDescriptionOut += "SFORM Orientation\n";
   for (int i = 0; i < 3; i++) {
      headerDescriptionOut += ("   "
            + VolumeFile::getOrientationLabel(sformOrientation[i]) + "\n");
   }
   headerDescriptionOut += "\n";

   QString dataCodeString = ("Unrecognized data type code = "
                             + QString::number(niftiHeaderOut.datatype));

   switch (niftiHeaderOut.datatype) {
      case DT_UNKNOWN:
         dataCodeString = "DT_UNKNOWN";
         break;
      case DT_BINARY:
         dataCodeString = "DT_BINARY";
         break;
      case NIFTI_TYPE_UINT8:
         dataCodeString = "NIFTI_TYPE_UINT8";
         break;
      case NIFTI_TYPE_INT16:
         dataCodeString = "NIFTI_TYPE_INT16";
         break;
      case NIFTI_TYPE_INT32:
         dataCodeString = "NIFTI_TYPE_INT32";
         break;
      case NIFTI_TYPE_FLOAT32:
         dataCodeString = "NIFTI_TYPE_FLOAT32";
         break;
      case NIFTI_TYPE_COMPLEX64:
         dataCodeString = "NIFTI_TYPE_COMPLEX64";
         break;
      case NIFTI_TYPE_FLOAT64:
         dataCodeString = "NIFTI_TYPE_FLOAT64";
         break;
      case NIFTI_TYPE_RGB24:
         dataCodeString = "NIFTI_TYPE_RGB24";
         break;
      case NIFTI_TYPE_INT8:
         dataCodeString = "NIFTI_TYPE_INT8";
         break;
      case NIFTI_TYPE_UINT16:
         dataCodeString = "NIFTI_TYPE_UINT16";
         break;
      case NIFTI_TYPE_UINT32:
         dataCodeString = "NIFTI_TYPE_UINT32";
         break;
      case NIFTI_TYPE_INT64:
         dataCodeString = "NIFTI_TYPE_INT64";
         break;
      case NIFTI_TYPE_UINT64:
         dataCodeString = "NIFTI_TYPE_UINT64";
         break;
      case NIFTI_TYPE_FLOAT128:
         dataCodeString = "NIFTI_TYPE_FLOAT128";
         break;
      case NIFTI_TYPE_COMPLEX128:
         dataCodeString = "NIFTI_TYPE_COMPLEX128";
         break;
      case NIFTI_TYPE_COMPLEX256:
         dataCodeString = "NIFTI_TYPE_COMPLEX256";
         break;
   }
   headerDescriptionOut += ("Data Type: " + dataCodeString + "\n");
   headerDescriptionOut += "\n";

   const int spaceUnits = XYZT_TO_SPACE(niftiHeaderOut.xyzt_units);
   QString spaceUnitString("Unrecognized space code: "
                           + QString::number(spaceUnits));
   switch (spaceUnits) {
      case NIFTI_UNITS_UNKNOWN:
         spaceUnitString = "NIFTI_UNITS_UNKNOWN";
         break;
      case NIFTI_UNITS_METER:
         spaceUnitString = "NIFTI_UNITS_METER";
         break;
      case NIFTI_UNITS_MM:
         spaceUnitString = "NIFTI_UNITS_MM";
         break;
      case NIFTI_UNITS_MICRON:
         spaceUnitString = "NIFTI_UNITS_MICRON";
         break;
   }
   headerDescriptionOut +=("Space Units: " + spaceUnitString + "\n");

   const int timeUnits  = XYZT_TO_TIME(niftiHeaderOut.xyzt_units);
   QString timeUnitString("Unrecognized time code: "
                          + QString::number(timeUnits));
   switch (timeUnits) {
      case NIFTI_UNITS_UNKNOWN:
         timeUnitString = "NIFTI_UNITS_UNKNOWN";
         break;
      case NIFTI_UNITS_SEC:
         timeUnitString = "NIFTI_UNITS_SEC";
         break;
      case NIFTI_UNITS_MSEC:
         timeUnitString = "NIFTI_UNITS_MSEC";
         break;
      case NIFTI_UNITS_USEC:
         timeUnitString = "NIFTI_UNITS_USEC";
         break;
      case NIFTI_UNITS_HZ:
         timeUnitString = "NIFTI_UNITS_HZ";
         break;
      case NIFTI_UNITS_PPM:
         timeUnitString = "NIFTI_UNITS_PPM";
         break;
   }
   headerDescriptionOut +=("Time Units: " + timeUnitString + "\n");
   headerDescriptionOut += "\n";
}

/**
 * get a string representation of the Q/S Form code.
 */
QString
NiftiHelper::getQSFormCodeAsString(const int qsForm)
{
   QString s = "Invalid Code: " + QString::number(qsForm);

   switch (qsForm) {
      case NIFTI_XFORM_UNKNOWN:
         s = "NIFTI_XFORM_UNKNOWN";
         break;
      case NIFTI_XFORM_SCANNER_ANAT:
         s = "NIFTI_XFORM_SCANNER_ANAT";
         break;
      case NIFTI_XFORM_ALIGNED_ANAT:
         s = "NIFTI_XFORM_ALIGNED_ANAT";
         break;
      case NIFTI_XFORM_TALAIRACH:
         s = "NIFTI_XFORM_TALAIRACH";
         break;
      case NIFTI_XFORM_MNI_152:
         s = "NIFTI_XFORM_MNI_152";
         break;
   }

   return s;
}
