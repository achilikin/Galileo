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

/* 
	Bridge RX0/TX1 serial ports to USB serial for connecting to skyplot software
	like MediaTek MiniGPS of Locosys GPSFox 
	or for uploading EPO data
	or (at your own risk!) for firmware upgrade
*/

MtkGps gps;

// default baud rate for skyplot is 38400
// for firmware update and EPO use PMTK_BR_9600
#define BRIDGE_BAUD PMTK_BR_38400

// set to 1 to monitor bridge data on RS232/TTL serial port
// set it to 0 for firmware update or for EPO uploading 
// to avoid a lot binary dumps on monitoring terminal
#define MONITOR_BRIDGE 1

#if MONITOR_BRIDGE

#define MAX_MONITOR_LEN 256

SerialTerminal term(MAX_MONITOR_LEN+2);

// string from GPS
int  igps = 0;
char str_gps[MAX_MONITOR_LEN+2];

// string from external SW
int  isw = 0;
char str_sw[MAX_MONITOR_LEN+2];
#endif

void setup()
{
	Serial.begin(BRIDGE_BAUD); // Galileo USB serial, connected to external SW
#if MONITOR_BRIDGE
	term.attach(&Serial2);
	term.begin(PMTK_BR_115200); // RS232/TTL headers for monitoring
#endif
	// detect MTK baud rate
	uint32_t gpsbr = gps.detect(&Serial1);
	if (gpsbr == PMTK_BR_INVALID)
		gpsbr = PMTK_BR_9600;

	Serial1.begin(gpsbr);
	gps.attach(&Serial1);
	// set default bridge baud rate
	if (gpsbr != BRIDGE_BAUD) {
		gps.setNmeaBaudRate(BRIDGE_BAUD);
		gps.begin(BRIDGE_BAUD);
	}
	// set default skyplot settings
	gps.setUpdateRate(1);
	gps.setEasyMode(PMTK_ARG_ON);
	gps.setOutput(NMEA_SEN_RMC | NMEA_SEN_GSA | NMEA_SEN_VTG | NMEA_SEN_GGA, 0, 0, NMEA_SEN_GSV);
	gps.sendCommand(PMTK_SET_DGPS_MODE, PMTK_DGPS_WAAS);
}

int binary = 0;

void loop()
{
	char c;

	// copy data from gps to the second serial port
	while(Serial1.available()) {
		c = Serial1.read();
		Serial.print(c);
#if MONITOR_BRIDGE
		if (c == '$')
			igps = 0;
		// check for EOL
		if (c == 0x0A) {
			if (binary && (igps > 0) && str_gps[igps-1] != 0x0D)
				goto insert_gps;
			str_gps[igps++] = c;
			str_gps[igps] = '\0';

			// turn off binary if PMTK text received
			if (binary && (nmea_get_type(str_gps) == NMEA_SEN_MTK))
				binary = 0;
			// clone output to monitoring port
			if (!binary)
				term.print(">%s", str_gps); // clone output to monitoring port
			else
				term.dump(">", str_gps, igps);
			igps = 0;
			continue;
		}
insert_gps:
		if (c != 0x0D && !isprint(c) && !binary)
			c = '.';
		if (igps < MAX_MONITOR_LEN)
			str_gps[igps++] = c; {
			continue;
		}
		// clone output to monitoring port
		if (!binary)
			term.print(">%s\n", str_gps); // clone output to monitoring port
		else
			term.dump(">", str_gps, igps);
		igps = 0;
#endif
	}
	// copy data from the second serial port to gps
	while(Serial.available()) {
		c = Serial.read();
		Serial1.print(c);
#if MONITOR_BRIDGE
		if (c == '$')
			isw = 0;
		// check for EOL
		if (c == 0x0A) {
			if (binary && (isw > 0) && (str_sw[isw-1] != 0x0D))
				goto insert_sw;
			str_sw[isw++] = c;
			str_sw[isw] = '\0';
			if (!binary) {
				if (isw > 1) // some GPS utilities add extra 0x0A at the end, skip it
					term.print("<%s", str_sw); // clone output to USB serial for monitoring
			}
			else
				term.dump("<", str_sw, isw);
			if (strncmp(str_sw, "$PMTK253,1,", 11) == 0)
				binary = 1;
			isw = 0;
			continue;
		}
insert_sw:
		if (c != 0x0D && !isprint(c) && !binary)
			c = '.';
		if (isw < MAX_MONITOR_LEN)
			str_sw[isw++] = c;
		else {
			if (!binary)
				term.print("<%s\n", str_sw); // clone output to USB serial for monitoring
			else
				term.dump("<", str_sw, isw);
			isw = 0;
		}
#endif
	}
}
