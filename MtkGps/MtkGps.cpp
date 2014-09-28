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
#include "MtkGps.h"

/*
	Galileo library for GPS units compatible with MediaTek PMTK protocol,
	tested with the Adafruit Ultimate GPS GTop module using MTK3399 chipset
	http://www.adafruit.com/products/746
	https://learn.adafruit.com/adafruit-ultimate-gps
*/

static const char *bin_off = "\x24\x0E\x00\xFD\x00\x00\x00\x00\x00\x00\xF3\x0D\x0A";

MtkGps::MtkGps(int tzone)
{
	cidx = 0;
	brate = PMTK_BR_INVALID;
	gpsSerial = NULL;
	release = NULL;
	latitude = longitude = 0.0;
	valid = 0;
	rx = tx = 0;
	this->tzone = tzone;

	memset(&rmc, 0, sizeof(rmc));
	memset(&gga, 0, sizeof(gga));
	memset(&gll, 0, sizeof(gll));
	memset(&vtg, 0, sizeof(vtg));
	memset(&gsa, 0, sizeof(gsa));
	memset(&zda, 0, sizeof(zda));
	ngsv = igsv = 0;
	memset(&gsv, 0, sizeof(gsv));
	memset(&chn, 0, sizeof(chn));

	memset(cmd, 0, MAX_NMEA_LEN);
	memset(nmea, 0, MAX_NMEA_LEN);
}

TTYUARTClass *MtkGps::attach(TTYUARTClass *ser)
{
	TTYUARTClass *curPort = gpsSerial;
	gpsSerial = ser;
	return curPort;
}
void MtkGps::setTimeZone(int tzone)
{
	this->tzone = tzone;
}

// 14400 commented out as some strange things happened
// when this speed was used
static uint32_t brates[] = {
	PMTK_BR_4800,  PMTK_BR_9600,  /*PMTK_BR_14400,*/ PMTK_BR_19200,
	PMTK_BR_38400, PMTK_BR_57600, PMTK_BR_115200,
	PMTK_BR_INVALID
};

int MtkGps::begin(uint32_t baud)
{
	for(int i = 0; brates[i] != PMTK_BR_INVALID; i++) {
		if (baud == brates[i]) {
			if (gpsSerial) {
				gpsSerial->begin(baud);
				delay(10);
			}
			brate = baud;
			return 0;
		}
	}
	return -1;
}

uint32_t MtkGps::detect(TTYUARTClass *ser)
{
	if (ser == NULL)
		return PMTK_BR_INVALID;
	
	// store current serial port
	TTYUARTClass *gps = gpsSerial;
	gpsSerial = ser;
	uint32_t rate = brate;

	int br;
	const char *nmea;
	for (br = 0; brates[br] != PMTK_BR_INVALID; br++) {
		ser->begin(brates[br]);
		delay(10);
		long int timeout = millis();
		// send "binary format off" followed by TEST
		// and wait 500 msec for a reply
		write(bin_off, 13);
		delay(20);
		sendCommand(PMTK_TEST);
		while((millis() - timeout) < 500) {
			if ((nmea = read()) != NULL && (parse_nmea(nmea) == 0))
				goto found;
		}
	}

found:
	// restore original serial port
	gpsSerial = gps;
	begin(rate);
	
	return brates[br];
}

int MtkGps::setNmeaBaudRate(uint32_t baud)
{
	for(int i = 0; brates[i] != PMTK_BR_INVALID; i++) {
		if (baud == brates[i]) {
			sendCommand(PMTK_SET_NMEA_BAUD_RATE, baud);
			delay(10);
			return 0;
		}
	}
	return -1;
}

uint32_t MtkGps::getNmeaBaudRate(void)
{
	return brate;
}

int MtkGps::setNmeaFormat(bool text)
{
	if (text) {
		write(bin_off, 13);
		return 0;
	}

	return -1; // binary not supported, ignore
}

int MtkGps::getMtkPType(const char *nmea)
{
	char *ptype;
	char item[NMEA_MAX_ITEM_LEN];

	memset(item, 0, NMEA_MAX_ITEM_LEN);
	nmea_get_item(nmea, item);

	ptype = &item[5];
	if (isdigit(*ptype))
		return atoi(ptype);
	if (strcpy(ptype, "LOG") == 0)
		return PMTK_DT_LOCUS_LOG;

	return -1;
}

