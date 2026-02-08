#include "rm_auto_aim/solver/utils/extended_kalman_filter.hpp"

#include <cmath>

namespace rm_auto_aim {

ExtendedKalmanFilter::ExtendedKalmanFilter(int n_states, int n_obs)
    : n_states_(n_states), n_obs_(n_obs)
{
    x_ = VecX::Zero(n_states);
    P_ = MatXX::Identity(n_states, n_states);
    Q_ = MatXX::Identity(n_states, n_states);
    R_ = MatXX::Identity(n_obs, n_obs);
}

void ExtendedKalmanFilter::setFunctions(PredictFunc f, MeasureFunc h) {
    f_ = std::move(f);
    h_ = std::move(h);
}

void ExtendedKalmanFilter::setNoiseMatrices(const MatXX& Q, const MatXX& R) {
    Q_ = Q;
    R_ = R;
}

void ExtendedKalmanFilter::init(const VecX& x0) {
    x_ = x0;
    P_ = MatXX::Identity(n_states_, n_states_) * 0.1;
    initialized_ = true;
}

ExtendedKalmanFilter::VecX ExtendedKalmanFilter::predict(double dt) {
    if (!initialized_) return x_;

    // 计算雅可比矩阵 F
    MatXX F = computeF(x_, dt);

    // 状态预测
    x_ = f_(x_, dt);

    // 协方差预测
    P_ = F * P_ * F.transpose() + Q_;

    return x_;
}

ExtendedKalmanFilter::VecX ExtendedKalmanFilter::update(const Eigen::Vector4d& z) {
    if (!initialized_) return x_;

    // 计算观测雅可比 H
    auto H = computeH(x_);

    // 创新（残差）
    Eigen::Vector4d y = z - h_(x_);

    // 将yaw残差归一化到[-pi, pi]
    while (y(3) > M_PI) y(3) -= 2 * M_PI;
    while (y(3) < -M_PI) y(3) += 2 * M_PI;

    // 创新协方差
    Eigen::Matrix4d S = H * P_ * H.transpose() + R_;

    // 卡尔曼增益
    auto K = P_ * H.transpose() * S.inverse();

    // 状态更新
    x_ = x_ + K * y;

    // 协方差更新 (Joseph 形式提高数值稳定性)
    MatXX I = MatXX::Identity(n_states_, n_states_);
    MatXX IKH = I - K * H;
    P_ = IKH * P_ * IKH.transpose() + K * R_ * K.transpose();

    return x_;
}

ExtendedKalmanFilter::MatXX ExtendedKalmanFilter::computeF(const VecX& x, double dt) {
    // 数值微分计算雅可比
    MatXX F = MatXX::Identity(n_states_, n_states_);
    const double eps = 1e-5;

    VecX f0 = f_(x, dt);
    for (int i = 0; i < n_states_; i++) {
        VecX x_perturbed = x;
        x_perturbed(i) += eps;
        VecX f1 = f_(x_perturbed, dt);
        F.col(i) = (f1 - f0) / eps;
    }

    return F;
}

Eigen::Matrix<double, 4, Eigen::Dynamic> ExtendedKalmanFilter::computeH(const VecX& x) {
    Eigen::Matrix<double, 4, Eigen::Dynamic> H =
        Eigen::Matrix<double, 4, Eigen::Dynamic>::Zero(4, n_states_);
    const double eps = 1e-5;

    Eigen::Vector4d h0 = h_(x);
    for (int i = 0; i < n_states_; i++) {
        VecX x_perturbed = x;
        x_perturbed(i) += eps;
        Eigen::Vector4d h1 = h_(x_perturbed);
        H.col(i) = (h1 - h0) / eps;
    }

    return H;
}

}  // namespace rm_auto_aim
