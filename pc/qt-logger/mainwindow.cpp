#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

/*!
 * \brief Constructor.
 */
MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	m_usbDevice = new QSerialPort(this);
	connect(m_usbDevice,SIGNAL(readyRead()),this,SLOT(onSerialDataAvailable()));

	m_socket = new QTcpSocket(this);
	connect(m_socket, SIGNAL(connected()),this, SLOT(connected()));
	connect(m_socket, SIGNAL(disconnected()),this, SLOT(disconnected()));
	connect(m_socket, SIGNAL(readyRead()),this, SLOT(readyRead()));

	m_baudrate = 1500000;
	m_varCounter = 0;
	m_serialDeviceIsConnected = false;
	m_networkDeviceIsConnected = false;
	getAvalilableSerialDevices();
	applyStyle( true );
	m_lastChart = -1;
}
//====================================================================================

/*!
 * \brief Destructor.
 */
MainWindow::~MainWindow()
{
	delete ui;
	delete m_usbDevice;
}
//====================================================================================
/*
QMessageBox::information(this, tr("QT logger"),
						 tr("The following error occurred: %1.")
						 .arg(m_socket->errorString()));
*/
/*!
 * \brief Get serial devices.
 */
void MainWindow::getAvalilableSerialDevices()
{
	qDebug() << "Number of available ports: " << QSerialPortInfo::availablePorts().length();
	m_serialComPortList.clear();
	ui->serialPortSelect_comboBox->clear();
	foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
		QString dbgStr = "Vendor ID: ";
		if (serialPortInfo.hasVendorIdentifier()) {
			dbgStr+= serialPortInfo.vendorIdentifier();
		} else {
			dbgStr+= " - ";
		}
		dbgStr+= "  Product ID: ";
		if (serialPortInfo.hasProductIdentifier()) {
			dbgStr+= serialPortInfo.hasProductIdentifier();
		} else {
			dbgStr+= " - ";
		}
		dbgStr+= " Name: " + serialPortInfo.portName();
		dbgStr+= " Description: "+serialPortInfo.description();
		qDebug()<<dbgStr;
		m_serialComPortList.push_back(serialPortInfo);
		ui->serialPortSelect_comboBox->addItem(serialPortInfo.portName() +" "+serialPortInfo.description());
	}
	{
		QSerialPortInfo serialPortInfo("TCP");
		m_serialComPortList.push_back(serialPortInfo);
		ui->serialPortSelect_comboBox->addItem("TCP");
	}
}
//====================================================================================

/*!
 * \brief Write to serial port.
 * \param message - message to write.
 */
void MainWindow::serialWrite(QString message)
{
	if (m_serialDeviceIsConnected) {
		m_usbDevice->write(message.toUtf8()); // Send the message to the device
		qDebug() << "Message to serial device: "<<message;
	}
	if (m_networkDeviceIsConnected) {
		m_socket->write(message.toUtf8());
		qDebug() << "Message to TCP socket: "<<message;
	}
}
//====================================================================================

/*!
 * \brief Calculate twos compliment.
 * \param value - ADU value from accelerometer.
 * \return converted value.
 */
static qint32 TwosCompliment(quint32 value);
static qint32 TwosCompliment(quint32 value)
{
	return ((qint32)value << 12) >> 12;
}
//===========================================================================================


/*!
 * \brief Calculate std deviation.
 */
static double CalcStdDev(int *val, int cnt)
{
	int i;
	double t, v = 0.0,aver = 0.0;

	// Caltulate average.
	for (i=0;i<cnt;++i) {
		aver += ((double)val[i]);
	}
	aver/=((double)cnt);
	// Caltulate variance.
	for (i=0;i<cnt;++i) {
		t = aver - ((double)val[i]);
		v += t*t;
	}
	v/=((double)cnt);
	return sqrt(v);
}
//===========================================================================================



/*!
 * \brief Parse line from ESP8266 (D1 mini).
 * \param s - line to parse.
 */
