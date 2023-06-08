#include "detector.h"

Detector::Detector(){}

Detector::~Detector(){}

QString Detector::matchDetect(QString assumption, int &angle, cv::Mat src)
{
    //qDebug() << "Detector::matchDetect " << src.cols << " " <<src.rows;
    if(src.empty()) return nullptr;
    for(int i = 0; i < 4; ++ i)
    {
        int assumpt_angle = i * 90 + angle;
        if(assumpt_angle == 360) assumpt_angle = 0;
        QString template_path = TEMPLATES_PATH + assumption + QString::number(assumpt_angle) + TEMPLATE_SUFFIX;
        //qDebug() << "template_path:" << template_path;
        cv::Mat template_mat = cv::imread(template_path.toStdString(), CV_8UC1);
        cv::resize(template_mat, template_mat, src.size());
        //cv::imshow("t" + std::to_string(i), template_mat);
        cv::Mat result;

        cv::matchTemplate(src, template_mat, result, cv::TM_CCOEFF_NORMED);

        double min_val, max_val;
        cv::Point min_loc, max_loc;
        cv::minMaxLoc(result, &min_val, &max_val, &min_loc, &max_loc);

        
		if (assumption == "A")
		{
			qDebug() << "A max: " << max_val;
			if (max_val >= 0.5)
			{
				angle = assumpt_angle;
				return assumption;
			}
		}
		else
		{
			qDebug() << "else max: " << max_val;
			if(max_val >= 0.8)
			{
				angle = assumpt_angle;
				return assumption;
			}
		}
       
    }
    
    return nullptr;

    //cv::imshow("template_mat", template_mat);
    
}

QString Detector::matchDetect(int &angle, cv::Mat src)
{
    if(src.empty()) return nullptr;

    QStringList matches{"A", "B", "C", "D"};
    for(auto m : matches)
    {
        QString ret = matchDetect(m, angle, src);
        if(ret != nullptr) return ret;
    }
    return nullptr;
}
