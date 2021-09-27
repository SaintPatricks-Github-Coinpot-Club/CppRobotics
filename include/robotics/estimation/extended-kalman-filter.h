#pragma once

#include <robotics/common.h>
#include <robotics/system/nonlinear-system.h>

#include <Eigen/Dense>
#include <cmath>
#include <vector>

namespace Robotics::Estimation {

    /**
     * @brief A class for implemeting an Extended Kalman Filter
     */
    template <int StateSize, int InputSize, int OutputSize> class EKF {
        using State = ColumnVector<StateSize>;
        using Input = ColumnVector<InputSize>;
        using Measurement = ColumnVector<OutputSize>;

        using NonlinearSystem = Robotics::Model::NonlinearSystem<StateSize, InputSize, OutputSize>;

      public:
        /**
         * @brief Creates a new LQR path planner
         * @param A state matrix
         * @param B control matrix
         * @param Q state weights matrix
         * @param R control weights matrix
         */
        EKF(NonlinearSystem system, SquareMatrix<StateSize> Q, SquareMatrix<OutputSize> R)
            : system(system), Q(Q), R(R)
        {
        }

        State Update(State previous_estimate, Measurement z, Input u)
        {
            // Predicted state estimate
            x_predicted = system.PropagateDynamics(previous_estimate, u);

            // Predicted covariance estimate
            SquareMatrix<StateSize> J_F = system.GetJacobian(u);
            P_predicted = J_F * P_estimate * J_F.transpose() + Q;

            // Update
            z_predicted = system.GetOutputMatrix() * x_predicted;
            residual = z - z_predicted;

            const Matrix<OutputSize, StateSize>& J_H = system.GetOutputMatrixJacobian();
            S = J_H * P_predicted * J_H.transpose() + R;
            K = P_predicted * J_H.transpose() * S.inverse();
            x_estimate = x_predicted + K * residual;
            P_estimate = (SquareMatrix<StateSize>::Identity() - K * J_H) * P_predicted;

            return x_estimate;
        }

        void SetTimeStep(double step) { dt = step; }

      private:
        NonlinearSystem system;

        double dt{0.1};

        SquareMatrix<StateSize> P_predicted, P_estimate;
        SquareMatrix<OutputSize> S;
        Robotics::Matrix<StateSize, OutputSize> K;

        State x_predicted, x_estimate;
        Measurement z_predicted, residual;

        // State covariance
        const SquareMatrix<StateSize> Q;

        // Observation covariance
        const SquareMatrix<OutputSize> R;
    };

}  // namespace Robotics::Estimation