

#ifndef __MATRIX_UTILITIES_H__
#define __MATRIX_UTILITIES_H__

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

#include <vector>
#include <iostream>
#include <cmath>
using namespace std;
//because I don't want to type std:: every other line

//NOTICE: this is NOT meant to be as error friendly as matlab, it will check some things, error condition is a 0x0 matrix result, plus a line in cerr
//if a matrix has a row shorter than the first row, expect a segfault.  Calling checkDim will look for this, but it is a relatively slow operation to do on every input, so it is not used internally.

class MatrixUtilities
{
   typedef unsigned int msize_t;
   
   public:
   ///
   /// matrix multiplication
   ///
   template <typename T>
   static void multiply(const vector<vector<T> > &left, const vector<vector<T> > &right, vector<vector<T> > &result);
   
   ///
   /// reduced row echelon form
   ///
   template <typename T>
   static void rref(vector<vector<T> > &inout);
   
   ///
   /// matrix inversion - wrapper to rref for now
   ///
   template <typename T>
   static void inverse(const vector<vector<T> > &in, vector<vector<T> > &result);
   
   ///
   /// matrix addition - for simple code
   ///
   template <typename T>
   static void add(const vector<vector<T> > &left, const vector<vector<T> > &right, vector<vector<T> > &result);
   
   ///
   /// scalar addition - for simple code
   ///
   template <typename T>
   static void add(const vector<vector<T> > &left, const T &right, vector<vector<T> > &result);
   
   ///
   /// matrix subtraction - for simple code
   ///
   template <typename T>
   static void subtract(const vector<vector<T> > &left, const vector<vector<T> > &right, vector<vector<T> > &result);
   
   ///
   /// transpose - for simple code
   ///
   template <typename T>
   static void transpose(const vector<vector<T> > &in, vector<vector<T> > &result);
   
   ///
   /// debugging - verify matrix is rectangular and show its dimensions - returns true if rectangular
   ///
   template <typename T>
   static bool checkDim(const vector<vector<T> > &in);
   
   ///
   /// allocate a matrix, don't initialize
   ///
   template <typename T>
   static void resize(const int rows, const int columns, vector<vector<T> > &result, bool destructive = false);
   
   ///
   /// allocate a matrix of specified size
   ///
   template <typename T>
   static void zeros(const int rows, const int columns, vector<vector<T> > &result);
   
   ///
   /// make an identity matrix
   ///
   template <typename T>
   static void identity(const int size, vector<vector<T> > &result);
   
   ///
   /// horizontally concatenate matrices
   ///
   template <typename T>
   static void horizCat(const vector<vector<T> > &left, const vector<vector<T> > &right, vector<vector<T> > &result);
   
   ///
   /// vertically concatenate matrices
   ///
   template <typename T>
   static void vertCat(const vector<vector<T> > &top, const vector<vector<T> > &bottom, vector<vector<T> > &result);
   
   ///
   /// print a matrix
   ///
   template <typename T>
   static void display(const vector<vector<T> > &in);
   
   ///
   /// grab a piece of a matrix
   ///
   template <typename T>
   static void getChunk(const int firstrow, const int lastrow, const int firstcol, const int lastcol, const vector<vector<T> > &in, vector<vector<T> > &result);
};

template<typename T>
void MatrixUtilities::multiply(const vector<vector<T> > &left, const vector<vector<T> > &right, vector<vector<T> > &result)
{//the stupid multiply O(n^3) - the O(n^2.78) version might not be that hard to implement with the other functions here
   msize_t leftrows = left.size(), rightrows = right.size(), leftcols, rightcols;
   vector<vector<T> > tempstorage, *tresult = &result;//pointer because you can't change a reference
   bool copyout = false;
   if (&left == &result || &right == &result)
   {
      copyout = true;
      tresult = &tempstorage;
      cout << "For faster execution, don't use an input of a matrix function as the output" << endl;
   }
   if (leftrows && rightrows)
   {
      leftcols = left[0].size();
      rightcols = right[0].size();
      if (leftcols && rightcols && (rightrows == leftcols))
      {
         resize(leftrows, rightcols, (*tresult), true);//could use zeros(), but common index last lets us zero at the same time
         msize_t i, j, k;
         for (i = 0; i < leftrows; ++i)
         {
            for (j = 0; j < rightcols; ++j)
            {
               (*tresult)[i][j] = 0;
               for (k = 0; k < leftcols; ++k)
               {
                  (*tresult)[i][j] += left[i][k] * right[k][j];
               }
            }
         }
      } else {
         cerr << "multiply error condition" << endl;
         result.resize(0);
         return;
      }
   } else {
      cerr << "multiply error condition" << endl;
      result.resize(0);
      return;
   }
   if (copyout)
   {
      result = tempstorage;//simply because dereferencing might be an additional cycle
   }
}

