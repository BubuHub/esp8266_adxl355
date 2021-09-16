#include "xyseriesiodevice.h"

#include <QtCharts/QXYSeries>

XYSeriesIODevice::XYSeriesIODevice(QXYSeries *series, QObject *parent) :QIODevice(parent), m_series(series)
{
	m_first = true;
}
//====================================================================================

qint64 XYSeriesIODevice::readData(char *data, qint64 maxSize)
{
	Q_UNUSED(data)
	Q_UNUSED(maxSize)
	return -1;
}
//====================================================================================

void XYSeriesIODevice::populateData(int *data, int cnt)
{
	if (m_buffer.isEmpty()) {
		m_buffer.reserve(sampleCount);
		for (int i = 0; i < sampleCount; ++i) {
			m_buffer.append(QPointF(i, 0));
		}
	}
	int start = 0;
	const int availableSamples = cnt;
	if (availableSamples < sampleCount) {
		start = sampleCount - availableSamples;
		for (int s = 0; s < start; ++s) {
			m_buffer[s].setY(m_buffer.at(s + availableSamples).y());
		}
	}
	for (int s = start; s < sampleCount; ++s, data ++) {
		int y = *data;
		m_buffer[s].setY(y);
		if (m_first) {
			m_first = false;
			yMin = yMax = y;
		} else if (y < yMin || y > yMax) {
			if (y < yMin) yMin = y;
			if (y> yMax) yMax = y;
		}
	}

	m_series->replace(m_buffer);
}
//====================================================================================

qint64 XYSeriesIODevice::writeData(const char *data, qint64 maxSize)
{
	static const int resolution = 4;

	if (m_buffer.isEmpty()) {
		m_buffer.reserve(sampleCount);
		for (int i = 0; i < sampleCount; ++i)
			m_buffer.append(QPointF(i, 0));
	}

	int start = 0;
	const int availableSamples = int(maxSize) / resolution;
	if (availableSamples < sampleCount) {
		start = sampleCount - availableSamples;
		for (int s = 0; s < start; ++s)
			m_buffer[s].setY(m_buffer.at(s + availableSamples).y());
	}

	for (int s = start; s < sampleCount; ++s, data += resolution)
		m_buffer[s].setY(qreal(uchar(*data) -128) / qreal(128));

	m_series->replace(m_buffer);
	return (sampleCount - start) * resolution;
}
//====================================================================================
