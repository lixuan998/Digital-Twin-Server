#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	this->setWindowTitle("数字孪生客户端");
	memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
	m_pcMyCamera = NULL;
	m_bGrabbing = false;
	m_hWnd = (void*)ui->display_label->winId();
	handle = NULL;

	display_origin = true;
	tcp_messager = new Tcp_Messager();
	timer.setInterval(10);
	connect(&timer, &QTimer::timeout, this, [&] {showDetections(); });

	initTable();
	connectMenuSignals();
	connectSpinboxSignals();
	connectRadioButtonSignals();
	connectTcpMessagerSignals();
}

MainWindow::~MainWindow()
{
	if (timer.isActive()) timer.stop();
	delete ui;

	if (tcp_thread.isRunning())
	{
		emit stopListening();
		tcp_thread.quit();
		tcp_thread.wait();
		if (tcp_messager != nullptr)
		{
			delete tcp_messager;
			tcp_messager = nullptr;
		}
		
	}
}

/*Private Functions*/

void MainWindow::initTable()
{
	ui->image_result_tableview->setEditTriggers(QAbstractItemView::NoEditTriggers);
	model.setHorizontalHeaderLabels({ "字母", "旋转角度", "X轴坐标", "Y轴坐标" });
	model.setItem(0, 0, new QStandardItem());
	ui->image_result_tableview->setModel(&model);
}

void MainWindow::connectMenuSignals()
{
	connect(ui->import_calib_data, &QAction::triggered, this, &MainWindow::choseFile);
}

void MainWindow::connectSpinboxSignals()
{
	//connect(ui->exposure_spinbox, &QDoubleSpinBox::valueChanged, this, &QMainWindow::on_setPara_button_clicked);
	connect(ui->exposure_spinbox, SIGNAL(valueChanged(double)), this, SLOT(setPara(double)));
	connect(ui->framerate_spinbox, SIGNAL(valueChanged(double)), this, SLOT(setPara(double)));
	connect(ui->gain_spinbox, SIGNAL(valueChanged(double)), this, SLOT(setPara(double)));
}

void MainWindow::connectRadioButtonSignals()
{
	connect(ui->origin_rbutton, &QRadioButton::toggled, this, [&](bool checked) {
		if (checked)
		{
			on_stop_button_clicked();
			display_origin = true;
			on_start_button_clicked();
		}
		});

	connect(ui->binary_rbutton, &QRadioButton::toggled, this, [&](bool checked) {
		if (checked)
		{
			on_stop_button_clicked();
			display_origin = false;
			on_start_button_clicked();
		}
		});
}

void MainWindow::connectTcpMessagerSignals()
{
	connect(this, &MainWindow::sendMsg, tcp_messager, &Tcp_Messager::sendMessage);
	connect(this, &MainWindow::startListening, tcp_messager, &Tcp_Messager::startListening);
	connect(this, &MainWindow::stopListening, tcp_messager, &Tcp_Messager::stopListening);

	connect(tcp_messager, &Tcp_Messager::listening, this, [&] {
		ui->listen_pushButton->setText("监听中...");
		ui->listen_pushButton->setStyleSheet("QPushButton{"
			"background-color: rgb(38, 162, 105);}"
			"QPushButton:hover{"
			"background-color: rgb(46, 194, 126);}");
		ui->ip_lineEdit->setDisabled(true);
		ui->port_lineEdit->setDisabled(true);
		});

	connect(tcp_messager, &Tcp_Messager::notListening, this, [&] {
		ui->listen_pushButton->setText("监听");
		ui->listen_pushButton->setStyleSheet("QPushButton{"
			"background-color: rgb(165, 29, 45);}"
			"QPushButton:hover{"
			"background-color: rgb(224, 27, 36);}");
		ui->ip_lineEdit->setDisabled(false);
		ui->port_lineEdit->setDisabled(false);
		});

	connect(tcp_messager, &Tcp_Messager::connected, this, [&](QHostAddress address) {
		ui->client_state_label->setStyleSheet("QLabel{color: rgb(38, 162, 105);}");
		QString text = address.toString() + "已连接";
		ui->client_state_label->setText(text);
		});

	connect(tcp_messager, &Tcp_Messager::disconnected, this, [&] {
		ui->client_state_label->setStyleSheet("QLabel{color: rgb(224, 27, 36);}");
		QString text = "未连接";
		ui->client_state_label->setText(text);
		});

	connect(tcp_messager, &Tcp_Messager::sendMsg, this, [&](QString msg) {
		ui->msg_textEdit->setTextColor(QColor(255, 45, 4));
		ui->msg_textEdit->append(msg + ((msg != "Stop") ? " (位置信息)" : "(停止信号)"));
		ui->msg_textEdit->moveCursor(QTextCursor::End);
		});

	connect(tcp_messager, &Tcp_Messager::recivedMsg, this, [&](QString msg) {
		ui->msg_textEdit->setTextColor(QColor(6, 173, 88));
		ui->msg_textEdit->append(msg + " (回应信息)");
		ui->msg_textEdit->moveCursor(QTextCursor::End);
		});

	tcp_messager->moveToThread(&tcp_thread);
}

