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
#include <Arduino.h>

#include <MtkGps.h>
#include <SerialTerminal.h>
#include <SimpleCli.h>
#include <led.h>
#include <ticker.h>

#define UTC_OFFSET 60

MtkGps gps;

// LED to blink on every NMEA sentence
led_t gps_led;

// what to show on the terminal
int nmea_echo = 0;
int pmtk_echo = 0;
int show_data = 0;

// terminal to print to and for cli handler
#define MAX_TERM_STR_LEN 512

SerialTerminal term(MAX_TERM_STR_LEN);

extern int cli_handler(char *buf, PrintTerminal *term);
SimpleCli cli(&term, cli_handler);

// 2 seconds timer to print GPS data
int ticker(void *data);
ticker_t timer(2000, ticker);

void setup()
{
	// use USB serial as a terminal
	term.attach(&Serial);
	term.begin(PMTK_BR_115200);

	// attach debugging led to flush on every NMEA sentence
	gps_led.attach(13);

	// gps on Serial1: RX/TX pins 0/1
	// detect GPS baud rate
	term.print("Detecting GPS baud rate...\n");
	uint32_t gpsbr = gps.detect(&Serial1);
	if (gpsbr == PMTK_BR_INVALID) {
		gpsbr = PMTK_BR_9600;
		term.print("Unable to detect MTK3339, using default baud rate 9600\n");
	}
	else
		term.print("GPS baud rate %u detected\n", gpsbr);
	
	term.print("gps data echo is %s\n", show_data ? "on" : "off");
	term.print("gps nmea echo is %s\n", nmea_echo ? "on" : "off");
	term.print("gps pmtk echo is %s\n", nmea_echo ? "on" : "off");

	gps.attach(&Serial1);
	gps.begin(gpsbr);
	// gps unit initialization
	// enable RMC and GGA output; GGA every forth poll 
	gps.setOutput(NMEA_SEN_RMC, 0, 0, NMEA_SEN_GGA);
	if (nmea_echo) term.print("<%s\n", gps.cmd);
	// works with update rate 1Hz only
	gps.setEasyMode(PMTK_ARG_OFF);
	if (nmea_echo) term.print("<%s\n", gps.cmd);
	// Set the update rate 2Hz
	gps.setUpdateRate(2);
	if (nmea_echo) term.print("<%s\n", gps.cmd);
	// turn on some MTK features for better satellites tracking
	gps.sendCommand(PMTK_SET_AIC_MODE, PMTK_ARG_ON);
	if (nmea_echo) term.print("<%s\n", gps.cmd);
	gps.sendCommand(PMTK_SET_SBAS_ENABLE, PMTK_ARG_ON);
	if (nmea_echo) term.print("<%s\n", gps.cmd);
	gps.sendCommand(PMTK_SET_DGPS_MODE, PMTK_DGPS_WAAS);
	if (nmea_echo) term.print("<%s\n", gps.cmd);
	// set navigation speed threshold 0.2 m/s
	gps.setNavThreshold(PMTK_NAV_THRESHOLD_OFF);
	if (nmea_echo) term.print("<%s\n", gps.cmd);
	// request firmware release information
	gps.sendCommand(PMTK_Q_RELEASE);
	if (nmea_echo) term.print("<%s\n", gps.cmd);
	// it is still summer time in Ireland, add 1 hour to UTC...
	gps.setTimeZone(UTC_OFFSET);
}

void loop()
{
	int ch;
	const char *nmea;
	unsigned long tstamp;

	while(1) {
		// check terminal input
		if ((ch = term.getch()) != 0)
			cli.interact(ch);
		// check gps unit for new nmea sentences
		if ((nmea = gps.read()) != NULL) {
			gps_led.on();
			if (nmea_echo && nmea[1] == 'G')
				term.print(">%s\n", nmea);
			if (pmtk_echo && nmea[1] == 'P')
				term.print(">%s\n", nmea);
			gps.parse_nmea(nmea);
			gps_led.off();
		}

		// read milliseconds for timer(s) processing
		tstamp = millis();
		timer.tick(tstamp, NULL);
	}
}

// 2 seconds timer ticker procedure
int ticker(void *data)
{
	uint16_t msec;
	struct tm tfix;

	if (show_data && gps.isValid(NMEA_SEN_RMC)) {
		gps.getFixTime(&tfix, &msec);

		term.print("%02d/%02d/20%02d ", tfix.tm_mday, tfix.tm_mon, tfix.tm_year);
		term.print("%02d:%02d:%02d.%03d ", tfix.tm_hour, tfix.tm_min, tfix.tm_sec, msec);
		term.print("lat: %.8f lon: %.8f ", gps.latitude, gps.longitude);
		term.print("spd: %.2fN %.2fK ", gps.rmc.speed, gps.rmc.speed * 1.852);
		term.print("dir: %6.2f ", gps.rmc.course);
		term.print("alt: %5.2f ", gps.gga.altitude);
		term.print("nsat: %2d ", gps.gga.nsat);

		term.print("fix: %d quality: %d %s\n", gps.rmc.flags & NMEA_VALID, gps.gga.quality, gps.getFixQuality());
	}

	return 0;
}
