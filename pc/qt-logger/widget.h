#ifndef WIDGET_H
#define WIDGET_H

#include <QtWidgets/QWidget>
#include <QtCharts/QChartGlobal>
#include <QtCharts/QValueAxis>

QT_CHARTS_BEGIN_NAMESPACE
class QLineSeries;
class QChart;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

class XYSeriesIODevice;

QT_BEGIN_NAMESPACE
class QAudioInput;
class QAudioDeviceInfo;
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
	explicit Widget(QWidget *parent = nullptr);
	~Widget();
	void populateData(int *data, int cnt);
	void clear();
private:
	XYSeriesIODevice *m_device = nullptr;
	QChart           *m_chart;
	QLineSeries      *m_series ;
	QValueAxis       *m_axisY;
};

#endif // WIDGET_H
