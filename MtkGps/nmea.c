/*	BSD License
	Copyright (c) 2014 Andrey Chilikin https://github.com/achilikin

	Redistribution and use in source and binary forms, with or without 
	modification, are permitted provided that the following conditions
	are met:

	1. Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer
	in the documentation and/or other materials provided with the distribution.
	3. Neither the name of the copyright holder nor the names of its
	contributors may be used to endorse or promote products derived
	from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.	
*/
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "nmea.h"

int nmea_is_valid(const char *nmea)
{
	if (*nmea != '$')
		return 0;

	uint8_t crc;
	const char *pcrc = nmea + 1;
	for(crc = 0; *pcrc && *pcrc != '*'; pcrc++)
		crc ^= *pcrc;

	if (*pcrc != '*')
		return 0;
	
	if (crc != strtoul(pcrc+1, NULL, 16))
		return 0;

	return NMEA_VALID;
}

int nmea_get_type(const char *nmea)
{
	if (strncmp(nmea, "$GPRMC,", 7) == 0)
		return NMEA_SEN_RMC;
	if (strncmp(nmea, "$GPGGA,", 7) == 0)
		return NMEA_SEN_GGA;
	if (strncmp(nmea, "$GPGLL,", 7) == 0)
		return NMEA_SEN_GLL;
	if (strncmp(nmea, "$GPVTG,", 7) == 0)
		return NMEA_SEN_VTG;
	if (strncmp(nmea, "$GPGSA,", 7) == 0)
		return NMEA_SEN_GSA;
	if (strncmp(nmea, "$GPGSV,", 7) == 0)
		return NMEA_SEN_GSV;
	if (strncmp(nmea, "$GPZDA,", 7) == 0)
		return NMEA_SEN_ZDA;
	if (strncmp(nmea, "$PMTKCHN,", 9) == 0)
		return NMEA_SEN_MCHN;
	if (strncmp(nmea, "$PMTK", 5) == 0)
		return NMEA_SEN_MTK;
	if (strncmp(nmea, "$PGTOP,", 7) == 0)
		return NMEA_SEN_PGTOP;
	if (strncmp(nmea, "$PGACK,", 7) == 0)
		return NMEA_SEN_PGACK;
	
	return NMEA_INVALID;
}

// extracts an item from the nmea string
const char *nmea_get_item(const char *nmea, char *item)
{
	while(*nmea != ',' && *nmea != '*')
		*item++ = *nmea++;
	*item = '\0';
	// skip trailing ','
	if (*nmea == ',')
		return nmea + 1;
	// stay on '*'
	return nmea;
}

/* 
	NMEA sentences parsers do not validate nmea string
	so any validation MUST be done before calling nmea_parse_* functions
*/

int nmea_parse_gpgll(const char *nmea, gpgll_t *rec)
{
	gpgll_t gll;
	const char *next;
	char item[NMEA_MAX_ITEM_LEN];
	
	memset(&gll, 0, sizeof(gll));
	next = nmea_get_item(nmea, item); // skip NMEA header

	next = nmea_get_item(next, item);
	gll.latitude = atof(item);
	next = nmea_get_item(next, item);
	if (*item == 'S')
		gll.flags |= NMEA_LAT_SOUTH;
	else if (*item != 'N')
		return -1;

	next = nmea_get_item(next, item);
	gll.longitude = atof(item);
	next = nmea_get_item(next, item);
	if (*item == 'W')
		gll.flags |= NMEA_LON_WEST;
	else if (*item != 'E')
		return -1;
	next = nmea_get_item(next, item);
	if (*item) {
		char *msec;
		gll.time = strtoul(item, &msec, 10);
		if (*msec == '.')
			gll.millisec = atoi(msec + 1);
	}

	memcpy(rec, &gll, sizeof(gll));
	return 0;
}

