#include "gesture_manager.h"

#include <fstream>
#include <sstream>
#include <cmath>

#define PI 3.14159265

#include <QCoreApplication>
#include <QFile>
#include <QtDebug>

using namespace rapidxml;

CGestureManager& CGestureManager::Instance()
{
    static CGestureManager instance;
    return instance;
}

void CGestureManager::initGestures()
{
    QString path = QCoreApplication::applicationDirPath();
    path += "/gestures.xml";
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << file.errorString() << "File: " << path;
        return;
    }

    QString content = file.readAll();
    Instance().m_content = content.toStdString();
}

std::string CGestureManager::findGesture(const std::vector<cv::Point> &points, const cv::Point &center)
{
    std::vector<double> angles;
    for (const auto& point: points)
        angles.push_back(findFingerAngle(point, center));

    xml_document<> gesturesXml;
    std::string content = Instance().m_content;
    gesturesXml.parse<0>(&content[0]);
    xml_node<>* gesturesRoot =  gesturesXml.first_node();
    if(!gesturesRoot)
    {
        qDebug() << "Error with parsing xml";
        return "";
    }

    xml_node<>* node = gesturesRoot->first_node();
    std::string gesture = node->name();
    bool isGesture = true;
    for (xml_node<>* child = node->first_node(); child; child = child->next_sibling())
    {
        std::vector<int> bounds;
        std::istringstream stream(child->value());
        std::string part;
        while (getline(stream, part, ';'))
            bounds.push_back(std::stoi(part));

        size_t pos = 5;
        std::string name = child->name();
        if(name.compare("base"))
            pos = std::stoi(child->first_attribute()->value()) - 1;

        if(angles.size() > pos)
            if(angles[pos] < bounds[0] || angles[pos] > bounds[1])
                isGesture = false;
    }
    if(!isGesture)
        return "";

    return gesture;
}

CGestureManager::CGestureManager()
{}

CGestureManager::~CGestureManager()
{}

void CGestureManager::findLineCoefficients(const cv::Point &point1, const cv::Point &point2, int &A, int &B)
{
    A = point1.y - point2.y;
    B = point2.x - point1.x;
}

double CGestureManager::findFingerAngle(const cv::Point &finger, const cv::Point &center)
{
    int fingerA, fingerB;
    findLineCoefficients(finger, center, fingerA, fingerB);

    cv::Point centerTop(center);
    centerTop.y = 0;
    int centerA, centerB;
    findLineCoefficients(centerTop, center, centerA, centerB);

    return acos((fingerA * centerA + fingerB * centerB)/
                (sqrt(fingerA*fingerA + fingerB*fingerB)*sqrt(centerA*centerA + centerB*centerB))) * 180.0 / PI;
}
