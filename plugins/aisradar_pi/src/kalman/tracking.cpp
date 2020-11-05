#include "Eigen/Dense"
#include <iostream>
#include "tracking.h"

using namespace std;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::vector;

Tracking::Tracking() {
  is_initialized_ = false;
  previous_timestamp_ = 0;

  //create a 4D state vector, we don't know yet the values of the x state
  kf_.x_ = VectorXd(4);

  //state covariance matrix P
  kf_.P_ = MatrixXd(4, 4);
  kf_.P_ << 1, 0, 0, 0,
            0, 1, 0, 0, 
            0, 0, 1000, 0,
            0, 0, 0, 1000;


  //measurement covariance
  kf_.R_ = MatrixXd(2, 2);
  kf_.R_ << 0.0225, 0,
            0,      0.0225;

  //measurement matrix
  kf_.H_ = MatrixXd(2, 4);
  kf_.H_ << 1, 0, 0, 0,
            0, 1, 0, 0;

  //the initial transition matrix F_
  kf_.F_ = MatrixXd(4, 4);
  kf_.F_ << 1, 0, 1, 0,
            0, 1, 0, 1,
            0, 0, 1, 0,
            0, 0, 0, 1;

  //set the acceleration noise components
  noise_ax = 5;
  noise_ay = 5;

}

Tracking::~Tracking() {

}

// Process a single measurement
void Tracking::ProcessMeasurement(const MeasurementPackage &measurement_pack) {
  if (!is_initialized_) {
    //cout << "Kalman Filter Initialization " << endl;

    //set the state with the initial location and zero velocity
    kf_.x_ << measurement_pack.raw_measurements_[0], measurement_pack.raw_measurements_[1], 0, 0;

    previous_timestamp_ = measurement_pack.timestamp_;
    is_initialized_ = true;
    return;
  }

  //compute the time elapsed between the current and previous measurements
  float dt = (measurement_pack.timestamp_ - previous_timestamp_) / 1000000.0;	//dt - expressed in seconds
  previous_timestamp_ = measurement_pack.timestamp_;

  float dt_2 = dt * dt;
  float dt_3 = dt_2 * dt;
  float dt_4 = dt_3 * dt;

  //Modify the F matrix so that the time is integrated
  kf_.F_(0, 2) = dt;
  kf_.F_(1, 3) = dt;

  //set the process covariance matrix Q
  kf_.Q_ = MatrixXd(4, 4);
  kf_.Q_ <<  dt_4/4*noise_ax, 0,               dt_3/2*noise_ax, 0,
             0,               dt_4/4*noise_ay, 0,               dt_3/2*noise_ay,
             dt_3/2*noise_ax, 0,               dt_2*noise_ax,   0,
             0,               dt_3/2*noise_ay, 0,               dt_2*noise_ay;

  //predict
  kf_.Predict();

  //measurement update
  kf_.Update(measurement_pack.raw_measurements_);

  std::cout << "x_= " << kf_.x_ << std::endl;
  std::cout << "P_= " << kf_.P_ << std::endl;

}