int nmea_parse_gpgga(const char *nmea, gpgga_t *rec)
{
	gpgga_t gga;
	const char *next;
	char item[NMEA_MAX_ITEM_LEN];

	memset(&gga, 0, sizeof(gga));
	next = nmea_get_item(nmea, item);

	next = nmea_get_item(next, item);
	if (*item) {
		char *msec;
		gga.time = strtoul(item, &msec, 10);
		if (*msec == '.')
			gga.millisec = atoi(msec + 1);
	}
	
	next = nmea_get_item(next, item);
	gga.latitude = atof(item);
	next = nmea_get_item(next, item);
	if (*item == 'S')
		gga.flags |= NMEA_LAT_SOUTH;
	else if (*item != 'N')
		return -1;

	next = nmea_get_item(next, item);
	gga.longitude = atof(item);
	next = nmea_get_item(next, item);
	if (*item == 'W')
		gga.flags |= NMEA_LON_WEST;
	else if (*item != 'E')
		return -1;

	next = nmea_get_item(next, item);
	gga.quality = atoi(item);
	
	next = nmea_get_item(next, item);
	gga.nsat = atoi(item);
	
	next = nmea_get_item(next, item);
	gga.hdop = atof(item);
	
	next = nmea_get_item(next, item);
	gga.altitude = atof(item);
	
	next = nmea_get_item(next, item);
	if (*item != 'M')
		return -1;
	
	next = nmea_get_item(next, item);
	gga.separation = atof(item);
	
	next = nmea_get_item(next, item);
	if (*item != 'M')
		return -1;
	
	next = nmea_get_item(next, item);
	gga.age = atof(item);
	
	next = nmea_get_item(next, item);
	gga.station = atoi(item);
	
	memcpy(rec, &gga, sizeof(gga));
	return 0;
}

int nmea_parse_gprmc(const char *nmea, gprmc_t *rec)
{
	gprmc_t rmc;
	const char *next;
	char item[NMEA_MAX_ITEM_LEN];

	memset(&rmc, 0, sizeof(rmc));
	next = nmea_get_item(nmea, item);

	next = nmea_get_item(next, item);
	if (*item) {
		char *msec;
		rmc.time = strtoul(item, &msec, 10);
		if (*msec == '.')
			rmc.millisec = atoi(msec);
	}
	
	next = nmea_get_item(next, item);
	if (*item == 'A')
		rmc.flags |= NMEA_VALID;
	else if (*item != 'V')
		return -1;
	
	next = nmea_get_item(next, item);
	rmc.latitude = atof(item);

	next = nmea_get_item(next, item);
	if (*item == 'S')
		rmc.flags |= NMEA_LAT_SOUTH;
	else if (*item != 'N')
		return -1;

	next = nmea_get_item(next, item);
	rmc.longitude = atof(item);

	next = nmea_get_item(next, item);
	if (*item == 'W')
		rmc.flags |= NMEA_LON_WEST;
	else if (*item != 'E')
		return -1;

	next = nmea_get_item(next, item);
	rmc.speed = atof(item);

	next = nmea_get_item(next, item);
	rmc.course = atof(item);

	next = nmea_get_item(next, item);
	rmc.date = atoi(item);

	next = nmea_get_item(next, item);
	rmc.variation = atoi(item);
	next = nmea_get_item(next, item);
	if (*item == 'W')
		rmc.flags |= NMEA_VAR_WEST;

	memcpy(rec, &rmc, sizeof(rmc));
	return 0;
}

int nmea_parse_gpvtg(const char *nmea, gpvtg_t *rec)
{
	gpvtg_t vtg;
	const char *next;
	char item[NMEA_MAX_ITEM_LEN];

	memset(&vtg, 0, sizeof(vtg));
	next = nmea_get_item(nmea, item);

	next = nmea_get_item(next, item);
	if (*item) {
		vtg.ttrack = atof(item);
		vtg.flags |= NMEA_VTG_TRACK;
	}

	next = nmea_get_item(next, item);
	if (*item != 'T')
		return -1;

	next = nmea_get_item(next, item);
	if (*item) {
		vtg.mtrack = atof(item);
		vtg.flags |= NMEA_VTG_MRACK;
	}

	next = nmea_get_item(next, item);
	if (*item != 'M')
		return -1;

	next = nmea_get_item(next, item);
	if (*item) {
		vtg.nspeed = atof(item);
		vtg.flags |= NMEA_VTG_NSPEED;
	}

	next = nmea_get_item(next, item);
	if (*item != 'N')
		return -1;
	
	next = nmea_get_item(next, item);
	if (*item) {
		vtg.kspeed = atof(item);
		vtg.flags |= NMEA_VTG_KSPEED;
	}

	next = nmea_get_item(next, item);
	if (*item != 'K')
		return -1;

	memcpy(rec, &vtg, sizeof(vtg));
	return 0;
}

