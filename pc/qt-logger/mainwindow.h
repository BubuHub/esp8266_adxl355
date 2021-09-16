#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QPalette>
#include <QSettings>
#include <QTcpSocket>
#include <QPointer>
#include <vector>


#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#define QtoA(x) (x).toAscii()
#define QtoC(x) (x).toAscii().constData()
#define QtoD(x) (x).toAscii().data()
#else
#define QtoA(x) (x).toLatin1()
#define QtoC(x) (x).toLatin1().constData()
#define QtoD(x) (x).toLatin1().data()
#endif


#define VAR_CNT (512)

#define NopForTr(x)

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

	void applyStyle( bool dark );
	void parseLine(QString &s);
	void clearVariables() {
		m_counter = 0;
		m_varCounter = 0;
	}
private slots:
	void onSerialDataAvailable();
	void on_connect_button_clicked();
	void on_disconnect_button_clicked();
	void on_filter_button_clicked();
	void openUART(bool b);

	void connected();
	void disconnected();
	void readyRead();
private:
	void getAvalilableSerialDevices();
	void serialWrite(QString message);

	Ui::MainWindow              *ui;
	qint32                       m_baudrate;
	QSerialPort                 *m_usbDevice;
	QTcpSocket                  *m_socket;
	std::vector<QSerialPortInfo> m_serialComPortList; //A list of the available ports for the dropdownmenue in the GUI
	QString                      m_deviceDescription;
	bool                         m_serialDeviceIsConnected;
	bool                         m_networkDeviceIsConnected;
	QPalette                     m_orgPalette;
	QPalette                     m_darkPalette;
	QString                      m_lineBuffer;
	QString                      m_sockLineBuffer;
	QSettings                    m_settings;
	QFile                        m_file;
	quint64                      m_counter;
	QElapsedTimer                m_etimer;
	int                          m_varBufferX[VAR_CNT];
	int                          m_varBufferY[VAR_CNT];
	int                          m_varBufferZ[VAR_CNT];
	int                          m_varCounter;
	int                          m_lastChart;
};

#endif // MAINWINDOW_H