void MainWindow::ShowErrorMsg(QString csMessage, int nErrorNum)
{
	QString errorMsg = csMessage;
	if (nErrorNum != 0)
	{
		QString TempMsg;
		TempMsg.sprintf(": Error = %x: ", nErrorNum);
		errorMsg += TempMsg;
	}

	switch (nErrorNum)
	{
	case MV_E_HANDLE:           errorMsg += "Error or invalid handle ";                                         break;
	case MV_E_SUPPORT:          errorMsg += "Not supported function ";                                          break;
	case MV_E_BUFOVER:          errorMsg += "Cache is full ";                                                   break;
	case MV_E_CALLORDER:        errorMsg += "Function calling order error ";                                    break;
	case MV_E_PARAMETER:        errorMsg += "Incorrect parameter ";                                             break;
	case MV_E_RESOURCE:         errorMsg += "Applying resource failed ";                                        break;
	case MV_E_NODATA:           errorMsg += "No data ";                                                         break;
	case MV_E_PRECONDITION:     errorMsg += "Precondition error, or running environment changed ";              break;
	case MV_E_VERSION:          errorMsg += "Version mismatches ";                                              break;
	case MV_E_NOENOUGH_BUF:     errorMsg += "Insufficient memory ";                                             break;
	case MV_E_ABNORMAL_IMAGE:   errorMsg += "Abnormal image, maybe incomplete image because of lost packet ";   break;
	case MV_E_UNKNOW:           errorMsg += "Unknown error ";                                                   break;
	case MV_E_GC_GENERIC:       errorMsg += "General error ";                                                   break;
	case MV_E_GC_ACCESS:        errorMsg += "Node accessing condition error ";                                  break;
	case MV_E_ACCESS_DENIED:	errorMsg += "No permission ";                                                   break;
	case MV_E_BUSY:             errorMsg += "Device is busy, or network disconnected ";                         break;
	case MV_E_NETER:            errorMsg += "Network error ";                                                   break;
	}

	QMessageBox::information(NULL, "PROMPT", errorMsg);
}

void __stdcall MainWindow::ImageCallBack(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{
	if (pUser)
	{
		MainWindow *pMainWindow = (MainWindow*)pUser;
		pMainWindow->ImageCallBackInner(pData, pFrameInfo);
	}
}

void MainWindow::ImageCallBackInner(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo)
{

	MV_DISPLAY_FRAME_INFO stDisplayInfo;

	MV_CC_PIXEL_CONVERT_PARAM stConvertParam = { 0 };
	unsigned char * conv_pData = (unsigned char *)malloc(pFrameInfo->nWidth * pFrameInfo->nHeight * 3 + 2048);

	memset(&stConvertParam, 0, sizeof(MV_CC_PIXEL_CONVERT_PARAM));
	stConvertParam.nWidth = pFrameInfo->nWidth;                 
	stConvertParam.nHeight = pFrameInfo->nHeight;             
	stConvertParam.pSrcData = pData;                  
	stConvertParam.nSrcDataLen = pFrameInfo->nFrameLen;         
	stConvertParam.enSrcPixelType = pFrameInfo->enPixelType;    
	stConvertParam.enDstPixelType = PixelType_Gvsp_BGR8_Packed; 
	stConvertParam.pDstBuffer = conv_pData;                    
	stConvertParam.nDstBufferSize = (pFrameInfo->nWidth * pFrameInfo->nHeight * 3 + 2048);
	MV_CC_ConvertPixelType(handle, &stConvertParam);
	cv::Mat mat = cv::Mat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC3, conv_pData);
	image_processer.processImage(mat, display_origin);
	detections = image_processer.getDetections();

	catch_points = image_processer.getCatchPoints();
	memset(&stDisplayInfo, 0, sizeof(MV_DISPLAY_FRAME_INFO));

	stDisplayInfo.hWnd = m_hWnd;
	stDisplayInfo.pData = mat.data;
	// stDisplayInfo.pData = pData;
	stDisplayInfo.nDataLen = pFrameInfo->nFrameLen;
	stDisplayInfo.nWidth = pFrameInfo->nWidth;
	stDisplayInfo.nHeight = pFrameInfo->nHeight;
	stDisplayInfo.enPixelType = PixelType_Gvsp_BGR8_Packed;
	// stDisplayInfo.enPixelType = pFrameInfo->enPixelType;
	m_pcMyCamera->DisplayOneFrame(&stDisplayInfo);
	mat.release();
	free(conv_pData);
}

