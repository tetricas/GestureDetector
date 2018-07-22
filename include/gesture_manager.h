#pragma once

#include <string>
#include <vector>

#include <opencv2/core/core.hpp>

#include "rapidxml.hpp"

class CGestureManager
{
public:
    static CGestureManager& Instance();

    static void initGestures();
    static std::string findGesture(const std::vector<cv::Point>& points, const cv::Point& center);

    CGestureManager(CGestureManager const&) = delete;
    CGestureManager& operator= (CGestureManager const&) = delete;

private:
    CGestureManager();
    ~CGestureManager();

    static void findLineCoefficients(const cv::Point& point1, const cv::Point& point2, int& A, int& B);
    static double findFingerAngle(const cv::Point& finger, const cv::Point& center);

    std::string m_content;

};