void MainWindow::parseLine(QString &s)
{
	int id, x, y, z;
	bool ok;
	QStringList v = s.split(";");
	QString res;
	if (v.count() >= 3) {
		x = v.at(0).toUInt(&ok);
		y = v.at(1).toUInt(&ok);
		z = v.at(2).toUInt(&ok);
		/* Measurement */
		res = QString("%1;%2;%3\r\n").arg(x).arg(y).arg(z);
		m_counter++;
		m_varBufferX[m_varCounter] = x;
		m_varBufferY[m_varCounter] = y;
		m_varBufferZ[m_varCounter] = z;
		m_varCounter++;
		m_file.write(QtoA(res));
		m_file.flush();
		if (m_varCounter == VAR_CNT) {
			int rate = (m_varCounter*1000u)/m_etimer.elapsed();
			qDebug()<<"Measurements = "<<m_counter<<", elapsed = "<<m_etimer.elapsed()<<", rate = " << rate << "Hz";
			m_etimer.start();
			ui->vX->setText(QString("%1").arg(x));
			ui->vY->setText(QString("%1").arg(y));
			ui->vZ->setText(QString("%1").arg(z));
			ui->vF->setText(QString("%1 Hz").arg(rate));
			ui->varX->setText(QString("%1").arg(CalcStdDev(m_varBufferX,VAR_CNT)));
			ui->varY->setText(QString("%1").arg(CalcStdDev(m_varBufferY,VAR_CNT)));
			ui->varZ->setText(QString("%1").arg(CalcStdDev(m_varBufferZ,VAR_CNT)));
			m_varCounter = 0;
			{
				int chartid = ui->comboBoxChart->currentIndex();
				if (m_lastChart != chartid) {
					m_lastChart = chartid;
					ui->chart->clear();
				}
				switch(chartid) {
				case 1: {ui->chart->populateData(m_varBufferX,VAR_CNT);} break;
				case 2: {ui->chart->populateData(m_varBufferY,VAR_CNT);} break;
				case 3: {ui->chart->populateData(m_varBufferZ,VAR_CNT);} break;
				default:break;
				}
			}
		}
	}
	//qDebug()<<"Got line: "<<v;

}
//====================================================================================

/*!
 * \brief New data available - divide data into lines.
 */
void MainWindow::onSerialDataAvailable()
{
	if (m_serialDeviceIsConnected == true) {
		QByteArray b = m_usbDevice->readAll();
		const char *buf = b.constData();
		int i,len = b.length();

		for (i=0; i<len; ++i) {
			char ch = buf[i];
			switch (ch) {
			case '\r':
			case '\n': {
				if (m_lineBuffer.length()) {
					parseLine(m_lineBuffer);
				}
				m_lineBuffer.clear();
			} break;
			default: {
				m_lineBuffer.append(ch);
			}
			}
		}
	}
}
//====================================================================================

/*!
 * \brief Remove extension from file name.
 * \param fileName - file name.
 */
inline QString withoutExtension(const QString & fileName)
{
	return fileName.left(fileName.lastIndexOf("."));
}
//====================================================================================

/*!
 * \brief Remove date marker from file name.
 * \param fileName - file name.
 */
inline QString withoutDate(const QString & fileName)
{
	return fileName.left(fileName.lastIndexOf("_"));
}
//====================================================================================