void MainWindow::showDetections()
{
	model.setRowCount(0);

	for (size_t i = 0; i < detections.size(); ++i)
	{
		model.setItem(i, 0, new QStandardItem(detections[i].first));
		model.setItem(i, 1, new QStandardItem(QString::number(detections[i].second)));
		model.setItem(i, 2, new QStandardItem(QString::number(catch_points[i].x)));
		model.setItem(i, 3, new QStandardItem(QString::number(catch_points[i].y)));
	}
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (m_pcMyCamera != NULL)
	{
		auto ret = QMessageBox::warning(NULL, "警告", "相机未关闭，是否仍要关闭?", QMessageBox::Yes, QMessageBox::No);
		if (ret == QMessageBox::No)
		{
			event->ignore();
		}
		if (ui->stop_button->isEnabled()) on_stop_button_clicked();
		if (ui->close_device_button->isEnabled()) on_close_device_button_clicked();
		else event->accept();

	}
	else
	{
		event->accept();
	}
}

/*Private Slots*/

void MainWindow::choseFile()
{
	QString calib_data_file = QFileDialog::getOpenFileName(this, QString("选择文件"), "", "Calibration info (*.calib)");
	if (calib_data_file == "") return;
	else
	{
		calib_params.clear();
		thresh = 100;
		QFile file(calib_data_file);
		file.open(QIODevice::ReadOnly);
		while (!file.atEnd())
		{
			QString line = file.readLine();
			bool isok = false;
			float tmp_num = line.toFloat(&isok);
			if (isok) calib_params.push_back(tmp_num);
		}
	}
	thresh = calib_params[0];
	calib_params.pop_front();

	image_processer = Image_Process(calib_params, thresh);
}

void MainWindow::on_send_stop_pushButton_clicked()
{
	if (detections.size() != catch_points.size()) return;
	//tcp_messager.sendMessage("Stop");
	emit sendMsg("Stop");
}

void MainWindow::on_send_msg_pushButton_clicked()
{
	if (detections.size() != catch_points.size()) return;
	QString msg_to_send;
	msg_to_send = "NUM=" + QString::number(int(catch_points.size()));
	//qDebug() << "msg: " << msg_to_send;
	//tcp_messager.sendMessage(msg_to_send);
	emit sendMsg(msg_to_send);

	for (int i = 0; i < detections.size(); ++i)
	{
		int letter_type = (detections[i].first.toStdString()[0] - 'A') + 1;
		msg_to_send = "PART=[" + QString::number(letter_type) + ",[" + QString::number(catch_points[i].y) + "," + QString::number(catch_points[i].x) + ",0]" +
			",[0,0," + QString::number(detections[i].second) + "]]";
		//qDebug() << "msg: " << msg_to_send;
		emit sendMsg(msg_to_send);
		// tcp_messager.sendMessage(msg_to_send);
	}
}


void MainWindow::on_listen_pushButton_clicked()
{
	if (ui->listen_pushButton->text() == "监听")
	{
		tcp_thread.start();
		QString ip = ui->ip_lineEdit->text();
		QHostAddress ip_address = (ip == "Any") ? QHostAddress::Any : QHostAddress(ui->ip_lineEdit->text());
		int port = ui->port_lineEdit->text().toInt();
		emit startListening(ip_address, port);
	}
	else
	{
		emit stopListening();
	}
}

