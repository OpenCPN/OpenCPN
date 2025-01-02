#ifndef KALMANFILTER3D_H
#define KALMANFILTER3D_H

/**
 * @file KalmanFilter3D.h
 * @brief Header file for the KalmanFilter3D class.
 *
 * Created by: HAMDI BOUKAMCHA
 * Date: 06/01/2024
 */

class KalmanFilter3D {
public:
  KalmanFilter3D();
  /**
   * @brief Constructor for KalmanFilter3D.
   * @param dt Time step.
   * @param process_noise_std Standard deviation of the process noise.
   * @param measurement_noise_std Standard deviation of the measurement noise.
   */
  KalmanFilter3D(double dt, double process_noise_std,
                 double measurement_noise_std);

  void Init(double dt, double process_noise_std, double measurement_noise_std);

  /**
   * @brief Predicts the next state of the system.
   */
  void predict();

  /**
   * @brief Updates the state with a new measurement.
   * @param z Array of measurements.
   */
  void update(const double z[3]);

  /**
   * @brief Gets the current state.
   * @param state Array to store the current state.
   */
  void getState(double state[6]) const;

private:
  double dt;       ///< Time step.
  double A[6][6];  ///< State transition matrix.
  double Q[6][6];  ///< Process noise covariance matrix.
  double H[3][6];  ///< Measurement matrix.
  double R[3][3];  ///< Measurement noise covariance matrix.
  double P[6][6];  ///< Estimate error covariance matrix.
  double x[6];     ///< State vector.

  /**
   * @brief Multiplies a 6x6 matrix with a 6x1 vector.
   * @param A 6x6 matrix.
   * @param B 6x1 vector.
   * @param result 6x1 result vector.
   */
  void matrixMultiply(const double A[6][6], const double B[6],
                      double result[6]);

  /**
   * @brief Multiplies two 6x6 matrices.
   * @param A First 6x6 matrix.
   * @param B Second 6x6 matrix.
   * @param result 6x6 result matrix.
   */
  void matrixMultiply(const double A[6][6], const double B[6][6],
                      double result[6][6]);

  /**
   * @brief Transposes a 6x6 matrix.
   * @param A 6x6 matrix.
   * @param result 6x6 result matrix.
   */
  void matrixTranspose(const double A[6][6], double result[6][6]);

  /**
   * @brief Adds two 6x6 matrices.
   * @param A First 6x6 matrix.
   * @param B Second 6x6 matrix.
   * @param result 6x6 result matrix.
   */
  void matrixAdd(const double A[6][6], const double B[6][6],
                 double result[6][6]);

  /**
   * @brief Sets a 6x6 matrix to the identity matrix.
   * @param I 6x6 matrix to be set to identity.
   */
  void identityMatrix(double I[6][6]);

  /**
   * @brief Adds two 3x3 matrices.
   * @param A First 3x3 matrix.
   * @param B Second 3x3 matrix.
   * @param result 3x3 result matrix.
   */
  void matrixAdd(const double A[3][3], const double B[3][3],
                 double result[3][3]);

  /**
   * @brief Subtracts one 3x3 matrix from another.
   * @param A First 3x3 matrix.
   * @param B Second 3x3 matrix.
   * @param result 3x3 result matrix.
   */
  void matrixSubtract(const double A[3][3], const double B[3][3],
                      double result[3][3]);

  /**
   * @brief Computes the inverse of a 3x3 matrix.
   * @param A 3x3 matrix.
   * @param result 3x3 result matrix (inverse of A).
   */
  void matrixInverse(const double A[3][3], double result[3][3]);
};

#endif  // KALMANFILTER3D_H
