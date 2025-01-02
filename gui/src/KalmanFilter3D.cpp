#include "KalmanFilter3D.h"
#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

KalmanFilter3D::KalmanFilter3D() {}

KalmanFilter3D::KalmanFilter3D(double dt, double process_noise_std,
                               double measurement_noise_std) {
  Init(dt, process_noise_std, measurement_noise_std);
}

void KalmanFilter3D::Init(double dt, double process_noise_std,
                          double measurement_noise_std) {
  this->dt = dt;

  // State transition matrix (A)
  double A_temp[6][6] = {{1, 0, 0, dt, 0, 0}, {0, 1, 0, 0, dt, 0},
                         {0, 0, 1, 0, 0, dt}, {0, 0, 0, 1, 0, 0},
                         {0, 0, 0, 0, 1, 0},  {0, 0, 0, 0, 0, 1}};
  std::copy(&A_temp[0][0], &A_temp[0][0] + 6 * 6, &A[0][0]);

  // Process noise covariance matrix (Q)
  double q = process_noise_std * process_noise_std;
  double Q_temp[6][6] = {{q, 0, 0, 0, 0, 0}, {0, q, 0, 0, 0, 0},
                         {0, 0, q, 0, 0, 0}, {0, 0, 0, q, 0, 0},
                         {0, 0, 0, 0, q, 0}, {0, 0, 0, 0, 0, q}};
  std::copy(&Q_temp[0][0], &Q_temp[0][0] + 6 * 6, &Q[0][0]);

  // Measurement matrix (H)
  double H_temp[3][6] = {
      {1, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 0, 1, 0, 0, 0}};
  std::copy(&H_temp[0][0], &H_temp[0][0] + 3 * 6, &H[0][0]);

  // Measurement noise covariance matrix (R)
  double r = measurement_noise_std * measurement_noise_std;
  double R_temp[3][3] = {{r, 0, 0}, {0, r, 0}, {0, 0, r}};
  std::copy(&R_temp[0][0], &R_temp[0][0] + 3 * 3, &R[0][0]);

  // State estimate vector (x)
  std::fill(x, x + 6, 0.0);

  // Estimate covariance matrix (P)
  identityMatrix(P);
}

void KalmanFilter3D::predict() {
  // Predict the state
  double x_pred[6];
  matrixMultiply(A, x, x_pred);
  std::copy(x_pred, x_pred + 6, x);

  // Predict the estimate covariance
  double AP[6][6];
  matrixMultiply(A, P, AP);
  double At[6][6];
  matrixTranspose(A, At);
  double APAt[6][6];
  matrixMultiply(AP, At, APAt);
  matrixAdd(APAt, Q, P);
}

void KalmanFilter3D::update(const double z[3]) {
  // Compute the Kalman gain
  double HP[3][6];
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 6; ++j) {
      HP[i][j] = 0;
      for (int k = 0; k < 6; ++k) {
        HP[i][j] += H[i][k] * P[k][j];
      }
    }
  }

  double HPHt[3][3];
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      HPHt[i][j] = 0;
      for (int k = 0; k < 6; ++k) {
        HPHt[i][j] += HP[i][k] * H[j][k];
      }
    }
  }

  double S[3][3];
  matrixAdd(HPHt, R, S);

  double S_inv[3][3];
  matrixInverse(S, S_inv);

  double PHt[6][3];
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 3; ++j) {
      PHt[i][j] = 0;
      for (int k = 0; k < 3; ++k) {
        PHt[i][j] += P[i][k] * H[j][k];
      }
    }
  }

  double K[6][3];
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 3; ++j) {
      K[i][j] = 0;
      for (int k = 0; k < 3; ++k) {
        K[i][j] += PHt[i][k] * S_inv[k][j];
      }
    }
  }

  // Update the state estimate
  double y[3];
  for (int i = 0; i < 3; ++i) {
    y[i] = z[i];
    for (int j = 0; j < 6; ++j) {
      y[i] -= H[i][j] * x[j];
    }
  }

  double K_y[6];
  for (int i = 0; i < 6; ++i) {
    K_y[i] = 0;
    for (int j = 0; j < 3; ++j) {
      K_y[i] += K[i][j] * y[j];
    }
  }

  for (int i = 0; i < 6; ++i) {
    x[i] += K_y[i];
  }

  // Update the estimate covariance
  double KH[6][6];
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 6; ++j) {
      KH[i][j] = 0;
      for (int k = 0; k < 3; ++k) {
        KH[i][j] += K[i][k] * H[k][j];
      }
    }
  }

  double I[6][6];
  identityMatrix(I);

  double I_KH[6][6];
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 6; ++j) {
      I_KH[i][j] = I[i][j] - KH[i][j];
    }
  }

  double newP[6][6];
  matrixMultiply(I_KH, P, newP);
  std::copy(&newP[0][0], &newP[0][0] + 6 * 6, &P[0][0]);
}

