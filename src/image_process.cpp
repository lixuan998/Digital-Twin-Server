#include "image_process.h"

//public functions
Image_Process::Image_Process()
{
    thresh = 100;
}

Image_Process::Image_Process(QVector<float> calib_params, int thresh)
{
    this->calib_params = calib_params;
    this->thresh = thresh;
}

void Image_Process::processImage(cv::Mat &input, bool display_origin)
{   
    type_and_angles.clear();
    catch_points.clear();
    color_mat = input;
    colorToBinary();
    cv::findContours(binary_mat, contours_in_vector, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
    convertContourFromVector();
    contourFilterate();
    contourSort();
    if(!display_origin) cv::cvtColor(binary_mat, color_mat, cv::COLOR_GRAY2BGR);
    
    drawResult(color_mat);
}

QVector< std::pair<QString, int> > Image_Process::getDetections()
{
    return type_and_angles;
}

QVector<cv::Point> Image_Process::getCatchPoints()
{
    return catch_points;
}

int Image_Process::getContoursCount()
{
	return contours_in_class_after_filterate.size();
}
//private functions
void Image_Process::colorToBinary()
{
    switch (color_mat.channels())
    {
        case 1 :
        {
            grey_mat = color_mat;
            break;
        }
        case 3 :
        {
            cv::cvtColor(color_mat, grey_mat, cv::COLOR_BGR2GRAY);
            break;
        }
        case 4 :
        {
            cv::cvtColor(color_mat, grey_mat, cv::COLOR_BGRA2GRAY);
            break;
        }
        default: qDebug() << "这个图片的通道数闻所未闻 debug from Image_Process.cpp line 34";
    }
    cv::medianBlur(grey_mat, grey_mat, 3);
    cv::threshold(grey_mat, binary_mat, thresh, 255, cv::THRESH_BINARY);
}

void Image_Process::convertContourFromVector()
{
    contours_in_class_before_filterate.clear();
    for(size_t i = 0; i < contours_in_vector.size(); ++ i)
    {
        contours_in_class_before_filterate.push_back(Contour(contours_in_vector[i]));
    }
}

void Image_Process::contourFilterate()
{
    contours_in_class_after_filterate.clear();
    contours_in_vector.clear();
    for(size_t i = 0; i < contours_in_class_before_filterate.size(); ++ i)
    {
        if(Filter(contours_in_class_before_filterate[i]))
        {
            contours_in_class_after_filterate.push_back(contours_in_class_before_filterate[i]);
            contours_in_vector.push_back(contours_in_class_before_filterate[i].getContour());
        }
    }
    contours_in_class_before_filterate.clear();
    //qDebug() << "contours_in_class_after_filterate : " << contours_in_class_after_filterate.size();
}

bool Image_Process::Filter(Contour single_contour)
{
    cv::RotatedRect tmp_rotated_rect = single_contour.getRotatedRect();
    int width = std::min(tmp_rotated_rect.size.height, tmp_rotated_rect.size.width);
    int height = std::max(tmp_rotated_rect.size.height, tmp_rotated_rect.size.width);

    double alpha = (double) width / (double) height;
	//qDebug() << "alpha: " << alpha;
    if(alpha > 0.4 && alpha < 0.7)
    {
        //qDebug() << "tmp_rotated_rect.size.area() " << tmp_rotated_rect.size.area();
        if(tmp_rotated_rect.size.area() > 30000 && tmp_rotated_rect.size.area() <= 500000) return true;
        else return false;
    }
    else return false;
}

void Image_Process::contourSort()
{
    std::sort(contours_in_class_after_filterate.begin(), contours_in_class_after_filterate.end(), Contour::cmpY);
    if(contours_in_class_after_filterate.size() != 9) return;
    for(size_t i = 0; i < contours_in_class_after_filterate.size(); i += 3)
    {
        std::sort(contours_in_class_after_filterate.begin() + i, contours_in_class_after_filterate.begin() + i + 3, Contour::cmpX);
    }
}

void Image_Process::drawResult(cv::Mat draw_mat)
{
    //cv::drawContours(draw_mat, contours_in_vector, -1, cv::Scalar(0, 0, 255), 5);
    drawRect(draw_mat);
}

void Image_Process::drawRect(cv::Mat draw_mat)
{    
    for(size_t i = 0; i < contours_in_class_after_filterate.size(); ++ i)
    {
        cv::Point2f points[4];
        cv::Point center_point = contours_in_class_after_filterate[i].getCenterPoint();
        cv::RotatedRect rrect = contours_in_class_after_filterate[i].getRotatedRect();
        
        rrect.points(points);

        std::pair<QString, int> type_and_angle = detectImageType(rrect);
		
        if(type_and_angle.first != nullptr)
        {
            cv::Point catch_point = transformCoordinate(contours_in_class_after_filterate[i].getCenterPoint());
            catch_points.push_back(catch_point);
            cv::circle(draw_mat, center_point, 5, cv::Scalar(0, 0, 255), 16);
			
			for (size_t j = 0; j < 4; ++j)
			{
				cv::line(draw_mat, points[j], points[(j + 1) % 4], cv::Scalar(255, 0, 0), 16, cv::FILLED);
			}

            QString display_text;
            display_text = type_and_angle.first + " " + QString::number(type_and_angle.second) + "'";
            cv::putText(draw_mat, display_text.toStdString(), contours_in_class_after_filterate[i].getCenterPoint(), cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 255, 0), 4);
        } 
    }
}

std::pair<QString, int> Image_Process::detectImageType(cv::RotatedRect rrect)
{
    std::pair<QString, int> type_and_angle = std::pair<QString, int>{nullptr, -1};
    cv::Point2f points[4];
    rrect.points(points);

    cv::Point2f roi_left_point;
    roi_left_point.x = std::min(std::min(points[0].x, points[1].x), std::min(points[2].x, points[3].x));
    roi_left_point.y = std::min(std::min(points[0].y, points[1].y), std::min(points[2].y, points[3].y));
    roi_left_point.x = std::max(float(0), roi_left_point.x);
    roi_left_point.y = std::max(float(0), roi_left_point.y);
    int roi_width = std::max(std::max(points[0].x, points[1].x), std::max(points[2].x, points[3].x)) - roi_left_point.x;
    int roi_height = std::max(std::max(points[0].y, points[1].y), std::max(points[2].y, points[3].y)) - roi_left_point.y;
    if(roi_left_point.x + roi_width <= binary_mat.cols && roi_left_point.y + roi_height <= binary_mat.rows)
    {
        cv::Mat tmp = binary_mat(cv::Rect(roi_left_point.x, roi_left_point.y, roi_width, roi_height));
		cv::waitKey(1);

        int angle = (int) rrect.angle;
        
        type_and_angle.first = detector.matchDetect(angle, tmp);
        angle = (360 - (angle + 270) % 360);
        if(angle == 360) angle = 0;
        type_and_angle.second = angle;
    }
    if(type_and_angle.first != nullptr) type_and_angles.push_back(type_and_angle);
    return type_and_angle;
}

cv::Point Image_Process::transformCoordinate(cv::Point cam_point)
{
    cv::Point catch_point{-1, -1};
    if(calib_params.size() != 6) return catch_point;
    catch_point.x = cam_point.x * calib_params[0] + cam_point.y * calib_params[1] + calib_params[2];
    catch_point.y = cam_point.x * calib_params[3] + cam_point.y * calib_params[4] + calib_params[5];
    return catch_point;
}