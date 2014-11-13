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
#ifndef __NMEA_DATA_H__
#define __NMEA_DATA_H__

/*
	Basic NMEA sentences parser for MTK based GPS units
	Some good sources of NMEA sentences format:
	http://aprs.gids.nl/nmea/
	http://www.gpsinformation.org/dale/nmea.htm
*/

#include <stdint.h>

/* NMEA sentences supported by MTK3339 */
#define NMEA_INVALID	0
#define NMEA_SEN_GLL	0x0001 // GPGLL interval - Geographic Position - Latitude longitude
#define NMEA_SEN_RMC	0x0002 // GPRMC interval - Recommended Minimum Specific GNSS Sentence
#define NMEA_SEN_VTG	0x0004 // GPVTG interval - Course over Ground and Ground Speed
#define NMEA_SEN_GGA	0x0008 // GPGGA interval - GPS Fix Data
#define NMEA_SEN_GSA	0x0010 // GPGSA interval - GNSS DOPS and Active Satellites
#define NMEA_SEN_GSV	0x0020 // GPGSV interval - GNSS Satellites in View 
#define NMEA_SEN_ZDA	0x0040 // GPZDA interval - Time & Date
/* 
	MTK specific sentence PMTKCHN: GPS channel status
	use with care, if baud rate is not 9600 can reset module to 9600
	at least I saw it quite a few times
*/
#define NMEA_SEN_MCHN	0x0080

/* MTK reply sentence */
#define NMEA_SEN_MTK	0x8000 // PMTK sentence
#define NMEA_SEN_PGACK	0x4000 // PGACK sentence
#define NMEA_SEN_PGTOP	0x2000 // PGTOP sentence

#define NMEA_VALID		0x0001
#define NMEA_LAT_SOUTH	0x0002
#define NMEA_LON_WEST	0x0004
#define NMEA_VAR_WEST	0x0008

/*	$GPGLL
	Geographic Position, Latitude / Longitude and time.
*/
typedef struct gpgll_s
{
	uint16_t flags;
	uint16_t millisec;
	uint32_t time;
	double   latitude;
	double   longitude;
} gpgll_t;

/*	$GPGGA
	Global Positioning System Fix Data. Time,
	position and fix related data for a GPS receiver.
*/
typedef struct gpgga_s
{
	uint16_t flags;
	uint16_t millisec;
	uint32_t time;
	double   latitude;
	double   longitude;
	uint8_t  quality;
	uint8_t  nsat;
	double   hdop;
	double   altitude;
	double   separation;
	double   age;
	uint16_t station;
} gpgga_t;

/*	$GPRMC
	Recommended minimum specific GPS/Transit data
*/
typedef struct gprmc_s
{
	uint16_t flags;
	uint16_t millisec;
	uint32_t time;
	uint32_t date;
	double   latitude;
	double   longitude;
	double   speed;
	double   course;
	double   variation;
} gprmc_t;

/*	$GPVTG
	Track Made Good and Ground Speed.
*/
#define NMEA_VTG_TRACK	0x0001
#define NMEA_VTG_MRACK	0x0002
#define NMEA_VTG_NSPEED	0x0004
#define NMEA_VTG_KSPEED	0x0008

typedef struct gpvtg_s
{
	uint32_t flags;
	double ttrack;
	double mtrack;
	double nspeed;
	double kspeed;
} gpvtg_t;

/*	$GPGSA
	GPS DOP and active satellites status
*/
#define NMEA_GSA_AUTO	 'A'
#define NMEA_GSA_MANUAL  'M'
#define NMEA_GSA_NO_FIX  '1'
#define NMEA_GSA_2D_FIX  '2'
#define NMEA_GSA_3D_FIX  '3'
#define NMEA_GSA_MAX_PRN 12 

typedef struct gpgsa_s
{
	uint8_t select;   // 'A'uto or 'M'anual
	uint8_t fix;      // 1=no fix, 2=2D fix, 3=3D fix
	uint8_t prn[14];  // PRNs of satellites used for fix, 2 extra to align floats
	double  pdop;     // dilution of precision, DOP
	double  hdop;     // horizontal DOP
	double  vdop;     // vertical DOP
} gpgsa_t;

/*	$GPZDA
	UTC, day, month, year, and local time zone.
*/
#define NMEA_ZDA_LTZ_VALID 0x0001

typedef struct gpzda_s
{
	uint32_t flags;
	uint32_t time;
	uint16_t millisec;
	uint32_t date;
	uint16_t ltz;
} gpzda_t;

/*	$GPGSV
	GSV - Satellites in View, 4 satellites per sentence
*/
#define NMEA_MAX_GSV 32

typedef struct gpgsv_s
{
	uint16_t prn;
	uint16_t snr;       // 00-99 dB, 0 if not tracking
	int16_t  elevation; // 0-90, elevation in degrees
	int16_t  azimuth;   // 0-359 degrees, azimuth from true north
} gpgsv_t;

/*	$PMTKCHN
	MTK3339 specific: compressed SVN channel info, 32 'ppnnt' values
	pp-PRN, nn-SNR, t-0 idle, 1 searching, 2 tracking
*/ 
#define MTK_MAX_CHN 32

typedef struct mtkchn_s
{
	uint8_t  prn;
	uint8_t  snr;
	uint16_t track;
} mtkchn_t;

#ifdef __cplusplus
extern "C" {
#endif

/* calculates crc to check nmea string is valid */
int nmea_is_valid(const char *nmea);
/* returns NMEA_SEN_* types */
int nmea_get_type(const char *nmea);

// extracts an item from the nmea string
#define NMEA_MAX_ITEM_LEN 64
const char *nmea_get_item(const char *nmea, char *item);

/* 
	NMEA sentences parsers do not validate nmea string
	so any validation MUST be done before calling nmea_parse_* functions
*/

/* parses GLL sentence and fills gpgll_t structure */
int nmea_parse_gpgll(const char *nmea, gpgll_t *rec);
/* parses GGA sentence and fills gpgga_t structure */
int nmea_parse_gpgga(const char *nmea, gpgga_t *rec);
/* parses RMC sentence and fills gprmc_t structure */
int nmea_parse_gprmc(const char *nmea, gprmc_t *rec);
/* parses VTG sentence and fills gpvtg_t structure */
int nmea_parse_gpvtg(const char *nmea, gpvtg_t *rec);
/* parses GSA sentence and fills gpgsa_t structure */
int nmea_parse_gpgsa(const char *nmea, gpgsa_t *rec);
/* parses ZDA sentence and fills gpzda_t structure */
int nmea_parse_gpzda(const char *nmea, gpzda_t *rec);
/* parses GSV sentence and fills array[NMEA_MAX_GSV] of gpgsv_t structures */
int nmea_parse_gpgsv(const char *nmea, gpgsv_t *rec, uint16_t *nrec, uint16_t *ridx);
/* parses PMTKCHN sentence and fills array[MTK_MAX_CHN] of mtkchn_t structures */
int nmea_parse_mtkchn(const char *nmea, mtkchn_t *rec);

#ifdef __cplusplus
}
#endif

#endif
