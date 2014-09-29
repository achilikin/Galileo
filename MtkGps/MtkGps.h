/*	Apache 2.0 License
	
	Copyright (c) 2014 Andrey Chilikin https://github.com/achilikin

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/
#ifndef __MTK_GPS_H__
#define __MTK_GPS_H__

/*
	Galileo library for GPS modules compatible with MediaTek PMTK protocol,
	tested with the Adafruit Ultimate GPS GTop module using MTK3399 chipset
	http://www.adafruit.com/products/746
	https://learn.adafruit.com/adafruit-ultimate-gps
*/

#include <Arduino.h>
#include "TTYUART.h"
#include "nmea.h"

// ON/OFF arguments
#define PMTK_ARG_ON		1
#define PMTK_ARG_OFF	0

// PMTK_SET_BAUD arguments 
#define PMTK_BR_DEFAULT 0 // resets MTK chip to default speed
#define PMTK_BR_4800    4800
#define PMTK_BR_9600    9600
// #define PMTK_BR_14400   14400 // unreliable, do not use
#define PMTK_BR_19200   19200
#define PMTK_BR_38400   38400
#define PMTK_BR_57600   57600
#define PMTK_BR_115200  115200

// no MTK detected
#define PMTK_BR_INVALID 0

// PMTK_SET_DGPS_MODE arguments
#define  PMTK_DGPS_NONE 0
#define  PMTK_DGPS_RTCM 1
#define  PMTK_DGPS_WAAS 2

// PMTK_SET_DYN_MODE arguments
#define  PMTK_DGPS_FIXED   0  // stationary
#define  PMTK_DGPS_SLOW_GV 3  // slow ground vehicle
#define  PMTK_DGPS_FAST_GV 4  // fast ground vehicle
#define  PMTK_DGPS_SLOW_AB 5  // low dynamic airborne

// PMTK_SET_SBAS_MODE arguments
#define  PMTK_SBAS_TESTING   0
#define  PMTK_SBAS_INTEGRITY 1

// PMTK_SET_NAV_THRESHOLD arguments, 0.2/0.4/.../2.0 (m/s)
#define PMTK_NAV_THRESHOLD_OFF 0
#define PMTK_NAV_THRESHOLD_02  1 // 0.2 m/s or 0.72 km/h or 0.389 knot
#define PMTK_NAV_THRESHOLD_04  2 // 0.4 m/s or 1.44 km/h or 0.778 knot
#define PMTK_NAV_THRESHOLD_06  3 // 0.6 m/s or 2.16 km/h or 1.166 knot
#define PMTK_NAV_THRESHOLD_08  4 // 0.8 m/s or 2.88 km/h or 1.555 knot
#define PMTK_NAV_THRESHOLD_10  5 // 1.0 m/s or 3.60 km/h or 1.943 knot
#define PMTK_NAV_THRESHOLD_15  6 // 1.5 m/s or 5.40 km/h or 2.916 knot
#define PMTK_NAV_THRESHOLD_20  7 // 2.0 m/s or 7.20 km/h or 3.888 knot

// PMTK_CMD_ANTENNA arguments
#define  PCMD_ANTENNA_OFF 0
#define  PCMD_ANTENNA_ON  1
#define  PCMD_ANTENNA_Q   2

