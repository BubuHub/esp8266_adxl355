#include "widget.h"
#include "xyseriesiodevice.h"

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>


#include <QtWidgets/QVBoxLayout>
#include <QDebug>

QT_CHARTS_USE_NAMESPACE

Widget::Widget(QWidget *parent): QWidget(parent), m_chart(new QChart), m_series(new QLineSeries)
{
	QChartView *chartView = new QChartView(m_chart);
	chartView->setMinimumSize(300, 300);
	m_chart->addSeries(m_series);
	QValueAxis *axisX = new QValueAxis;
	axisX->setRange(0, XYSeriesIODevice::sampleCount);
	axisX->setLabelFormat("%g");
	//axisX->setTitleText("");
	m_axisY = new QValueAxis;
	//m_axisY->setRange(-1, 1);
	//m_axisY->setTitleText("Audio level");
	m_chart->addAxis(axisX, Qt::AlignBottom);
	m_series->attachAxis(axisX);
	m_chart->addAxis(m_axisY, Qt::AlignLeft);
	m_series->attachAxis(m_axisY);
	m_chart->legend()->hide();
	//m_chart->setTitle("Accelerometer");

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(chartView);


	m_device = new XYSeriesIODevice(m_series, this);
	m_device->open(QIODevice::WriteOnly);
}
//====================================================================================

Widget::~Widget()
{
	m_device->close();
}
//====================================================================================

void Widget::clear()
{
	m_device->m_first = true;
}
//====================================================================================

void Widget::populateData(int *data, int cnt)
{
	m_device->populateData(data, cnt);
	m_axisY->setRange(m_device->yMin-20, m_device->yMax+20);
}
//====================================================================================
