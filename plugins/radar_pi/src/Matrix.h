/******************************************************************************
 * Project:  OpenCPN
 * Purpose:  Radar Plugin
 * Authors:  Kees Verruijt, Blaz Bratanic
 ***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 *
 * Matrix class derived from http://codereview.stackexchange.com/questions/43548/fixed-size-matrix-implementation
 *
 */

#ifndef _MATRIX_H_
#define _MATRIX_H_

#include <cstdlib>
#include <iostream>
#include "pi_common.h"

PLUGIN_BEGIN_NAMESPACE

template <typename Ty, int N, int M = N>
struct Matrix {
  typedef Ty value_type;

  union {
    struct {
      Ty element[N][M];
    };
    struct {
      Ty flatten[N * M];
    };
  };

  // Access with bounds checking
  Ty& operator()(const int r, const int c) {
    assert(r >= 0 && r < N);
    assert(c >= 0 && c < M);
    return element[r][c];
  }

  const Ty operator()(const int r, const int c) const {
    assert(r >= 0 && r < N);
    assert(c >= 0 && c < M);
    return element[r][c];
  }

  // Return matrix transpose
  Matrix<Ty, M, N> Transpose() const {
    Matrix<Ty, M, N> result;
    for (int r = 0; r < N; ++r) {
      for (int c = 0; c < M; ++c) {
        result.element[c][r] = element[r][c];
      }
    }
    return result;
  }

  // Return matrix initialized to value
  Matrix<Ty, M, N> Init(Ty value) const {
    Matrix<Ty, M, N> result;
    for (int e = 0; e < M * N; ++e) {
      result.flatten[e] = Ty(value);
    }
    return result;
  }

  // Return matrix inverse
  Matrix<Ty, N, M> Inverse();

  Matrix<Ty, N, N> Identity() {
    Matrix<Ty, N, N> result = Matrix<Ty, N, N>();
    for (int i = 0; i < N * N; ++i) result.flatten[i] = Ty(0);
    for (int i = 0; i < N; ++i) result.element[i][i] = Ty(1);
    return result;
  }
};

///
// Matrix Inverse
///
// A design choice was made to keep Matrix an aggregate class to enable
// Matrix<float, 2, 2> = {1.0f, 1.0f, 0.5f, 0.2f} initialization.
// With c++11 it would be possible to create a Matrix base class
// and derive all variations from it, while retaining the brace
// initialization.
//
// Matrix inverse helpers
namespace detail {
template <typename Ty, int N, int M>
struct inverse;

// Matrix inversion for 2x2 matrix
template <typename Ty>
struct inverse<Ty, 2, 2> {
  Matrix<Ty, 2, 2> operator()(const Matrix<Ty, 2, 2>& a) {
    Matrix<Ty, 2, 2> result;
    Ty det = a.element[0][0] * a.element[1][1] - a.element[0][1] * a.element[1][0];
    assert(det != 0);

    result.element[0][0] = a.element[1][1] / det;
    result.element[1][1] = a.element[0][0] / det;
    result.element[0][1] = -a.element[0][1] / det;
    result.element[1][0] = -a.element[1][0] / det;
    return result;
  }
};

}  // namespace detail

// Define matrix inverse
template <typename Ty, int N, int M>
Matrix<Ty, N, M> Matrix<Ty, N, M>::Inverse() {
  return detail::inverse<Ty, N, M>()(*this);
}

///
//  Matrix operations
///
// Matrix product
template <typename Ty, int N, int M, int P>
Matrix<Ty, N, P> operator*(const Matrix<Ty, N, M>& a, const Matrix<Ty, M, P>& b) {
  Matrix<Ty, N, P> result;

  for (int r = 0; r < N; ++r) {
    for (int c = 0; c < P; ++c) {
      Ty accum = Ty(0);
      for (int i = 0; i < M; ++i) {
        accum += a.element[r][i] * b.element[i][c];
      }
      result.element[r][c] = accum;
    }
  }
  return result;
}

// Unary negation
template <typename Ty, int N, int M>
Matrix<Ty, N, M> operator-(const Matrix<Ty, N, M>& a) {
  Matrix<Ty, N, M> result;
  for (int e = 0; e < N * M; ++e) result.flatten[e] = -a.flatten[e];
  return result;
}

#define MATRIX_WITH_MATRIX_OPERATOR(op_symbol, op)                                            \
  template <typename Ty, int N, int M>                                                        \
  Matrix<Ty, N, M> operator op_symbol(const Matrix<Ty, N, M>& a, const Matrix<Ty, N, M>& b) { \
    Matrix<Ty, N, M> result;                                                                  \
    for (int e = 0; e < N * M; ++e) result.flatten[e] = a.flatten[e] op b.flatten[e];         \
    return result;                                                                            \
  }

MATRIX_WITH_MATRIX_OPERATOR(+, +);
MATRIX_WITH_MATRIX_OPERATOR(-, -);
#undef MATRIX_WITH_MATRIX_OPERATOR

#define MATRIX_WITH_SCALAR_OPERATOR(op_symbol, op)                              \
  template <typename Ty, int N, int M>                                          \
  Matrix<Ty, N, M> operator op_symbol(const Matrix<Ty, N, M>& a, Ty scalar) {   \
    Matrix<Ty, N, M> result;                                                    \
    for (int e = 0; e < N * M; ++e) result.flatten[e] = a.flatten[e] op scalar; \
    return result;                                                              \
  }

MATRIX_WITH_SCALAR_OPERATOR(+, +);
MATRIX_WITH_SCALAR_OPERATOR(-, -);
MATRIX_WITH_SCALAR_OPERATOR(*, *);
MATRIX_WITH_SCALAR_OPERATOR(/, /);
#undef MATRIX_WITH_SCALAR_OPERATOR

template <typename Ty, int N, int M>
Matrix<Ty, N, M> operator+(Ty scalar, const Matrix<Ty, N, M>& a) {
  return a + scalar;
}

template <typename Ty, int N, int M>
Matrix<Ty, N, M> operator*(Ty scalar, const Matrix<Ty, N, M>& a) {
  return a * scalar;
}

template <typename Ty, int N, int M>
Matrix<Ty, N, M> operator-(Ty scalar, const Matrix<Ty, N, M>& a) {
  return -a + scalar;
}

PLUGIN_END_NAMESPACE
#endif
