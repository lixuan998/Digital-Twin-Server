#pragma once

#include <QWidget>
#include <vector>
#include <QThread>
#include <QDebug>
#include <QMap>
#include <iostream>

#include "opencv_headers.h"
#include "detector.h"
#include "contour.h"


class Image_Process
{
public:
    Image_Process();
    Image_Process(QVector<float> calib_params, int thresh = 100);
    void processImage(cv::Mat &input, bool display_origin = true);
    QVector< std::pair<QString, int> > getDetections();
    QVector<cv::Point> getCatchPoints();

	int getContoursCount();

private:
    void colorToBinary();

    void convertContourFromVector();

    bool Filter(Contour single_contour);

    void contourFilterate();

    void contourSort();

    void drawResult(cv::Mat draw_mat);

    void drawRect(cv::Mat draw_mat);

    std::pair<QString, int> detectImageType(cv::RotatedRect rrect);
    
    cv::Point transformCoordinate(cv::Point cam_point);
private:
    cv::Mat color_mat, binary_mat, grey_mat;

    //contours中的每一个元素存储了图像中的轮廓
    std::vector< std::vector<cv::Point> > contours_in_vector;

    std::vector<Contour> contours_in_class_before_filterate, contours_in_class_after_filterate;
    
    double thresh;

    Detector detector;

    QVector< std::pair<QString, int> > type_and_angles;

    QVector<cv::Point> catch_points;

    QVector<float> calib_params;
    
};