template<typename T>
void MatrixUtilities::rref(vector<vector<T> > &inout)
{
   msize_t rows = inout.size(), cols;
   if (rows)
   {
      cols = inout[0].size();
      if (cols)
      {
         msize_t i, j, k, myrow = 0;
         int pivotrow;
         T tempval;
         vector<T> swaprow;
         for (i = 0; i < cols; ++i)
         {
            if (myrow >= rows) break;//no pivots left
            tempval = 0;
            pivotrow = -1;
            for (j = myrow; j < rows; ++j)
            {//only search below for new pivot
               if (abs(inout[j][i]) > tempval)
               {
                  pivotrow = (int)j;
                  tempval = abs(inout[j][i]);
               }
            }
            if (pivotrow == -1)//it may be a good idea to include a "very small value" check here, but it could mess up if used on a matrix with all values very small
            {//naively expect linearly dependence to show as an exact zero
               continue;//move to the next column
            }
            swaprow = inout[pivotrow];//STL copy swap
            inout[pivotrow] = inout[myrow];
            inout[myrow] = swaprow;
            tempval = inout[myrow][i];
            inout[myrow][i] = 1;
            for (j = i + 1; j < cols; ++j)
            {
               inout[myrow][j] /= tempval;//divide row by pivot
            }
            for (j = 0; j < myrow; ++j)
            {//zero above pivot
               tempval = inout[j][i];
               inout[j][i] = 0;
               for (k = i + 1; k < cols; ++k)
               {
                  inout[j][k] -= tempval * inout[myrow][k];
               }
            }
            for (j = myrow + 1; j < rows; ++j)
            {//zero below pivot
               tempval = inout[j][i];
               inout[j][i] = 0;
               for (k = i + 1; k < cols; ++k)
               {
                  inout[j][k] -= tempval * inout[myrow][k];
               }
            }
            ++myrow;//increment row on successful pivot
         }
      } else {
         cerr << "rref error condition" << endl;
         inout.resize(0);
         return;
      }
   } else {
      cerr << "rref error condition" << endl;
      inout.resize(0);
      return;
   }
}

template<typename T>
void MatrixUtilities::inverse(const vector<vector<T> > &in, vector<vector<T> > &result)
{//rref implementation, there are faster (more complicated) ways - if it isn't invertible, it will hand back something strange
   msize_t inrows = in.size(), incols;
   if (inrows)
   {
      incols = in[0].size();
      if (incols == inrows)
      {
         vector<vector<T> > inter, inter2;
         identity(incols, inter2);
         horizCat(in, inter2, inter);
         rref(inter);
         getChunk(0, inrows, incols, incols * 2, inter, result);//already using a local variable, doesn't need to check for reference duplicity
      } else {
         cerr << "inverse error condition" << endl;
         result.resize(0);
         return;
      }
   } else {
      cerr << "inverse error condition" << endl;
      result.resize(0);
      return;
   }
}

template<typename T>
void MatrixUtilities::add(const vector<vector<T> > &left, const vector<vector<T> > &right, vector<vector<T> > &result)
{
   msize_t inrows = left.size(), incols;
   vector<vector<T> > tempstorage, *tresult = &result;
   bool copyout = false;
   if (&left == &result || &right == &result)
   {
      copyout = true;
      tresult = &tempstorage;
      cout << "For faster execution, don't use an input of a matrix function as the output" << endl;
   }
   if (inrows)
   {
      incols = left[0].size();
      if (inrows == right.size() && incols == right[0].size())//short circuit evaluation will protect against segfault
      {
         resize(inrows, incols, (*tresult), true);
         for (msize_t i = 0; i < inrows; ++i)
         {
            for (msize_t j = 0; j < incols; ++j)
            {
               (*tresult)[i][j] = left[i][j] + right[i][j];
            }
         }
      } else {
         cerr << "add error condition" << endl;
         result.resize(0);//use empty matrix for error condition
         return;
      }
   } else {
      cerr << "add error condition" << endl;
      result.resize(0);
      return;
   }
   if (copyout)
   {
      result = tempstorage;
   }
}

