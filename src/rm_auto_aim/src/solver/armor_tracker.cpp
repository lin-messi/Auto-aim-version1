#include "rm_auto_aim/solver/armor_tracker.hpp"

#include <cmath>
#include <limits>

namespace rm_auto_aim {

ArmorTracker::ArmorTracker() {
    ekf_ = std::make_unique<ExtendedKalmanFilter>(10, 4);
    ekf_->setFunctions(
        [](const Eigen::VectorXd& x, double dt) { return predictFunc(x, dt); },
        [](const Eigen::VectorXd& x) { return measureFunc(x); });
}

void ArmorTracker::setParams(
    double max_match_distance, double max_match_yaw_diff,
    int tracking_thres, double lost_time_thres)
{
    max_match_distance_ = max_match_distance;
    max_match_yaw_diff_ = max_match_yaw_diff;
    tracking_thres_ = tracking_thres;
    lost_time_thres_ = lost_time_thres;
}

void ArmorTracker::setEKFParams(
    double sigma2_q_x, double sigma2_q_y, double sigma2_q_z,
    double sigma2_q_yaw, double sigma2_q_r,
    double r_x, double r_y, double r_z, double r_yaw)
{
    sigma2_q_x_ = sigma2_q_x;
    sigma2_q_y_ = sigma2_q_y;
    sigma2_q_z_ = sigma2_q_z;
    sigma2_q_yaw_ = sigma2_q_yaw;
    sigma2_q_r_ = sigma2_q_r;
    r_x_ = r_x;
    r_y_ = r_y;
    r_z_ = r_z;
    r_yaw_ = r_yaw;

    // 设置EKF噪声矩阵
    Eigen::MatrixXd Q = Eigen::MatrixXd::Identity(10, 10);
    Q(0, 0) = sigma2_q_x_;  Q(1, 1) = sigma2_q_x_;   // xc, v_xc
    Q(2, 2) = sigma2_q_y_;  Q(3, 3) = sigma2_q_y_;   // yc, v_yc
    Q(4, 4) = sigma2_q_z_;  Q(5, 5) = sigma2_q_z_;   // zc, v_zc
    Q(6, 6) = sigma2_q_yaw_; Q(7, 7) = sigma2_q_yaw_; // yaw, v_yaw
    Q(8, 8) = sigma2_q_r_;                             // r
    Q(9, 9) = sigma2_q_z_;                             // d_zc

    Eigen::MatrixXd R = Eigen::MatrixXd::Identity(4, 4);
    R(0, 0) = r_x_;
    R(1, 1) = r_y_;
    R(2, 2) = r_z_;
    R(3, 3) = r_yaw_;

    ekf_->setNoiseMatrices(Q, R);
}

void ArmorTracker::update(const rm_interfaces::msg::Armors& armors, double dt) {
    // EKF预测步
    if (state_ == TrackerState::TRACKING || state_ == TrackerState::TEMP_LOST) {
        ekf_->predict(dt);
    }

    // 当前帧检测到的装甲板数量
    bool detected = !armors.armors.empty();

    switch (state_) {
        case TrackerState::LOST: {
            if (detected) {
                // 找到最近的装甲板初始化
                initEKF(armors.armors[0]);
                tracked_id_ = armors.armors[0].number;
                state_ = TrackerState::DETECTING;
                detect_count_ = 1;
            }
            break;
        }

        case TrackerState::DETECTING: {
            if (detected) {
                int match_idx = matchArmor(armors);
                if (match_idx >= 0) {
                    // 匹配成功，更新EKF
                    const auto& armor = armors.armors[match_idx];
                    Eigen::Vector4d z;
                    z << armor.pose.position.x,
                         armor.pose.position.y,
                         armor.pose.position.z,
                         2.0 * std::atan2(armor.pose.orientation.z,
                                          armor.pose.orientation.w);
                    ekf_->update(z);

                    detect_count_++;
                    if (detect_count_ >= tracking_thres_) {
                        state_ = TrackerState::TRACKING;
                    }
                } else {
                    // 未匹配，重新初始化
                    initEKF(armors.armors[0]);
                    tracked_id_ = armors.armors[0].number;
                    detect_count_ = 1;
                }
            } else {
                state_ = TrackerState::LOST;
                detect_count_ = 0;
            }
            break;
        }

        case TrackerState::TRACKING: {
            if (detected) {
                int match_idx = matchArmor(armors);
                if (match_idx >= 0) {
                    const auto& armor = armors.armors[match_idx];
                    Eigen::Vector4d z;
                    z << armor.pose.position.x,
                         armor.pose.position.y,
                         armor.pose.position.z,
                         2.0 * std::atan2(armor.pose.orientation.z,
                                          armor.pose.orientation.w);
                    ekf_->update(z);
                    lost_count_ = 0;
                    lost_time_ = 0;
                } else {
                    // 检测到装甲板但未匹配，进入临时丢失
                    lost_count_++;
                    lost_time_ += dt;
                    if (lost_time_ > lost_time_thres_) {
                        state_ = TrackerState::LOST;
                    } else {
                        state_ = TrackerState::TEMP_LOST;
                    }
                }
            } else {
                lost_count_++;
                lost_time_ += dt;
                if (lost_time_ > lost_time_thres_) {
                    state_ = TrackerState::LOST;
                } else {
                    state_ = TrackerState::TEMP_LOST;
                }
            }
            break;
        }

        case TrackerState::TEMP_LOST: {
            if (detected) {
                int match_idx = matchArmor(armors);
                if (match_idx >= 0) {
                    const auto& armor = armors.armors[match_idx];
                    Eigen::Vector4d z;
                    z << armor.pose.position.x,
                         armor.pose.position.y,
                         armor.pose.position.z,
                         2.0 * std::atan2(armor.pose.orientation.z,
                                          armor.pose.orientation.w);
                    ekf_->update(z);
                    state_ = TrackerState::TRACKING;
                    lost_count_ = 0;
                    lost_time_ = 0;
                }
            } else {
                lost_time_ += dt;
                if (lost_time_ > lost_time_thres_) {
                    state_ = TrackerState::LOST;
                }
            }
            break;
        }
    }

    // 根据tracked_id确定目标装甲板数量
    if (tracked_id_ == "1" || tracked_id_ == "hero") {
        target_armors_num_ = 2;  // 英雄双甲
    } else if (tracked_id_ == "sentry" || tracked_id_ == "outpost") {
        target_armors_num_ = 3;  // 哨兵/前哨站三甲
    } else {
        target_armors_num_ = 4;  // 步兵四甲
    }
}

void ArmorTracker::initEKF(const rm_interfaces::msg::Armor& armor) {
    double x = armor.pose.position.x;
    double y = armor.pose.position.y;
    double z = armor.pose.position.z;
    double yaw = 2.0 * std::atan2(armor.pose.orientation.z, armor.pose.orientation.w);

    // 初始状态: 位置=装甲板位置, 速度=0, r=0.2m(初始估计)
    Eigen::VectorXd x0 = Eigen::VectorXd::Zero(10);
    x0(0) = x;     // xc
    x0(1) = 0;     // v_xc
    x0(2) = y;     // yc
    x0(3) = 0;     // v_yc
    x0(4) = z;     // zc
    x0(5) = 0;     // v_zc
    x0(6) = yaw;   // yaw
    x0(7) = 0;     // v_yaw
    x0(8) = 0.2;   // r (初始旋转半径估计)
    x0(9) = 0;     // d_zc

    ekf_->init(x0);
}

int ArmorTracker::matchArmor(const rm_interfaces::msg::Armors& armors) {
    // 用预测位置与检测结果做关联
    auto state = ekf_->state();
    Eigen::Vector4d predicted_z = measureFunc(state);

    double min_dist = std::numeric_limits<double>::max();
    int best_idx = -1;

    for (size_t i = 0; i < armors.armors.size(); i++) {
        const auto& armor = armors.armors[i];

        // 位置距离
        double dx = armor.pose.position.x - predicted_z(0);
        double dy = armor.pose.position.y - predicted_z(1);
        double dz = armor.pose.position.z - predicted_z(2);
        double dist = std::sqrt(dx * dx + dy * dy + dz * dz);

        // yaw差异
        double det_yaw = 2.0 * std::atan2(armor.pose.orientation.z,
                                            armor.pose.orientation.w);
        double yaw_diff = std::abs(det_yaw - predicted_z(3));
        while (yaw_diff > M_PI) yaw_diff = std::abs(yaw_diff - 2 * M_PI);

        if (dist < max_match_distance_ && yaw_diff < max_match_yaw_diff_) {
            if (dist < min_dist) {
                min_dist = dist;
                best_idx = static_cast<int>(i);
            }
        }
    }

    return best_idx;
}

Eigen::VectorXd ArmorTracker::predictFunc(const Eigen::VectorXd& x, double dt) {
    // 匀速+匀角速度运动模型
    Eigen::VectorXd x1 = x;
    x1(0) += x(1) * dt;   // xc += v_xc * dt
    x1(2) += x(3) * dt;   // yc += v_yc * dt
    x1(4) += x(5) * dt;   // zc += v_zc * dt
    x1(6) += x(7) * dt;   // yaw += v_yaw * dt
    return x1;
}

Eigen::Vector4d ArmorTracker::measureFunc(const Eigen::VectorXd& x) {
    // 从旋转中心反推装甲板位置
    Eigen::Vector4d z;
    z(0) = x(0) - std::cos(x(6)) * x(8);  // x_armor = xc - r*cos(yaw)
    z(1) = x(2) - std::sin(x(6)) * x(8);  // y_armor = yc - r*sin(yaw)
    z(2) = x(4) + x(9);                   // z_armor = zc + d_zc
    z(3) = x(6);                           // yaw
    return z;
}

}  // namespace rm_auto_aim
