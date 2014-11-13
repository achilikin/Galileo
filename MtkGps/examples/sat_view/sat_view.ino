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
#include <Arduino.h>

#include <MtkGps.h>
#include <SerialTerminal.h>
#include <ticker.h>

#define GPS_BAUD PMTK_BR_9600
#define MAX_TERM_STR_LEN 256

#define UTC_OFFSET 60

MtkGps gps;

// set to 1 to close NMEA output to USB serial
#define CLONE_NMEA 1

#if CLONE_NMEA
// USB port for NMEA output
SerialTerminal usb(MAX_TERM_STR_LEN);
#endif

// our serial terminal
SerialTerminal term(MAX_TERM_STR_LEN);

// some vt100 codes 
const char cdwn[]  = { 27, '[', 'J', '\0' }; // clear down
const char ceol[]  = { 27, '[', '0', 'K', '\0' }; // clear to EOL
const char gotop[] = { 27, '[', '1', ';', '1', 'H','\0' }; // top left
const char civis[] = { 27, '[', '?', '2', '5', 'l','\0' }; // invisible cursor

// 1 second timer to print satellites data
int ticker(void *data);
ticker_t timer(1000, ticker);

void setup()
{
#if CLONE_NMEA
	usb.attach(&Serial);
	usb.begin(PMTK_BR_115200); // clone nmea to USB
#endif

	term.attach(&Serial2);
	term.begin(PMTK_BR_115200);   // RS232/TTL headers for monitoring

	term.puts(gotop); // go to 0,0
	term.puts(cdwn);  // clear screen
	// print initial header
	ticker(gps.gsv);

	// detect MTK baud rate
	uint32_t gpsbr = gps.detect(&Serial1);
	if (gpsbr == PMTK_BR_INVALID)
		gpsbr = PMTK_BR_9600;

	Serial1.begin(gpsbr);
	gps.attach(&Serial1);

	// set default bridge baud rate to 9600 as we are going to use NMEA_SEN_MCHN
	gps.setNmeaBaudRate(GPS_BAUD);
	gps.begin(GPS_BAUD);

	// set local time to UTC offset
	gps.setTimeZone(UTC_OFFSET);

	gps.setUpdateRate(2);
	gps.setEasyMode(PMTK_ARG_ON);
	gps.sendCommand(PMTK_SET_SBAS_ENABLE, PMTK_ARG_ON);
	gps.sendCommand(PMTK_SET_DGPS_MODE, PMTK_DGPS_WAAS);
	// we are interested in satellites and time/date only
	// NMEA_SEN_ZDA  - time of the last fix, 
	// NMEA_SEN_GSV  - snr, elevation, azimuth
	// NMEA_SEN_GSA  - fix type and satellites in use
	// NMEA_SEN_MCHN - idle, search, track info
	gps.setOutput(NMEA_SEN_ZDA, NMEA_SEN_GSV | NMEA_SEN_GSA, 0, 0, NMEA_SEN_MCHN);
}

void loop()
{
	const char *nmea;
	unsigned long tstamp, pstamp;

	pstamp = millis();

	while(1) {
		// read milliseconds for timer(s) processing
		tstamp = millis();
		timer.tick(tstamp, gps.gsv);

		// check gps module for a new nmea sentences
		if ((nmea = gps.read()) != NULL) {
			gps.parse_nmea(nmea);
#if CLONE_NMEA
			usb.print("%4u %u %s\n", tstamp - pstamp, tstamp, nmea);
#endif
			pstamp = tstamp;
		}
	}
}

// get channel info for the satellite
static char get_sat_tracking(int prn)
{
	static char track[3] = {'I', 'S', 'T' }; // idle, search, tracking

	for(int i = 0; i < MTK_MAX_CHN; i++) {
		if (gps.chn[i].prn == prn)
			return track[gps.chn[i].track];
	}
	return '*'; // unknown
}

// get used/unused state for the satellite
static char get_sat_state(int prn)
{
	for(int i = 0; i < NMEA_GSA_MAX_PRN; i++) {
		if (gps.gsa.prn[i] == prn)
			return 'U'; // used for solution
	}
	return '\0';
}

int ticker(void *data)
{
	struct tm fixtm;
	gpgsv_t *gsv = (gpgsv_t *)data;
	const char *fixtype[3] = { " ", "2D", "3D" };

	term.puts(gotop); // go to 0,0 position
	term.puts(civis); // hide cursor
	term.print("  # | SID | ELE | AZIM | CNR ");
	// print last fix time if available
	if (gps.isValid(NMEA_SEN_ZDA)) {
		uint8_t fix = 0;
		// get type of fix if available
		if (gps.isValid(NMEA_SEN_GSA)) {
			if ((gps.gsa.fix >= NMEA_GSA_NO_FIX) && (gps.gsa.fix <= NMEA_GSA_3D_FIX))
				fix = gps.gsa.fix - NMEA_GSA_NO_FIX;
		}
		gps.getFixTime(&fixtm);
		term.print(" Fix %s: %02d:%02d:%02d %d/%02d/%02d",
			fixtype[fix],
			fixtm.tm_hour, fixtm.tm_min, fixtm.tm_sec,
			fixtm.tm_year+2000, fixtm.tm_mon, fixtm.tm_mday);
	}
	else
		term.print("searching...");
	// clean to the end on the line
	term.puts(ceol);
	term.puts("\n");
	
	// print visible satellites info and count statistics
	int nidle = 0, ntrack = 0, nsearch = 0, nused = 0;
	for(int sat = 1, i = 0; i < gps.ngsv; i++) {
		if (gsv[i].prn != 0) {
			term.print(" %2d |  %2d |  %2d |  %3d |", sat++, gsv[i].prn, gsv[i].elevation, gsv[i].azimuth);
			for(int n = 0; n < gsv[i].snr; n++)
				term.putch('=');
			if (gsv[i].snr)
				term.print(" %2d", gsv[i].snr);
			else
				term.putch('-');
			char track = get_sat_tracking(gsv[i].prn);
			char state = get_sat_state(gsv[i].prn);
			if (state)
				nused ++;
			if (track == 'T')
				ntrack++;
			else if (track == 'S')
				nsearch++;
			else if (track == 'I')
				nidle++;

			term.print(" %c%c", track, state);
			term.puts(ceol);
			term.puts("\n");
		}
	}
	uint32_t rx;
	gps.getPortStat(&rx);
	term.print("  rx: %-10u        ", rx);
	// print satellites statistics
	if (gps.isValid(NMEA_SEN_ZDA)) {
		if (nused)
			term.print("Used: %d ", nused);
		if (ntrack)
			term.print("Tracking: %d ", ntrack);
		if (nidle)
			term.print("Idle: %d ", nidle);
		if (nsearch)
			term.print("Searching: %d", nsearch);
	}
	term.puts(cdwn);
	return 0;
}