template<typename T>
void MatrixUtilities::add(const vector<vector<T> > &left, const T &right, vector<vector<T> > &result)
{
   msize_t inrows = left.size(), incols;
   vector<vector<T> > tempstorage, *tresult = &result;
   bool copyout = false;
   if (&left == &result)
   {
      copyout = true;
      tresult = &tempstorage;
      cout << "For faster execution, don't use an input of a matrix function as the output" << endl;
   }
   if (inrows)
   {
      incols = left[0].size();
      resize(inrows, incols, (*tresult), true);
      for (msize_t i = 0; i < inrows; ++i)
      {
         for (msize_t j = 0; j < incols; ++j)
         {
            (*tresult)[i][j] = left[i][j] + right;
         }
      }
   } else {
      cerr << "add error condition" << endl;
      result.resize(0);
      return;
   }
   if (copyout)
   {
      result = tempstorage;
   }
}

template<typename T>
void MatrixUtilities::subtract(const vector<vector<T> > &left, const vector<vector<T> > &right, vector<vector<T> > &result)
{
   msize_t inrows = left.size(), incols;
   vector<vector<T> > tempstorage, *tresult = &result;
   bool copyout = false;
   if (&left == &result || &right == &result)
   {
      copyout = true;
      tresult = &tempstorage;
      cout << "For faster execution, don't use an input of a matrix function as the output" << endl;
   }
   if (inrows)
   {
      incols = left[0].size();
      if (inrows == right.size() && incols == right[0].size())//short circuit evaluation will protect against segfault
      {
         resize(inrows, incols, (*tresult), true);
         for (msize_t i = 0; i < inrows; ++i)
         {
            for (msize_t j = 0; j < incols; ++j)
            {
               (*tresult)[i][j] = left[i][j] - right[i][j];
            }
         }
      } else {
         cerr << "subtract error condition" << endl;
         result.resize(0);
         return;
      }
   } else {
      cerr << "subtract error condition" << endl;
      result.resize(0);
      return;
   }
   if (copyout)
   {
      result = tempstorage;
   }
}

template<typename T>
void MatrixUtilities::transpose(const vector<vector<T> > &in, vector<vector<T> > &result)
{
   msize_t inrows = in.size(), incols;
   vector<vector<T> > tempstorage, *tresult = &result;
   bool copyout = false;
   if (&in == &result)
   {
      copyout = true;
      tresult = &tempstorage;
      cout << "For faster execution, don't use an input of a matrix function as the output" << endl;
   }
   if (inrows)
   {
      incols = in[0].size();
      resize(incols, inrows, (*tresult), true);
      for (msize_t i = 0; i < inrows; ++i)
      {
         for (msize_t j = 0; j < incols; ++j)
         {
            (*tresult)[j][i] = in[i][j];
         }
      }
   } else {
      cerr << "transpose error condition" << endl;
      result.resize(0);
   }
   if (copyout)
   {
      result = tempstorage;
   }
}

template<typename T>
bool MatrixUtilities::checkDim(const vector<vector<T> > &in)
{
   bool ret = true;
   msize_t rows = in.size(), columns;
   cout << "Matrix is " << rows << " rows by ";
   if (rows)
   {
      columns = in[0].size();
      cout << columns << " columns" << endl;
      for (msize_t i = 1; i < rows; ++i)
      {
         if (in[i].size() != columns)
         {
            ret = false;
            cerr << "ERROR: row " << i << " has " << in[i].size() << " columns" << endl;
         }
      }
   } else {
      cout << "0 columns" << endl;
   }
   return ret;
}

template<typename T>
void MatrixUtilities::resize(const int rows, const int columns, vector<vector<T> > &result, bool destructive)
{
   if (destructive && result.size() && ((int)result.capacity() < rows || (int)result[0].capacity() < columns))
   {//for large matrices, copying to preserve contents is slow
      result.resize(0);//not intended to dealloc, just to set number of items to copy to zero
   }//default is nondestructive resize, copies everything
   result.resize(rows);
   for (msize_t i = 0; i < (const msize_t)rows; ++i)
   {//naive method, may end up copying everything twice if both row and col resizes require realloc
      result[i].resize(columns);
   }
}

template<typename T>
void MatrixUtilities::zeros(const int rows, const int columns, vector<vector<T> > &result)
{
   resize(rows, columns, result, true);
   for (msize_t i = 0; i < rows; ++i)
   {
      for (msize_t j = 0; j < columns; ++j)
      {
         result[i][j] = 0;//should cast to float or double fine
      }
   }
}