void MainWindow::on_search_button_clicked()
{
	ui->device_list->clear();

	memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
	int nRet = CMvCamera::EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &m_stDevList);
	if (MV_OK != nRet)
	{
		return;
	}

	for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
	{
		QString strMsg;
		MV_CC_DEVICE_INFO* pDeviceInfo = m_stDevList.pDeviceInfo[i];
		if (NULL == pDeviceInfo)
		{
			continue;
		}

		if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
		{
			int nIp1 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
			int nIp2 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
			int nIp3 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
			int nIp4 = (m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

			if (strcmp("", (char*)pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName) != 0)
			{
				strMsg.sprintf("[%d]GigE:   %s  (%d.%d.%d.%d)", i, pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName, nIp1, nIp2, nIp3, nIp4);
			}
			else
			{
				strMsg.sprintf("[%d]GigE:   %s %s (%s)  (%d.%d.%d.%d)", i, pDeviceInfo->SpecialInfo.stGigEInfo.chManufacturerName,
					pDeviceInfo->SpecialInfo.stGigEInfo.chModelName, pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber, nIp1, nIp2, nIp3, nIp4);
			}
		}
		else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE)
		{
			if (strcmp("", (char*)pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName) != 0)
			{
				strMsg.sprintf("[%d]UsbV3:  %s", i, pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
			}
			else
			{
				strMsg.sprintf("[%d]UsbV3:  %s %s (%s)", i, pDeviceInfo->SpecialInfo.stUsb3VInfo.chManufacturerName,
					pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName, pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
			}
		}
		else
		{
			ShowErrorMsg("Unknown device enumerated", 0);
		}
		ui->device_list->addItem(strMsg);
	}

	if (0 == m_stDevList.nDeviceNum)
	{
		ShowErrorMsg("No device", 0);
		return;
	}
	ui->device_list->setCurrentIndex(0);
}

void MainWindow::on_open_device_button_clicked()
{
	int nIndex = ui->device_list->currentIndex();
	qDebug() << nIndex;
	if ((nIndex < 0) | (nIndex >= MV_MAX_DEVICE_NUM))
	{
		ShowErrorMsg("Please select device", 0);
		return;
	}

	if (NULL == m_stDevList.pDeviceInfo[nIndex])
	{
		ShowErrorMsg("Device does not exist", 0);
		return;
	}
	qDebug() << "info: " << m_stDevList.pDeviceInfo[nIndex];
	if (m_pcMyCamera == NULL)
	{
		m_pcMyCamera = new CMvCamera;
		if (NULL == m_pcMyCamera)
		{
			return;
		}
	}

	int nRet = m_pcMyCamera->Open(m_stDevList.pDeviceInfo[nIndex]);

	qDebug() << "ret: " << nRet;
	if (MV_OK != nRet)
	{
		delete m_pcMyCamera;
		m_pcMyCamera = NULL;
		ShowErrorMsg("Open Fail", nRet);
		return;
	}

	nRet = MV_CC_CreateHandle(&handle, m_stDevList.pDeviceInfo[nIndex]);
	if (MV_OK != nRet)
	{
		delete m_pcMyCamera;
		m_pcMyCamera = NULL;
		ShowErrorMsg("Open Fail", nRet);
		return;
	}
	if (m_stDevList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
	{
		unsigned int nPacketSize = 0;
		nRet = m_pcMyCamera->GetOptimalPacketSize(&nPacketSize);
		if (nRet == MV_OK)
		{
			nRet = m_pcMyCamera->SetIntValue("GevSCPSPacketSize", nPacketSize);
			if (nRet != MV_OK)
			{
				ShowErrorMsg("Warning: Set Packet Size fail!", nRet);
			}
		}
		else
		{
			ShowErrorMsg("Warning: Get Packet Size fail!", nRet);
		}
	}

	m_pcMyCamera->SetEnumValue("AcquisitionMode", MV_ACQ_MODE_CONTINUOUS);
	m_pcMyCamera->SetEnumValue("TriggerMode", MV_TRIGGER_MODE_OFF);

	ui->open_device_button->setEnabled(false);
	ui->close_device_button->setEnabled(true);

	ui->start_button->setEnabled(true);
	ui->stop_button->setEnabled(false);
}

void MainWindow::on_close_device_button_clicked()
{
	if (m_pcMyCamera)
	{
		m_pcMyCamera->Close();
		delete m_pcMyCamera;
		m_pcMyCamera = NULL;
	}
	m_bGrabbing = false;
	ui->open_device_button->setEnabled(true);
	ui->close_device_button->setEnabled(false);
	ui->start_button->setEnabled(false);
	ui->stop_button->setEnabled(false);
}

void MainWindow::on_start_button_clicked()
{

	if (calib_params.empty() || calib_params.size() != 6)
	{
		QMessageBox::warning(nullptr, "警告", "标定数据未导入或导入数据错误", QMessageBox::Ok);
		return;
	}
	m_pcMyCamera->RegisterImageCallBack(ImageCallBack, this);

	int nRet = m_pcMyCamera->StartGrabbing();
	if (MV_OK != nRet)
	{
		ShowErrorMsg("Start grabbing fail", nRet);
		return;
	}
	m_bGrabbing = true;
	on_getPara_button_clicked();
	ui->start_button->setEnabled(false);
	ui->stop_button->setEnabled(true);
	ui->exposure_spinbox->setEnabled(true);
	ui->gain_spinbox->setEnabled(true);
	ui->framerate_spinbox->setEnabled(true);
	ui->getPara_button->setEnabled(true);
	ui->setPara_button->setEnabled(true);

	ui->close_device_button->setDisabled(true);
	ui->binary_rbutton->setEnabled(true);
	ui->origin_rbutton->setEnabled(true);

	timer.start();
}

void MainWindow::on_stop_button_clicked()
{
	timer.stop();
	int nRet = m_pcMyCamera->StopGrabbing();
	if (MV_OK != nRet)
	{
		ShowErrorMsg("Stop grabbing fail", nRet);
		return;
	}
	m_bGrabbing = false;

	ui->start_button->setDisabled(false);
	ui->stop_button->setDisabled(true);
	ui->exposure_spinbox->setDisabled(true);
	ui->gain_spinbox->setDisabled(true);
	ui->framerate_spinbox->setDisabled(true);
	ui->getPara_button->setDisabled(true);
	ui->setPara_button->setDisabled(true);

	ui->close_device_button->setEnabled(true);
	ui->binary_rbutton->setDisabled(true);
	ui->origin_rbutton->setDisabled(true);


}

void MainWindow::on_getPara_button_clicked()
{
	MVCC_FLOATVALUE stFloatValue;
	memset(&stFloatValue, 0, sizeof(MVCC_FLOATVALUE));

	int nRet = m_pcMyCamera->GetFloatValue("ExposureTime", &stFloatValue);
	if (MV_OK != nRet)
	{
		ShowErrorMsg("Get Exposure Time Fail", nRet);
	}
	else
	{
		ui->exposure_spinbox->setValue(stFloatValue.fCurValue);
		emax = stFloatValue.fMax;
		emin = stFloatValue.fMin;
	}

	nRet = m_pcMyCamera->GetFloatValue("Gain", &stFloatValue);
	if (MV_OK != nRet)
	{
		ShowErrorMsg("Get Gain Fail", nRet);
	}
	else
	{
		ui->gain_spinbox->setValue(stFloatValue.fCurValue);
		gmax = stFloatValue.fMax;
		gmin = stFloatValue.fMin;
	}

	nRet = m_pcMyCamera->GetFloatValue("ResultingFrameRate", &stFloatValue);
	if (MV_OK != nRet)
	{
		ShowErrorMsg("Get Frame Rate Fail", nRet);
	}
	else
	{
		ui->framerate_spinbox->setValue(stFloatValue.fCurValue);
		fmax = stFloatValue.fMax;
		fmin = stFloatValue.fMin;
	}
}

void MainWindow::on_setPara_button_clicked()
{
	if (ui->exposure_spinbox->value() >= emin && ui->exposure_spinbox->value() <= emax)
	{
		m_pcMyCamera->SetEnumValue("ExposureAuto", 0);
		int nRet = m_pcMyCamera->SetFloatValue("ExposureTime", ui->exposure_spinbox->value());
		if (MV_OK != nRet)
		{
			ShowErrorMsg("Set Exposure Time Fail", nRet);
		}
	}

	if (ui->gain_spinbox->value() >= gmin && ui->gain_spinbox->value() <= gmax)
	{
		m_pcMyCamera->SetEnumValue("GainAuto", 0);
		int nRet = m_pcMyCamera->SetFloatValue("Gain", ui->gain_spinbox->value());
		if (MV_OK != nRet)
		{
			ShowErrorMsg("Set Gain Fail", nRet);
		}
	}

	if (ui->framerate_spinbox->value() >= fmin && ui->framerate_spinbox->value() <= fmax)
	{
		int nRet = m_pcMyCamera->SetFloatValue("AcquisitionFrameRate", ui->framerate_spinbox->value());
		if (MV_OK != nRet)
		{
			ShowErrorMsg("Set Frame Rate Fail", nRet);
		}
	}
}

void MainWindow::setPara(double value)
{
	qDebug() << "value: " << value;
	on_setPara_button_clicked();
}