int MtkGps::parse_nmea(const char *str) 
{
	if (!nmea_is_valid(str))
		return -1;

	int ret, nmea_type = nmea_get_type(str);

	if (nmea_type == NMEA_SEN_GGA) {
		ret = nmea_parse_gpgga(str, &gga);
		if (ret == 0)
			valid |= NMEA_SEN_GGA;
		return ret;
	}

	if (nmea_type == NMEA_SEN_RMC) {
		ret = nmea_parse_gprmc(str, &rmc);
		if (ret == 0) {
			double frac;
			latitude = rmc.latitude / 100.0;
			frac = modf(latitude, &latitude);
			latitude += frac/0.6;
			if (rmc.flags & NMEA_LAT_SOUTH)
				latitude = -latitude;

			longitude = rmc.longitude / 100.0;
			frac = modf(longitude, &longitude);
			longitude += frac/0.6;
			if (rmc.flags & NMEA_LON_WEST)
				longitude = -longitude;
			valid |= NMEA_SEN_RMC;
		}
		return ret;
	}

	if (nmea_type == NMEA_SEN_GLL) {
		ret = nmea_parse_gpgll(str, &gll);
		if (ret == 0)
			valid |= NMEA_SEN_GLL;
		return ret;
	}

	if (nmea_type == NMEA_SEN_VTG) {
		ret = nmea_parse_gpvtg(str, &vtg);
		if (ret == 0)
			valid |= NMEA_SEN_VTG;
		return ret;
	}

	if (nmea_type == NMEA_SEN_GSA) {
		ret = nmea_parse_gpgsa(str, &gsa);
		if (ret == 0)
			valid |= NMEA_SEN_GSA;
		return ret;
	}

	if (nmea_type == NMEA_SEN_ZDA) {
		ret = nmea_parse_gpzda(str, &zda);
		if (ret == 0)
			valid |= NMEA_SEN_ZDA;
		return ret;
	}

	if (nmea_type == NMEA_SEN_GSV) {
		ret = nmea_parse_gpgsv(str, gsv, &ngsv, &igsv);
		if (ret == 0)
			valid |= NMEA_SEN_GSV;
		return ret;
	}

	if (nmea_type == NMEA_SEN_MCHN) {
		ret = nmea_parse_mtkchn(str, chn);
		if (ret == 0)
			valid |= NMEA_SEN_MCHN;
		return ret;
	}

	// firmware release info
	if (nmea_type == NMEA_SEN_MTK) {
		int type = getMtkPType(nmea);
		if (type == PMTK_DT_RELEASE) {
			if (release != NULL)
				free((void *)release);
			char *end = strchr(nmea, '*');
			if (end)
				*end = '\0';
			release = strdup(nmea + 9);
			return 0;
		}
	}
	return 0;
}

const char *MtkGps::read(void)
{
	if (gpsSerial == NULL || !gpsSerial->available())
		return NULL;

	char c = gpsSerial->read();
	rx++;
	// reset line if '$' received from MTK3339
	if (c == '$')
		cidx = 0;

	// EOL received, return full nmea line ready to be parsed
	if (c == '\n') {
		nmea[cidx] = '\0';
		cidx = 0;
		return nmea;
	}

	if (cidx < (MAX_NMEA_LEN - 1))
		nmea[cidx++] = c;

	return NULL;
}

// calculate CRC, append to the string and send to serial port
int MtkGps::sendStr(char *str)
{
	if (*str != '$')
		return -1;
	char *ptr = str + 1;

	uint8_t crc = 0;
	while(*ptr) {
		crc ^= *ptr++;
		tx++;
	}
	sprintf(ptr, "*%02X", crc);
	tx += 5; // '*XX<CR><LF>
	if (gpsSerial) {
		gpsSerial->println(str);
		delay(10);
	}

	strcpy(cmd, str);
	return 0;
}

int MtkGps::write(const void *data, uint32_t len)
{
	if (gpsSerial)
		return gpsSerial->write((const uint8_t *)data, len);
	tx += len;
	return 0;
}

void MtkGps::getPortStat(uint32_t *rxstat, uint32_t *txstat)
{
	if (rxstat)
		*rxstat = rx;
	if (txstat)
		*txstat = tx;
}

// arg >= 0, not used if  < 0
char *MtkGps::sendCommand(unsigned cmd, int arg)
{
	static char str[64];

	if (cmd > 999)
		return "Invalid command\n";
	if (arg >= 0)
		sprintf(str, "$PMTK%03d,%d", cmd, arg);
	else
		sprintf(str, "$PMTK%03d", cmd);

	sendStr(str);
	return str;
}