template<typename T>
void MatrixUtilities::identity(const int size, vector<vector<T> > &result)
{
   resize(size, size, result, true);
   for (msize_t i = 0; i < (const msize_t)size; ++i)
   {
      for (msize_t j = 0; j < (const msize_t)size; ++j)
      {
         result[i][j] = ((i == j) ? 1 : 0);//ditto, forgive the ternary
      }
   }
}

template<typename T>
void MatrixUtilities::horizCat(const vector<vector<T> > &left, const vector<vector<T> > &right, vector<vector<T> > &result)
{
   msize_t inrows = left.size(), leftcols, rightcols;
   vector<vector<T> > tempstorage, *tresult = &result;
   bool copyout = false;
   if (&left == &result || &right == &result)
   {
      copyout = true;
      tresult = &tempstorage;
      cout << "For faster execution, don't use an input of a matrix function as the output" << endl;
   }
   if (inrows && inrows == right.size())
   {
      leftcols = left[0].size();
      rightcols = right[0].size();
      (*tresult) = left;//use STL copy to start
      resize(inrows, leftcols + rightcols, (*tresult));//values survive nondestructive resize
      for (msize_t i = 0; i < inrows; ++i)
      {
         for (msize_t j = 0; j < rightcols; ++j)
         {
            (*tresult)[i][j + leftcols] = right[i][j];
         }
      }
   } else {
      cerr << "horizCat error condition" << endl;
      result.resize(0);
      return;
   }
   if (copyout)
   {
      result = tempstorage;
   }
}

template<typename T>
void MatrixUtilities::vertCat(const vector<vector<T> > &top, const vector<vector<T> > &bottom, vector<vector<T> > &result)
{
   msize_t toprows = top.size(), botrows = bottom.size(), incols;
   vector<vector<T> > tempstorage, *tresult = &result;
   bool copyout = false;
   if (&top == &result || &bottom == &result)
   {
      copyout = true;
      tresult = &tempstorage;
      cout << "For faster execution, don't use an input of a matrix function as the output" << endl;
   }
   if (toprows && botrows)
   {
      incols = top[0].size();
      if (incols == bottom[0].size())
      {
         (*tresult) = top;
         resize(toprows + botrows, incols, (*tresult));//nondestructive resize
         for (msize_t i = 0; i < botrows; ++i)
         {
            for (msize_t j = 0; j < incols; ++j)
            {
               (*tresult)[i + toprows][j] = bottom[i][j];
            }
         }
      } else {
         cerr << "vertCat error condition" << endl;
         result.resize(0);
         return;
      }
   } else {
      cerr << "vertCat error condition" << endl;
      result.resize(0);
      return;
   }
   if (copyout)
   {
      result = tempstorage;
   }
}

template<typename T>
void MatrixUtilities::display(const vector<vector<T> > &in)
{
   msize_t inrows = in.size(), incols;
   if (inrows)
   {
      incols = in[0].size();
      cout << inrows << " x " << incols << " matrix";
      if (incols)
      {
         cout << ":";
         for (msize_t i = 0; i < inrows; ++i)
         {
            cout << endl;
            for (msize_t j = 0; j < incols; ++j)
            {
               cout << " " << in[i][j];
            }
         }
      }
      cout << endl;
   } else {
      cout << "0 x 0 matrix" << endl;
   }
}

template<typename T>
void MatrixUtilities::getChunk(const int firstrow, const int lastrow, const int firstcol, const int lastcol, const vector<vector<T> > &in, vector<vector<T> > &result)
{
   msize_t outrows = lastrow - firstrow;
   msize_t outcols = lastcol - firstcol;
   if (lastrow <= firstrow || lastcol <= firstcol || firstrow < 0 || firstcol < 0 || lastrow > (int)in.size() || lastcol > (int)in[0].size())
   {
      cerr << "getChunk error condition" << endl;
      result.resize(0);
      return;
   }
   vector<vector<T> > tempstorage, *tresult = &result;
   bool copyout = false;
   if (&in == &result)
   {
      copyout = true;
      tresult = &tempstorage;
      cout << "For faster execution, don't use an input of a matrix function as the output" << endl;
   }
   resize(outrows, outcols, (*tresult), true);
   for (msize_t i = 0; i < outrows; ++i)
   {
      for (msize_t j = 0; j < outcols; ++j)
      {
         (*tresult)[i][j] = in[i + firstrow][j + firstcol];
      }
   }
   if (copyout)
   {
      result = tempstorage;
   }
}

#endif