void MainWindow::openUART(bool b)
{

	QString portName = m_serialComPortList[ui->serialPortSelect_comboBox->currentIndex()].portName();

	/* Close UART connection */
	if (m_serialDeviceIsConnected) {
		m_file.close();
		m_usbDevice->close();
		m_serialDeviceIsConnected = false;
	}
	/* Close TCP connection */
	if (m_networkDeviceIsConnected) {
		m_file.close();
		m_socket->close();
		m_networkDeviceIsConnected = false;
	}
	if (b) {
		if (portName.isEmpty()) {
			/* This is TCP socket */
			m_socket->connectToHost("192.168.10.1", 2500);
			// we need to wait...
			if(!m_socket->waitForConnected(5000)) {
				//qDebug() << "Error: " << m_socket->errorString();
				QMessageBox::information(this, tr("QT logger"),
										 tr("Nie mogę połączyć się przez TCP: %1.")
										 .arg(m_socket->errorString()));
			} else {
				statusBar()->showMessage("Połączono przez TCP :-)",5000);
			}
		} else {
			/* This is serial port */
			if (!m_serialDeviceIsConnected) {
				clearVariables();
				m_usbDevice->setPortName(portName);
				m_deviceDescription = m_serialComPortList[ui->serialPortSelect_comboBox->currentIndex()].description();
				qDebug() << "connecting to: "<<m_usbDevice->portName();
				if (m_usbDevice->open(QIODevice::ReadWrite)) {
					//Now the serial port is open try to set configuration
					if (!m_usbDevice->setBaudRate(m_baudrate)) {       //Depends on your boud-rate on the Device
						qDebug()<<m_usbDevice->errorString();
					}
					if (!m_usbDevice->setDataBits(QSerialPort::Data8)) {
						qDebug()<<m_usbDevice->errorString();
					}
					if (!m_usbDevice->setParity(QSerialPort::NoParity)) {
						qDebug()<<m_usbDevice->errorString();
					}
					if (!m_usbDevice->setStopBits(QSerialPort::OneStop)) {
						qDebug()<<m_usbDevice->errorString();
					}
					if (!m_usbDevice->setFlowControl(QSerialPort::NoFlowControl)) {
						qDebug()<<m_usbDevice->errorString();
					}
					//If any error was returned the serial il corrctly configured
					qDebug() << "Connection to: "<< m_usbDevice->portName() << " " << m_deviceDescription << " connected";
					m_serialDeviceIsConnected = true;
					statusBar()->showMessage("Połączono przez UART :-)",5000);
				} else {
					qDebug() << "Connection to: "<< m_usbDevice->portName() << " " << m_deviceDescription << " not connected";
					qDebug() <<"         Error: "<<m_usbDevice->errorString();
					m_serialDeviceIsConnected = false;
					m_file.close();
					QMessageBox::information(this, tr("QT logger"),
											 tr("Nie mogę połączyć się z portem UART: %1.")
											 .arg(m_usbDevice->errorString()));
				}
			}
		}
	}
}
//====================================================================================

void MainWindow::on_filter_button_clicked()
{
	bool closeAfterOperation = false, opOk = false;

	if (m_serialDeviceIsConnected == false) {
		closeAfterOperation = true;
		openUART(true);
	}
	if (m_serialDeviceIsConnected) {
		/* Configure filter */
		int vlpf = ui->comboBoxLPF->currentIndex();
		int vhpf = ui->comboBoxHPF->currentIndex();
		int v = ((vhpf<<4) | vlpf);
		QString cmd = QString("W28%1\n").arg(v,2,16,QLatin1Char('0'));
		serialWrite(cmd);
		opOk = true;
	}
	if (closeAfterOperation) {
		openUART(false);
	}
}
//====================================================================================

/*!
 * \brief Connect button clicked.
 */
void MainWindow::on_connect_button_clicked()
{
	openUART(false);
	ui->chart->clear();
	m_file.close();
	const QString DEFAULT_DIR_KEY("default_dir");
	QString sfile;
	{
		sfile = withoutDate(withoutExtension(m_settings.value(DEFAULT_DIR_KEY).toString()));
		/* Add date/time */
		QDateTime local(QDateTime::currentDateTime());
		sfile += local.toString("_yyyy-MM-ddThh-mm-ss")+".csv";
	}
	QString fname = QFileDialog::getSaveFileName(nullptr, "Plik do zapisu", sfile, "Text files (*.csv)" );
	qDebug() << "name is : " << fname;
	if (fname.isEmpty()) return;
	QDir CurrentDir;
	m_settings.setValue(DEFAULT_DIR_KEY,CurrentDir.absoluteFilePath(fname));
	m_file.setFileName(fname);
	if (m_file.open(QIODevice::WriteOnly | QIODevice::Append)) {
		m_file.write("X[adu];Y[adu];Z[adu]\r\n");
		openUART(true);
	}
}
//====================================================================================

/*!
 * \brief Disconnect button clicked.
 */
void MainWindow::on_disconnect_button_clicked()
{
	openUART(false);
}
//====================================================================================

/*!
 * \brief Set application style (light/dark).
 */