void KalmanFilter3D::getState(double state[6]) const {
  std::copy(x, x + 6, state);
}

void KalmanFilter3D::matrixMultiply(const double A[6][6], const double B[6],
                                    double result[6]) {
  for (int i = 0; i < 6; ++i) {
    result[i] = 0;
    for (int j = 0; j < 6; ++j) {
      result[i] += A[i][j] * B[j];
    }
  }
}

void KalmanFilter3D::matrixMultiply(const double A[6][6], const double B[6][6],
                                    double result[6][6]) {
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 6; ++j) {
      result[i][j] = 0;
      for (int k = 0; k < 6; ++k) {
        result[i][j] += A[i][k] * B[k][j];
      }
    }
  }
}

void KalmanFilter3D::matrixTranspose(const double A[6][6],
                                     double result[6][6]) {
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 6; ++j) {
      result[j][i] = A[i][j];
    }
  }
}

void KalmanFilter3D::matrixAdd(const double A[6][6], const double B[6][6],
                               double result[6][6]) {
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 6; ++j) {
      result[i][j] = A[i][j] + B[i][j];
    }
  }
}

void KalmanFilter3D::identityMatrix(double I[6][6]) {
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 6; ++j) {
      if (i == j) {
        I[i][j] = 1;
      } else {
        I[i][j] = 0;
      }
    }
  }
}

void KalmanFilter3D::matrixAdd(const double A[3][3], const double B[3][3],
                               double result[3][3]) {
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      result[i][j] = A[i][j] + B[i][j];
    }
  }
}

void KalmanFilter3D::matrixSubtract(const double A[3][3], const double B[3][3],
                                    double result[3][3]) {
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      result[i][j] = A[i][j] - B[i][j];
    }
  }
}

void KalmanFilter3D::matrixInverse(const double A[3][3], double result[3][3]) {
  // Calculate the determinant of the matrix A
  double det = A[0][0] * (A[1][1] * A[2][2] - A[1][2] * A[2][1]) -
               A[0][1] * (A[1][0] * A[2][2] - A[1][2] * A[2][0]) +
               A[0][2] * (A[1][0] * A[2][1] - A[1][1] * A[2][0]);

  if (det == 0) {
    // Matrix is singular and cannot be inverted
    cerr << "Matrix inversion failed, determinant is zero." << endl;
    return;
  }

  double inv_det = 1.0 / det;

  // Calculate the inverse of the matrix A
  result[0][0] = (A[1][1] * A[2][2] - A[1][2] * A[2][1]) * inv_det;
  result[0][1] = (A[0][2] * A[2][1] - A[0][1] * A[2][2]) * inv_det;
  result[0][2] = (A[0][1] * A[1][2] - A[0][2] * A[1][1]) * inv_det;
  result[1][0] = (A[1][2] * A[2][0] - A[1][0] * A[2][2]) * inv_det;
  result[1][1] = (A[0][0] * A[2][2] - A[0][2] * A[2][0]) * inv_det;
  result[1][2] = (A[0][2] * A[1][0] - A[0][0] * A[1][2]) * inv_det;
  result[2][0] = (A[1][0] * A[2][1] - A[1][1] * A[2][0]) * inv_det;
  result[2][1] = (A[0][1] * A[2][0] - A[0][0] * A[2][1]) * inv_det;
  result[2][2] = (A[0][0] * A[1][1] - A[0][1] * A[1][0]) * inv_det;
}
