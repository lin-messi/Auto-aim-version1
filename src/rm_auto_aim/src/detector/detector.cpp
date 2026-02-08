#include "rm_auto_aim/detector/detector.hpp"

#include <algorithm>
#include <cmath>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

namespace rm_auto_aim {

ArmorDetector::ArmorDetector(const DetectorParams& params) : params_(params) {}

std::vector<Armor> ArmorDetector::detect(const cv::Mat& input, Color detect_color) {
    // 1. 预处理生成二值图
    preprocess(input);

    if (params_.debug) {
        debug_img_ = input.clone();
    }

    // 2. 灯条检测
    auto lights = detectLights(input, detect_color);

    // 3. 按x坐标排序
    std::sort(lights.begin(), lights.end(),
              [](const Light& a, const Light& b) { return a.center.x < b.center.x; });

    // 4. 灯条配对生成装甲板
    auto armors = matchArmors(lights);

    if (params_.debug) {
        // 绘制灯条
        for (const auto& light : lights) {
            cv::Point2f pts[4];
            light.points(pts);
            for (int i = 0; i < 4; i++) {
                cv::line(debug_img_, pts[i], pts[(i + 1) % 4],
                         light.color == Color::RED ? cv::Scalar(0, 0, 255)
                                                   : cv::Scalar(255, 0, 0),
                         2);
            }
        }
        // 绘制装甲板
        for (const auto& armor : armors) {
            auto corners = armor.corners();
            for (size_t i = 0; i < corners.size(); i++) {
                cv::line(debug_img_, corners[i], corners[(i + 1) % corners.size()],
                         cv::Scalar(0, 255, 0), 2);
            }
            cv::putText(debug_img_, armor.number, armor.center(),
                        cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 255), 2);
        }
    }

    return armors;
}

void ArmorDetector::preprocess(const cv::Mat& input) {
    cv::Mat gray;
    cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, binary_, params_.binary_threshold, 255, cv::THRESH_BINARY);
}

std::vector<Light> ArmorDetector::detectLights(const cv::Mat& input, Color detect_color) {
    std::vector<Light> lights;

    // 查找轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary_, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (const auto& contour : contours) {
        // 轮廓点数过少则跳过
        if (contour.size() < 5) continue;

        // 拟合旋转矩形
        auto r_rect = cv::minAreaRect(contour);
        Light light(r_rect);

        // 几何约束检查
        if (!isValidLight(light)) continue;

        // 颜色分类
        light.color = classifyLightColor(input, r_rect);
        if (light.color != detect_color) continue;

        lights.push_back(light);
    }

    return lights;
}

bool ArmorDetector::isValidLight(const Light& light) const {
    // 长宽比约束
    double ratio = light.length / std::max(light.width, 1.0f);
    if (ratio < params_.light_min_ratio || ratio > params_.light_max_ratio) {
        return false;
    }

    // 倾斜角度约束（灯条应该近似竖直）
    float angle = light.tilt_angle;
    // 归一化角度到 [-90, 90]
    if (angle > 90.0f) angle -= 180.0f;
    if (angle < -90.0f) angle += 180.0f;
    if (std::abs(angle) > params_.light_max_angle) {
        return false;
    }

    return true;
}

Color ArmorDetector::classifyLightColor(const cv::Mat& input, const cv::RotatedRect& rect) const {
    // 获取灯条ROI
    cv::Mat mask = cv::Mat::zeros(input.size(), CV_8UC1);
    cv::Point2f pts[4];
    rect.points(pts);
    std::vector<cv::Point> roi_pts;
    for (int i = 0; i < 4; i++) {
        roi_pts.emplace_back(static_cast<int>(pts[i].x), static_cast<int>(pts[i].y));
    }
    cv::fillConvexPoly(mask, roi_pts, cv::Scalar(255));

    // 计算红蓝通道均值
    cv::Scalar mean_val = cv::mean(input, mask);
    double b_mean = mean_val[0];
    double r_mean = mean_val[2];

    // 通道差值判定颜色
    if (r_mean - b_mean > params_.light_color_diff_thresh) {
        return Color::RED;
    } else if (b_mean - r_mean > params_.light_color_diff_thresh) {
        return Color::BLUE;
    }

    // 默认返回红色（可根据需要调整）
    return r_mean > b_mean ? Color::RED : Color::BLUE;
}

std::vector<Armor> ArmorDetector::matchArmors(const std::vector<Light>& lights) {
    std::vector<Armor> armors;

    // 遍历所有灯条对
    for (size_t i = 0; i < lights.size(); i++) {
        for (size_t j = i + 1; j < lights.size(); j++) {
            const auto& left = lights[i];
            const auto& right = lights[j];

            // 检查是否构成有效装甲板
            if (!isValidArmor(left, right)) continue;

            // 检查两灯条之间是否有其他灯条
            if (containsLight(left, right, lights)) continue;

            Armor armor;
            armor.left_light = left;
            armor.right_light = right;

            // 根据灯条间距判断装甲板大小
            double center_dist = cv::norm(left.center - right.center);
            double avg_length = (left.length + right.length) / 2.0;
            double ratio = center_dist / avg_length;

            if (ratio < params_.armor_max_small_center_distance) {
                armor.type = ArmorType::SMALL;
            } else {
                armor.type = ArmorType::LARGE;
            }

            armor.number = "unknown";  // 待分类器填充
            armors.push_back(armor);
        }
    }

    return armors;
}

bool ArmorDetector::isValidArmor(const Light& left, const Light& right) const {
    // 灯条间距/平均灯条长度的比值
    double center_dist = cv::norm(left.center - right.center);
    double avg_length = (left.length + right.length) / 2.0;
    double ratio = center_dist / avg_length;

    // 距离约束
    bool small_valid = (ratio >= params_.armor_min_small_center_distance &&
                        ratio <= params_.armor_max_small_center_distance);
    bool large_valid = (ratio >= params_.armor_min_large_center_distance &&
                        ratio <= params_.armor_max_large_center_distance);
    if (!small_valid && !large_valid) {
        return false;
    }

    // 灯条连线角度约束（装甲板应水平）
    double dx = right.center.x - left.center.x;
    double dy = right.center.y - left.center.y;
    double angle = std::abs(std::atan2(dy, dx)) * 180.0 / CV_PI;
    if (angle > params_.armor_max_angle) {
        return false;
    }

    // 灯条长度比约束（两灯条长度不应差距过大）
    double length_ratio = left.length < right.length ? left.length / right.length
                                                     : right.length / left.length;
    if (length_ratio < 0.3) {
        return false;
    }

    return true;
}

bool ArmorDetector::containsLight(
    const Light& left, const Light& right,
    const std::vector<Light>& lights) const {
    // 检查left和right之间是否包含其他灯条
    float lx = left.center.x;
    float rx = right.center.x;

    for (const auto& light : lights) {
        if (&light == &left || &light == &right) continue;
        if (light.center.x > lx && light.center.x < rx) {
            return true;
        }
    }
    return false;
}

}  // namespace rm_auto_aim
