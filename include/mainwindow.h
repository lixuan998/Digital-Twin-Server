#ifndef MainWindow_H
#define MainWindow_H

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include <QMainWindow>
#include <QVector>
#include <QFileDialog>
#include <QTableView>
#include <QFile>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QRadioButton>
#include <QThread>
#include <QCloseEvent>
#include <QTimer>
#include <QThread>

#include "MvCamera.h"
#include "opencv_headers.h"
#include "image_process.h"
#include "tcp_messager.h"

#include <QDebug>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    CMvCamera*  m_pcMyCamera;
    MV_CC_DEVICE_INFO_LIST  m_stDevList;
    bool                    m_bGrabbing;   // 是否开始抓图
    void *m_hWnd;
    void *handle;

	float fmax, fmin;
	float emax, emin;
	float gmax, gmin;

    QVector<float> calib_params;
    int thresh;
    Image_Process image_processer;
    QStandardItemModel model;
    bool display_origin;
    Tcp_Messager *tcp_messager = nullptr;
	QThread tcp_thread;
    QVector< std::pair<QString, int> > detections;
    QVector<cv::Point> catch_points;

    QTimer timer;

private:

    void initTable();
    void connectMenuSignals();
	void connectSpinboxSignals();
    void connectRadioButtonSignals();
    void connectTcpMessagerSignals();

    void ShowErrorMsg(QString csMessage, int nErrorNum);
    void static __stdcall ImageCallBack(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser);
	void ImageCallBackInner(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo);

    void showDetections();

    void closeEvent(QCloseEvent *event);
private slots:

    void choseFile();

    void on_send_stop_pushButton_clicked();

    void on_send_msg_pushButton_clicked();

    void on_listen_pushButton_clicked();
    
    void on_search_button_clicked();

    void on_open_device_button_clicked();

    void on_close_device_button_clicked();

    void on_start_button_clicked();

    void on_stop_button_clicked();

    void on_getPara_button_clicked();

    void on_setPara_button_clicked();

	void setPara(double value);

	signals:
		void sendMsg(QString);
		void startListening(QHostAddress, int);
		void stopListening();
};

#endif // MainWindow_H