// A lot googleing required to build this list of PMTK commands
// some very badly documented, use at your oen risk.
// Not supported commands listed just for completeness
// some day I might add parsers for them too...
// Most of values are valid for GTop module from Adafruit
#define PMTK_TEST				  0 // no arg
#define PMTK_ACK				  1 // acknowledge 
#define PMTK_SYS_MSG			 10 // system startup message
#define PMTK_TXT_MSG			 11 // system text message
#define PCMD_ANTENNA			 33 // arg PCMD_ANTENNA_*
#define PMTK_CMD_HOT_START		101 // no arg
#define PMTK_CMD_WARM_START		102 // no arg
#define PMTK_CMD_COLD_START		103 // no arg
#define PMTK_CMD_FACTORY_RESET	104 // no arg
#define PMTK_CMD_CLEAR_EPO      127 // no arg
#define PMTK_CMD_STANDBY_MODE   161 // arg PMTK_ARG_ON or PMTK_ARG_OFF
#define PMTK_Q_LOCUS_STATUS		183 // no arg, reply PMTKLOG
#define PMTK_LOCUS_ERASE_FLASH	184 // 1 to erase all logger internal flash data
#define PMTK_LOCUS_LOGGER		185 // 0 to stop, 1 to start logger
#define PMTK_LOG_NOW			186 // 1 to make data snapshot
#define PMTK_LOCUS_CONFIG		187 // not supported
#define PMTK_SET_NMEA_UPDATE	220 // use setUpdateRate()
#define PMTK_SET_AL_DEE_CFG     223 // not supported
#define PMTK_CMD_PERIODIC_MODE  225 // not supported 
#define PMTK_SET_NMEA_BAUD_RATE	251 // arg PMTK_BR_*
#define PMTK_SET_OUTPUT_FMT	    253 // arg protocol format, baudrate
#define PMTK_SET_AIC_MODE		286 // arg PMTK_ARG_ON or PMTK_ARG_OFF
#define PMTK_SET_DGPS_MODE		301 // arg PMTK_DGPS_*
#define PMTK_SET_DYN_MODE		302 // arg PMTK_DYN_*
#define PMTK_SET_SBAS_ENABLE    313 // arg PMTK_ARG_ON or PMTK_ARG_OFF
#define PMTK_SET_SBAS_MODE      319 // arg PMTK_DGPS_*
#define PMTK_SET_DATUM			330 // arg 0 to 222
#define PMTK_SET_DATUM_ADVANCE	331 // not supported
#define PMTK_SET_NAV_THRESHOLD  386 // use setNavThreshold()
#define PMTK_Q_FIX_INTERVAL		400 // no arg
#define PMTK_DT_FIX_INTERVAL	500 // reply to 400
#define PMTK_Q_DGPS_MODE		401 // no arg
#define PMTK_DT_DGPS_MODE		501 // reply to 401
#define PMTK_Q_DYN_MODE			402 // no arg
#define PMTK_DT_DYN_MODE		502 // reply to 402
#define PMTK_Q_SBAS_ENABLE      413 // no arg
#define PMTK_DT_SBAS_ENABLE     513 // reply to 413
#define PMTK_Q_NMEA_OUTPUT		414 // no arg
#define PMTK_DT_NMEA_OUTPUT		514 // reply to 414
#define PMTK_Q_SBAS_MODE        419 // no arg
#define PMTK_DT_SBAS_MODE       519 // reply to 419
#define PMTK_Q_DATUM			430 // no arg
#define PMTK_Q_DATUM_ADVANCE	431 // no arg, needs 331 first?
#define PMTK_DT_DATUM			530 // reply to 430 and 431
#define PMTK_Q_NAV_THRESHOLD    447 // no arg
#define PMTK_DT_NAV_THRESHOLD   527 // reply to 447
#define PMTK_Q_USER_OPTION		490 // no arg
#define PMTK_DT_USER_OPTION		590 // reply to 490
#define PMTK_Q_RTCM_BAUD_RATE	602 // no arg
#define PMTK_DT_RTCM_BAUD_RATE	702 // reply to 602
#define PMTK_Q_RELEASE			605 // no arg
#define PMTK_DT_RELEASE			705 // reply to 605
#define PMTK_Q_EPO_INFO			607 // no arg
#define PMTK_DT_EPO_INFO		707 // reply to 607
#define PMTK_Q_LOCUS_DATA		622 // 0 - full, 1 - partial
#define PMTK_CMD_EASY_ENABLE    869 // use setEasyMode

// pseudo replies
#define PMTK_DT_LOCUS_LOG	   1000 // reserved for $PMTKLOG

