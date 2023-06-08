#pragma once
#include <QString>
#include <QDebug>
#include "opencv_headers.h"

#define TEMPLATES_PATH "./templates/"
#define TEMPLATE_SUFFIX ".png"

class Detector 
{
    public:     //public functions
    
        Detector();
        ~Detector();

        QString matchDetect(QString assumption, int &angle, cv::Mat src = cv::Mat());
        QString matchDetect(int &angle, cv::Mat src = cv::Mat());
    private:    //private functions

    public:     //public members

    private:    //private members
};