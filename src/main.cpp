#include <Arduino.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
  #include "user_interface.h"
}
//#include <SPI.h>
#include "Ticker.h"
#include "adxl355.h"
#include <vector>

/*
 * - PCB CONNECTIONS -
 * GND        - GND
 * VCC        - 3.3V
 * SCLK       - D5 GPIO14 (SCL)
 * MISO       - D6 GPIO12 (MISO)
 * MOSI       - D7 GPIO13 (MOSI)
 * CS         - D8 GPIO15 (CS)
 */

//====================================================================================
//===============================--- DEFINITIONS --===================================
//====================================================================================
#define GPIO_LED        2

#if 1
#undef DEBUG_ENABLED
#define pdebug(fmt, args...)
#define pwrite(fmt, len)
#else
#define DEBUG_ENABLED
#define pdebug(fmt, args...) Serial.printf(fmt, ## args)
#define pwrite(fmt, len) Serial.write(fmt, len)
#endif

#define BUFFER_SIZE (4 * 1024)

/* Serial port speed bps 2Mb works fine on WebMos D1 */
#define serialSpeed        (1500000ul)

//====================================================================================
//============================--- GLOBAL VARIABLES --=================================
//====================================================================================
volatile int    g_send_in_progress = 0;       /*!< Send in progress flag 1 - sending in progress */
unsigned int    g_measureCounter = 0;         /*!< Measurement couter.                           */
volatile int    g_serialMode = 1;             /*!< Serial mode flag.                             */
Ticker          g_ticker;                     /*!< Ticker.                                       */
char            g_buffer[256];                /*!< Global cmd line buffer.                       */
unsigned char   g_cnt = 0;                    /*!< Global line buffer byte counter.              */
int             g_blink = 0;                  /*!< Led blink counter.                            */
int             g_phase = 0;
int             xdata, ydata, zdata;

static int readFifoEntries();


//===========================================================================================
//==================================-- The code --===========================================
//===========================================================================================

/*!
 * \brief Setup.
 */
void setup()
{
	/* ========--- Disable WiFI ---======== */
	WiFi.mode(WIFI_OFF);
	WiFi.forceSleepBegin();
	delay(1); //Needed, at least in my tests WiFi doesn't power off without this for some reason
	pinMode(GPIO_LED,OUTPUT);
	pinMode(D6,INPUT_PULLUP);
	digitalWrite(GPIO_LED, LOW);

	Serial.begin(serialSpeed);
	Serial.setRxBufferSize(BUFFER_SIZE);
	pdebug("Setup start\n");

	//SPI.begin();
	//SPI.setHwCs(false);
	//SPI.setFrequency(2000000);
	hspi_init();
	delay(100);

	//Configure ADXL355:
	Serial.printf("start\r\nval = %d\r\n",readRegistry(REG_DEVID_AD));
	writeRegister(REG_RANGE, RANGE_2G); // 2G
	writeRegister(REG_POWER_CTL, MEASURE_MODE); // Enable measure mode

	// Give the sensor time to set up:
	delay(100);
}
//===========================================================================================

static int hexDigitToInt(char digit)
{
	if ('0' <= digit && digit <= '9') {
		return (int)(digit - '0');
	} else if ('a' <= digit && digit <= 'f') {
		return (int)(digit - ('a' - 10));
	} else if ('A' <= digit && digit <= 'F') {
		return (int)(digit - ('A' - 10));
	}
	return -1; //value not in [0-9][a-f] range
}
//====================================================================================

static int getVal8Hex(char *incomingData)
{
	return (hexDigitToInt(incomingData[0]) << 4) | hexDigitToInt(incomingData[1]);
}
//====================================================================================


/*!
 * \brief Execute command.
 */
static void OnCommand(void)
{
	if (g_cnt > 2) {
		switch((char)g_buffer[0]) {
			case 'W': {
				/* Write register */
				writeRegister(getVal8Hex(&g_buffer[1]), getVal8Hex(&g_buffer[3]));
				Serial.write("W:OK\n", 5);
			} break;
			case 'R': {
				/* Read register */
				int v = readRegistry(getVal8Hex(&g_buffer[1]));
				g_cnt = sprintf(g_buffer,"R:%02x\n", v);
				Serial.write(g_buffer, g_cnt);
			} break;
			case 'Q': {
				g_serialMode = 0;
			} break;
			case 'N': {
				g_serialMode = 1;
			} break;
			default: break;
		}
	}
	g_cnt = 0;
}
//====================================================================================

/*!
 * \brief Synchronize FIFO (find X axis marker).
 */
static int waitFirstX()
{
	int fifoCount = getFifoCount();
	int axisMeasures[] =  {0, 0, 0};
	for (int i = 0; i < fifoCount; i++) {
		readBytes(REG_FIFO_DATA, axisMeasures, 3);
		if ((axisMeasures[2]&1)) {
			readBytes(REG_FIFO_DATA, axisMeasures, 3);
			readBytes(REG_FIFO_DATA, axisMeasures, 3);
			return 1;
		}
	}
	return 0;
}
//===========================================================================================


/*!
 * \brief Process data (display on serial port for example).
 */
void processData()
{
	if (g_serialMode) {
		while (Serial.availableForWrite() < 24);
		Serial.print(xdata);
		Serial.print(";");
		Serial.print(ydata);
		Serial.print(";");
		Serial.print(zdata);
		Serial.print("\n");
	}
}
//===========================================================================================


/*!
 * \brief Read data from FIFO.
 */
static int readFifoEntries()
{
	int fifoCount = getFifoCount();
	int axisMeasures[] =  {0, 0, 0, 0, 0, 0, 0, 0, 0};

	for (int i = 0; i < fifoCount / 3; i++) {
		readBytes2(REG_FIFO_DATA, axisMeasures, 9, processData);
		if ((axisMeasures[2]&1) == 0) {
				Serial.print("Error - no X axis\n");
				return waitFirstX();
		}
		if ((axisMeasures[2]&2)) {
				Serial.print("Error - Fifo is empty\n");
		}
		xdata = (axisMeasures[2] >> 4) + (axisMeasures[1] << 4) + (axisMeasures[0] << 12);
		ydata = (axisMeasures[5] >> 4) + (axisMeasures[4] << 4) + (axisMeasures[3] << 12);
		zdata = (axisMeasures[8] >> 4) + (axisMeasures[7] << 4) + (axisMeasures[6] << 12);

		xdata = FixComplement(xdata);
		ydata = FixComplement(ydata);
		zdata = FixComplement(zdata);
	}
	return fifoCount / 3;
}
//===========================================================================================

/*!
 * \brief Event loop
 */
void loop()
{
	readFifoEntries();
	while (Serial.available()) {
		char c = Serial.read();
		g_buffer[g_cnt++] = c;
		if (c == '\n')  {
			g_buffer[g_cnt] = '\0';
			OnCommand();
		}
	}
}
//===========================================================================================