#define MAX_NMEA_LEN 256

class MtkGps {
public:
	// initializations only, use attach() to select serial port
	MtkGps(int tzone = 0); // time zone offset from UTC in minutes

	// attach to a serial port 
	// on Galileo Serial is USB, Serial1 is RX0/TX1, Serial2 RS232/TTL headers
	TTYUARTClass *attach(TTYUARTClass *ser);
	
	// time zone offset from UTC in minutes
	void setTimeZone(int tzone);
	// get last fix date/time in struct tm and milliseconds to msec
	// date valid only if RMC/ZDA sentences are configured for NMEA output
	void getFixTime(struct tm *fix, uint16_t *msec = NULL);

	// set serial port baud rate
	int begin(uint32_t baud);
	// detect if MTK is attached, returns baud rate detected or PMTK_BR_INVALID
	uint32_t detect(TTYUARTClass *ser);
	// set GPS communication speed
	int setNmeaBaudRate(uint32_t baud);
	// get GPS communication speed
	uint32_t getNmeaBaudRate(void);
	// set NMEA format - text or binary
	int setNmeaFormat(bool text);

	// get RX/TX statistics
	void getPortStat(uint32_t *rxstat, uint32_t *txstat = NULL);
	// str checksum will be calculated while sending
	int sendStr(const char *str);
	// write whatever to GPS module...
	int write(const void *data, uint32_t len);
	// send PMTK_* command with an argument
	char *sendCommand(unsigned cmd, int arg = -1);
	// output configuration, each mask presents which NMEA_SEN_* should be
	// generated every 1, 2, 3, 4 or 5 position fixes
	char *setOutput(uint32_t mask1, uint32_t mask2 = 0, uint32_t mask3 = 0, uint32_t mask4 = 0, uint32_t mask5 = 0);
	// set nmea update rate, 1 to 10 Hz
	int setUpdateRate(uint8_t uHz);
	// set navigation speed threshold, PTMK_NAV_THRESHOLD_*
	int setNavThreshold(uint8_t speed);
	// enable/disable MTK Easy mode, arg PMTK_ARG_ON/PMTK_ARG_OFF
	int setEasyMode(uint8_t arg);
	// get fix quality as a string
	const char *getFixQuality(void);

	// read serial port, returns NULL or nmea sentence read
	const char *read(void);
	// parses nmea sentence
	int parse_nmea(const char *nmea);
	// get PMTP packet type from the string
	int getMtkPType(const char *nmea);
	// check if NMEA_SEN_* type  data is populated
	int isValid(uint32_t sen) { return valid & sen; }
	uint32_t getValid(void) { return valid; }

	// get firmware release information string
	const char *getFWrelease(void);

	// lat and lon in signed degree format DDD.dddddddd
	double latitude, longitude;

	// nmea structures
	gprmc_t  rmc;
	gpgga_t  gga;
	gpgll_t  gll;
	gpvtg_t  vtg;
	gpgsa_t  gsa;
	gpzda_t  zda;
	uint16_t ngsv;
	gpgsv_t  gsv[NMEA_MAX_GSV];
	mtkchn_t chn[MTK_MAX_CHN];

	char cmd[MAX_NMEA_LEN]; // last command sent to GPS module

private:
	// serial port GPS module is attached to
	TTYUARTClass *gpsSerial;
	uint32_t fix_date; // latest fix date/time
	uint32_t fix_time;
	uint32_t fix_msec;
	int      tzone;	// offset to UTC in minutes
	uint32_t valid; // mask of populated nmea sentences
	uint32_t brate;	// serial port baud rate
	uint32_t rx;	// received bytes
	uint32_t tx;	// transmitted bytes
	uint32_t cidx;	// nmea string holder 
	uint16_t igsv;	// GSV parsing index
	const char *release;
	char nmea[MAX_NMEA_LEN]; // last nmea sentence received from GPS module
};

#endif
