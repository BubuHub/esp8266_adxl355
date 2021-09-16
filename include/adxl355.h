#ifndef __ADXL355_H__
#define __ADXL355_H__
#include "hspi.h"

//#define GPIO_CS         D8


// Memory register addresses:
#define REG_DEVID_AD               (0x00)
#define REG_DEVID_MST              (0x01)
#define REG_PARTID                 (0x02)
#define REG_REVID                  (0x03)
#define REG_STATUS                 (0x04)
#define REG_FIFO_ENTRIES           (0x05)
#define REG_TEMP2                  (0x06)
#define REG_TEMP1                  (0x07)
#define REG_XDATA3                 (0x08)
#define REG_XDATA2                 (0x09)
#define REG_XDATA1                 (0x0a)
#define REG_YDATA3                 (0x0b)
#define REG_YDATA2                 (0x0c)
#defineint REG_YDATA1              (0x0d)
#defineREG_ZDATA3                  (0x0e)
#define REG_ZDATA2                 (0x0f)
#define REG_ZDATA1                 (0x10)
#define REG_FIFO_DATA              (0x11)

#define REG_OFFSET_X_H             (0x1e)
#define REG_OFFSET_X_L             (0x1f)
#define REG_OFFSET_Y_H             (0x20)
#define REG_OFFSET_Y_L             (0x21)
#define REG_OFFSET_Z_H             (0x22)
#define REG_OFFSET_Z_L             (0x23)
#define REG_FILTER                 (0x28)
#define REG_RANGE                  (0x2c)
#define REG_POWER_CTL              (0x2d)
#define REG_RESET                  (0x2f)



// Device values
#define RANGE_2G     (0x01)
#define RANGE_4G     (0x02)
#define RANGE_8G     (0x03)
#define MEASURE_MODE (0x06)

// Operations
#define READ_BYTE  (0x01)
#define WRITE_BYTE (0x00)


/* 
 * Write registry in specific device address
 */
static void writeRegister(byte thisRegister, byte thisValue);
static void writeRegister(byte thisRegister, byte thisValue)
{
	uint16_t dataToSend = (thisRegister << 1) | WRITE_BYTE | ((uint16_t)thisValue)<<8;
	hspi_wait_ready();
	hspi_send_uint16(dataToSend);
	hspi_wait_ready();
}
//===========================================================================================

/* 
 * Read registry in specific device address
 */
static unsigned int readRegistry(byte thisRegister) 
{
	unsigned int result = 0;
	byte dataToSend = (thisRegister << 1) | READ_BYTE;
	hspi_prepare_txrx(1,1);
	*spi_fifo = dataToSend;
	hspi_start_tx();
	hspi_wait_ready();
	return *spi_fifo;
}
//===========================================================================================

/* 
 * Read multiple registries
 */
void readBytes(int addresses, int *readedData, int dataSize) 
{
	uint8_t *d = (uint8_t *)spi_fifo;
	byte dataToSend = (addresses << 1) | READ_BYTE;
	hspi_prepare_txrx(1,dataSize);
	*spi_fifo = dataToSend;
	hspi_start_tx();
	hspi_wait_ready();
	for(int i = 0; i < dataSize; i = i + 1) {
		readedData[i] = d[i];
	}
}
//===========================================================================================

/* 
 * Read multiple registries
 */
void readBytes2(int addresses, int *readedData, int dataSize, void (*fn)()) 
{
	uint8_t *d = (uint8_t *)spi_fifo;
	byte dataToSend = (addresses << 1) | READ_BYTE;
	hspi_prepare_txrx(1,dataSize);
	*spi_fifo = dataToSend;
	hspi_start_tx();
	fn();
	hspi_wait_ready();
	for(int i = 0; i < dataSize; i = i + 1) {
		readedData[i] = d[i];
	}
}
//===========================================================================================


int getFifoCount()
{
	return readRegistry(REG_FIFO_ENTRIES);
}
//===========================================================================================

static int32_t FixComplement(uint32_t value);
static int32_t FixComplement(uint32_t value)
{
	return ((int32_t)value << 12) >> 12;
}
//===========================================================================================

#endif