void MainWindow::applyStyle( bool dark )
{
	QString path;
	if (dark) {
		/* Setup dark style palette */
		QColor darkGray(53, 53, 53);
		QColor gray(128, 128, 128);
		QColor black(25, 25, 25);
		QColor blue(42, 130, 218);

		m_darkPalette.setColor(QPalette::Window, darkGray);
		m_darkPalette.setColor(QPalette::WindowText, Qt::white);
		m_darkPalette.setColor(QPalette::Base, black);
		m_darkPalette.setColor(QPalette::AlternateBase, darkGray);
		m_darkPalette.setColor(QPalette::ToolTipBase, blue);
		m_darkPalette.setColor(QPalette::ToolTipText, Qt::white);
		m_darkPalette.setColor(QPalette::Text, Qt::white);
		m_darkPalette.setColor(QPalette::Button, darkGray);
		m_darkPalette.setColor(QPalette::ButtonText, Qt::white);
		m_darkPalette.setColor(QPalette::Link, blue);
		m_darkPalette.setColor(QPalette::Highlight, blue);
		m_darkPalette.setColor(QPalette::HighlightedText, Qt::black);

		m_darkPalette.setColor(QPalette::Active, QPalette::Button, gray.darker());
		m_darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, gray);
		m_darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, gray);
		m_darkPalette.setColor(QPalette::Disabled, QPalette::Text, gray);
		m_darkPalette.setColor(QPalette::Disabled, QPalette::Light, darkGray);
		qApp->setPalette(m_darkPalette);
		path = ":default/dark.qss";
	} else {
		QColor darkGray(153, 153, 153);
		QColor gray(128, 128, 128);
		QColor blue(42, 130, 218);

		m_darkPalette.setColor(QPalette::Window, Qt::white);
		m_darkPalette.setColor(QPalette::WindowText, Qt::black);
		m_darkPalette.setColor(QPalette::Base, Qt::white);
		m_darkPalette.setColor(QPalette::AlternateBase, Qt::white);
		m_darkPalette.setColor(QPalette::ToolTipBase, blue);
		m_darkPalette.setColor(QPalette::ToolTipText, Qt::black);
		m_darkPalette.setColor(QPalette::Text, Qt::black);
		m_darkPalette.setColor(QPalette::Button, darkGray);
		m_darkPalette.setColor(QPalette::ButtonText, Qt::black);
		m_darkPalette.setColor(QPalette::Link, blue);
		m_darkPalette.setColor(QPalette::Highlight, blue);
		m_darkPalette.setColor(QPalette::HighlightedText, Qt::black);

		m_darkPalette.setColor(QPalette::Active, QPalette::Button, gray.darker());
		m_darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, gray);
		m_darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, gray);
		m_darkPalette.setColor(QPalette::Disabled, QPalette::Text, gray);
		m_darkPalette.setColor(QPalette::Disabled, QPalette::Light, darkGray);
		qApp->setPalette(m_darkPalette);
		path = ":default/light.qss";
	}
	/* Apply qss file */
	QFile f(path);
	if (!f.exists()) {
		qDebug("Unable to set stylesheet, file not found\n");
	} else {
		f.open(QFile::ReadOnly | QFile::Text);
		QTextStream ts(&f);
		qApp->setStyleSheet(ts.readAll());
	}
}
//===========================================================================================

void MainWindow::connected()
{
	m_networkDeviceIsConnected = true;
}
//===========================================================================================

void MainWindow::disconnected()
{
	m_networkDeviceIsConnected = false;
}
//===========================================================================================

void MainWindow::readyRead()
{
	if (m_networkDeviceIsConnected == true) {
		QByteArray b = m_socket->readAll();
		const char *buf = b.constData();
		int i,len = b.length();

		for (i=0; i<len; ++i) {
			char ch = buf[i];
			switch (ch) {
			case '\r':
			case '\n': {
				if (m_sockLineBuffer.length()) {
					parseLine(m_sockLineBuffer);
				}
				m_sockLineBuffer.clear();
			} break;
			default: {
				m_sockLineBuffer.append(ch);
			}
			}
		}
	}
}
//===========================================================================================