int nmea_parse_gpgsa(const char *nmea, gpgsa_t *rec)
{
	int i;
	gpgsa_t gsa;
	const char *next;
	char item[NMEA_MAX_ITEM_LEN];

	memset(&gsa, 0, sizeof(gsa));
	next = nmea_get_item(nmea, item);

	next = nmea_get_item(next, item);
	gsa.select = *item;

	next = nmea_get_item(next, item);
	gsa.fix = *item;

	for(i = 0; i < NMEA_GSA_MAX_PRN; i++) {
		next = nmea_get_item(next, item);
		if (*item) {
			uint32_t prn = atoi(item);
			if (prn > 0xFF)
				return -1;
			gsa.prn[i] = prn;
		}
	}
	
	next = nmea_get_item(next, item);	
	gsa.pdop = atof(item);

	next = nmea_get_item(next, item);
	gsa.hdop = atof(item);

	next = nmea_get_item(next, item);
	gsa.vdop = atof(item);
	
	memcpy(rec, &gsa, sizeof(gsa));
	return 0;
}

int nmea_parse_gpzda(const char *nmea, gpzda_t *rec)
{
	gpzda_t zda;
	const char *next;
	char item[NMEA_MAX_ITEM_LEN];

	memset(&zda, 0, sizeof(zda));
	next = nmea_get_item(nmea, item);

	next = nmea_get_item(next, item);
	if (*item) {
		char *msec;
		zda.time = strtoul(item, &msec, 10);
		if (*msec != '.')
			return -1;
		zda.millisec = atoi(msec + 1);
	}

	next = nmea_get_item(next, item);
	zda.date = atoi(item)*10000;
	next = nmea_get_item(next, item);
	zda.date += atoi(item) * 100;
	next = nmea_get_item(next, item);
	zda.date += atoi(item) % 100;
	nmea_get_item(next, item);
	if (*item) {
		zda.ltz = atoi(item);
		zda.flags |= NMEA_ZDA_LTZ_VALID;
	}

	memcpy(rec, &zda, sizeof(zda));
	return 0;
}

int nmea_parse_gpgsv(const char *nmea, gpgsv_t *rec, uint16_t *nrec, uint16_t *ridx)
{
	int i;
	gpgsv_t gsv;
	const char *next;
	char item[NMEA_MAX_ITEM_LEN];

	memset(&gsv, 0, sizeof(gsv));
	next = nmea_get_item(nmea, item); // skip $GPGSV
	next = nmea_get_item(next, item); // number of messages
	int nmsg = atoi(item);
	next = nmea_get_item(next, item); // skip message index
	int imsg = atoi(item);
	next = nmea_get_item(next, item); // number of SVNs
	int nsvn = atoi(item);
	if (imsg == 1) { // first GSV message, reset SVN index
		*nrec = nsvn;
		*ridx = 0;
	}
	uint16_t idx = *ridx;
	// parse four satellites info
	for(i = 0; i < 4; i++) {
		next = nmea_get_item(next, item);
		rec[idx].prn = atoi(item);
		next = nmea_get_item(next, item);
		rec[idx].elevation = atoi(item);
		next = nmea_get_item(next, item);
		rec[idx].azimuth = atoi(item);
		next = nmea_get_item(next, item);
		rec[idx].snr = atoi(item);
		idx++;
	}
	*ridx = idx;

	return 0;
}

int nmea_parse_mtkchn(const char *nmea, mtkchn_t *rec)
{
	int i;
	const char *next;
	char item[NMEA_MAX_ITEM_LEN];

	next = nmea_get_item(nmea, item); // skip $PMTKCHN
	memset(rec, 0, sizeof(mtkchn_t)*MTK_MAX_CHN);

	// parse MTK_MAX_CHN satellites info
	for(i = 0; i < MTK_MAX_CHN; i++) {
		next = nmea_get_item(next, item);
		uint32_t chn = atoi(item);
		if (chn) {
			rec[i].prn   = chn / 1000;
			rec[i].snr   = (chn % 1000) / 10;
			rec[i].track = chn % 10;
		}
	}

	return 0;
}