#define MTK3339_NMEA 19

static const uint32_t output_mask[MTK3339_NMEA] = {
	NMEA_SEN_GLL,
	NMEA_SEN_RMC,
	NMEA_SEN_VTG,
	NMEA_SEN_GGA,
	NMEA_SEN_GSA,
	NMEA_SEN_GSV,

	NMEA_INVALID,
	NMEA_INVALID,
	NMEA_INVALID,
	NMEA_INVALID,
	NMEA_INVALID,
	NMEA_INVALID,
	NMEA_INVALID,
	NMEA_INVALID,
	NMEA_INVALID,
	NMEA_INVALID,
	NMEA_INVALID,

	NMEA_SEN_ZDA,
	NMEA_SEN_MCHN
};

char *MtkGps::setOutput(uint32_t mask1, uint32_t mask2, uint32_t mask3, uint32_t mask4, uint32_t mask5)
{
	static char str[64];
	uint32_t mask[5];

	mask[0] = mask1;
	mask[1] = mask2;
	mask[2] = mask3;
	mask[3] = mask4;
	mask[4] = mask5;

	strcpy(str, "$PMTK314");
	char *out = strchr(str, '\0');

	for(int i = 0; i < MTK3339_NMEA; i++) {
		int freq = 0;
		for(int m = 0; m < 5; m++) {
			if (mask[m] & output_mask[i])
				freq = m + 1;
		}
		sprintf(out, ",%d", freq);
		out+= 2;
	}

	sendStr(str);
	return str;
}

int MtkGps::setUpdateRate(uint8_t uHz)
{
	if (uHz > 10)
		return -1;
	uint32_t msec = 1000/uHz;
	sendCommand(PMTK_SET_NMEA_UPDATE, msec);
	return 0;
}

static double nav_threshold[PMTK_NAV_THRESHOLD_20 + 1] = {
	0.0, 0.2, 0.4, 0.6, 0.8, 1.0, 1.5, 2.0
};

int MtkGps::setNavThreshold(uint8_t speed)
{
	char str[64];

	if (speed > PMTK_NAV_THRESHOLD_20)
		speed = PMTK_NAV_THRESHOLD_20;

	sprintf(str, "$PMTK%d,%.1f", PMTK_SET_NAV_THRESHOLD, nav_threshold[speed]);
	sendStr(str);

	return 0;
}

static const char *q_name[] = {
	"invalid", "GPS fix (SPS)", "DGPS fix", "PPS fix", "Real Time Kinematic",
	"Float RTK", "estimated (dead reckoning)", "Manual input mode", "Simulation mode",
	"unknown"
};

const char *MtkGps::getFixQuality(void)
{
	uint32_t idx = gga.quality;
	if (idx > sizeof(q_name)/sizeof(q_name[0]))
		idx = sizeof(q_name)/sizeof(q_name[0]);

	return q_name[idx];
}

int MtkGps::setEasyMode(uint8_t arg)
{
	char str[64];

	sprintf(str, "$PMTK%d,1,%d", PMTK_CMD_EASY_ENABLE, arg ? 1 : 0);
	sendStr(str);

	return 0;
}

const char *MtkGps::getFWrelease(void)
{
	if (release == NULL)
		return "<unknown>";
	return release;
}

void MtkGps::getFixTime(struct tm *fix, uint16_t *millis)
{
	uint32_t time = 0;
	uint32_t date = 0;
	uint16_t msec = 0;

	if (valid & NMEA_SEN_RMC) {
		date = rmc.date;
		time = rmc.time;
		msec = rmc.millisec;
	}
	else if (valid & NMEA_SEN_GGA) {
		time = gga.time;
		msec = gga.millisec;
	}
	else if (valid & NMEA_SEN_GLL) {
		time = gga.time;
		msec = gga.millisec;
	}
	else if (valid & NMEA_SEN_ZDA) {
		date = zda.date;
		time = zda.time;
		msec = zda.millisec;
	}

	fix->tm_hour = time / 10000;
	fix->tm_min  = (time % 10000) / 100;
	fix->tm_min  += tzone;
	fix->tm_sec  = (time % 100);
	fix->tm_mday = date / 10000;
	fix->tm_mon  = (date % 10000) / 100;
	fix->tm_year = (date % 100);

	if (tzone) {
		time_t tt = mktime(fix);
		memcpy(fix, localtime(&tt), sizeof(struct tm));
	}

	if (millis)
		*millis = msec;
}
