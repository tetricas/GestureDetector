#include "opencv_processor.h"

#include <cmath>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <iostream>
#include <string>

#include <QtDebug>

#include <opencv2/core/utility.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

#include "gesture_manager.h"

using namespace cv;

COpenCVProcessor::COpenCVProcessor(QObject *parent) :
    QObject(parent),
    m_roiXBegin(0.5),
    m_roiYEnd(0.8),
    m_backgroundCaptured(false),
    m_backgroundSubtractor(createBackgroundSubtractorMOG2(0,16, false)),
    m_isWorking(true),
    m_isCanCatchGesture(false)
{
    namedWindow("frame",1);
    CGestureManager::Instance().initGestures();
    m_gestureTimer = new QTimer(this);
    connect(m_gestureTimer, &QTimer::timeout, this, &COpenCVProcessor::catchNewGesture);
}

void COpenCVProcessor::run()
{
    VideoCapture camera(0);
    if(!camera.isOpened())  // check if we succeeded
        return;

    m_gestureTimer->start(3000);
    while(m_isWorking)
    {
        //Capture frame from camera
        Mat frame;
        camera >> frame;
        if(frame.data == nullptr)
            return;

        Mat frameFiltered;

        bilateralFilter(frame, frameFiltered, 5, 50, 100);
        //Operations on the frame
        flip(frameFiltered, frame, 1);
        Point start(m_roiXBegin * frame.size().width, 0),
                end(frame.size().width - 1, m_roiYEnd*frame.size().height);
        rectangle(frame, start, end, {255,0,0});
        Mat foregroundFrame, contourFrame;
        if(m_backgroundCaptured)
        {
            removeBackground(frame, foregroundFrame);
            Mat frameTresholded;
            thresholdHand(foregroundFrame, frameTresholded);
            contourFrame = frameTresholded.clone();
            Mat hierarchy, handContour;
            std::vector<Mat> contours;
            findContours(contourFrame, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
            size_t frameArea = contourFrame.size().width * contourFrame.size().height;
            if(findHandContour(contours, handContour, frameArea))
            {
                std::vector<Point> handConvexHull;
                convexHull(handContour, handConvexHull);
                Point handCenter;
                int handRadius;
                if(markHandCenter(frame, handContour, handCenter, handRadius))
                {
                    std::vector<Point> fingers;
                    markFingers(frame, handConvexHull, fingers, handCenter, handRadius);
                    polylines(frame, handConvexHull, false, {255,255,255},2);

                    static std::string gesture;
                    if(m_isCanCatchGesture)
                    {
                        gesture = "Gesture: " + CGestureManager::Instance().findGesture(fingers, handCenter);
                        m_isCanCatchGesture = false;
                    }
                    putText(frame, gesture, Point(0.6*frame.size().width,0.88*frame.size().height),
                            FONT_HERSHEY_DUPLEX, 1, {0,255,255});
                }
            }
        }

        imshow("frame", frame);
        waitKey(10);
    }
    emit finished();
}

void COpenCVProcessor::captureBackground()
{
    m_backgroundCaptured = true;
}

void COpenCVProcessor::stop()
{
    m_isWorking = false;
}

void COpenCVProcessor::catchNewGesture()
{
    m_isCanCatchGesture = true;
}

void COpenCVProcessor::thresholdHand(const cv::Mat& frameIn, Mat& frameOut)
{
    Mat frame, hsv, backProjection, handHist;
    medianBlur(frameIn, frame, 3);
    cvtColor(frameIn, hsv, COLOR_BGR2HSV);
    int roiX = m_roiXBegin * hsv.size().width;
    int roiY = m_roiYEnd * hsv.size().height;
    Mat ROI = frameIn(Rect(roiX, 0, roiX, roiY));

    for(int i(0); i < hsv.size().height; ++i)
        for(int j(0); j < hsv.size().width; ++j)
            if(!(i < roiY && j > roiX))
                hsv.row(i).col(j) = 0;

    const int channels[] = {0,1};
    int histSize[] = {180, 256};
    float hranges[] = { 0, 180 };
    float sranges[] = { 0, 256 };
    const float* ranges[] = {hranges,sranges};

    calcHist(&ROI, 1, channels, Mat(), handHist, 2, histSize, ranges);
    normalize(handHist,handHist, 0, 255, NORM_MINMAX);
    calcBackProject(&hsv, 1, channels, handHist, backProjection, ranges);

    int threshLower(150);
    threshold(backProjection, frameOut, threshLower, 255, 0);
}

bool COpenCVProcessor::findHandContour(const std::vector<Mat>& contours, Mat &handContour, size_t frameSize)
{
    double max_area = 0;
    int largestContour=-1;
    for( size_t i(0); i < contours.size(); ++i)
    {
        Mat cont = contours[i];
        double area = contourArea(cont);
        if(area > max_area && area < frameSize * 2 / 3)
        {
            max_area = area;
            largestContour = i;
        }
    }
    if(largestContour == -1)
        return false;
    else
    {
        handContour = contours[largestContour];
        return true;
    }
}

bool COpenCVProcessor::markHandCenter(cv::Mat& frame, const cv::Mat& contour, cv::Point& center, int &maxDist)
{
    maxDist = 0;
    center = Point(0,0);
    Rect rect = boundingRect(contour);
    int maxHeight = rect.y + 0.8 * rect.height;
    int minHeight = rect.y + 0.3 * rect.height;
    int maxWidth = rect.x+  0.6 * rect.width;
    int minWidth = rect.x + 0.3 * rect.width;

    //around 0.3 to 0.8 region of height (Faster calculation with ok results)
    for( int indY(minHeight); indY < maxHeight; ++indY)
    {
        //around 0.3 to 0.6 region of width (Faster calculation with ok results)
        for( int indX(minWidth); indX < maxWidth; ++indX)
        {
                int dist = pointPolygonTest(contour,Point(indX,indY),true);
                if(dist > maxDist)
                {
                    maxDist = dist;
                    center = Point(indX, indY);
                }
        }
    }
    double radius_thresh = 0.04;
    if(maxDist > radius_thresh * frame.size().width)
    {
        circle(frame, center, maxDist, {255,0,0},2);
        return true;
    }
    else
        return false;
}

void COpenCVProcessor::markFingers(cv::Mat& frame, const std::vector<cv::Point>& hull, std::vector<cv::Point>& fingers,
                                   const cv::Point& handCenter, int radius)
{
    int fingerRadius = radius / 5;

    size_t size = hull.size();
    Point basementPoint(0, 0);
    int maxHeight = 0;
    for( size_t i(0); i < size - 1; ++i)
    {
        double dist = sqrt(pow(hull[size - i].x - hull[size - i - 1].x, 2) +
                           pow(hull[size - i].y - hull[size - i - 1].y, 2));
        if (dist > fingerRadius * 1.5)
        {
            fingers.push_back(Point(hull[size - i].x, hull[size - i].y));
            if(hull[size - i].y == hull[size - i - 1].y && maxHeight < hull[size - i].y)
            {
                int middleX = hull[size - i].x > hull[size - i - 1].x ? hull[size - i - 1].x : hull[size - i].x;
                middleX -= (hull[size - i].x - hull[size - i - 1].x) / 2;
                basementPoint = Point(middleX, hull[size - i].y);
                maxHeight = hull[size - i].y;
            }
        }
    }

    double fingerThreshUpBound(3.8);
    size_t i = 0;
    while(i<fingers.size())
    {
        double dist = sqrt( pow(fingers[i].x- handCenter.x, 2) + pow(fingers[i].y - handCenter.y, 2));

        if( dist < radius * 1.5 ||
                dist > fingerThreshUpBound * radius ||
                fingers[i].y > handCenter.y + radius )
            fingers.erase( fingers.begin() + i);
        else
            ++i;
    }

    fingers.push_back( basementPoint);
    for(size_t k(0); k < fingers.size(); ++k)
    {
        Scalar color = {255, 0, 0};
        if(k == 0)
            color= {0, 0, 255};
        else if(k == fingers.size() - 1)
            color= {0, 255, 0};
        circle(frame, fingers[k], fingerRadius, color, 2);
        line(frame, fingers[k], handCenter, {0,255,255}, 2);
        putText(frame, std::to_string(k), fingers[k], FONT_HERSHEY_DUPLEX, 1, {0,255,255});
    }
}

void COpenCVProcessor::removeBackground(const Mat &frame, Mat &foreground)
{
    Mat fg_mask, eroded;
    m_backgroundSubtractor->apply(frame, fg_mask, 0);
    int erosionSize = 5;
    Mat element;
    element = getStructuringElement( MORPH_ELLIPSE,
                                     Size( 2 * erosionSize + 1, 2 * erosionSize + 1 ),
                                     Point( erosionSize, erosionSize ) );

    erode(fg_mask, eroded, element);
    bitwise_and(frame,frame, foreground, eroded);
}
