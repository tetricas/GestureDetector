#pragma once

#include <QObject>
#include <QTimer>

#include <opencv2/core/core.hpp>
#include <opencv2/video/background_segm.hpp>

class COpenCVProcessor : public QObject
{
    Q_OBJECT
public:
    explicit COpenCVProcessor(QObject *parent = nullptr);

signals:
    void finished();

public slots:

    void run();
    void captureBackground();

    void stop();
    void catchNewGesture();

private:
    void thresholdHand(const cv::Mat& frameIn, cv::Mat& frameOut);
    bool findHandContour(const std::vector<cv::Mat> &contours, cv::Mat& handContour, size_t frameSize);
    bool markHandCenter(cv::Mat& frame, const cv::Mat& contour, cv::Point& center, int &maxDist);
    void markFingers(cv::Mat& frame, const std::vector<cv::Point>& hull, std::vector<cv::Point>& fingers,
                     const cv::Point& handCenter, int radius);

    void removeBackground(const cv::Mat& frame, cv::Mat& foreground);

    double m_roiXBegin;
    double m_roiYEnd;
    bool m_backgroundCaptured;

    cv::Ptr<cv::BackgroundSubtractor> m_backgroundSubtractor;

    bool m_isWorking;
    QTimer* m_gestureTimer;
    bool m_isCanCatchGesture;
};
