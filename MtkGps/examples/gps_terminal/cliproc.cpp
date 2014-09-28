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
#include <SimpleCli.h>

/*
	CLI for testing MTK3339 GPS unit
*/

// some external variables from theb main loop
extern MtkGps gps;
extern int nmea_echo;
extern int pmtk_echo;
extern int show_data;
static int gps_standby = 0;

#define LINE_MAX 512

// list of supported commands 
const char *cmd_list[] = {
	"gps data [on|off]",
	"gps nmea [on|off]",
	"gps pmtk [on|off]",
	"gps standby [on|off]",
	"gps baud [4800, 9600, 19200, 38400, 57600, 115200]",
	"gps release [get]",
	"pmtk command",
	"set time",
	"system [cmd]",
};

// called by terminal for every new command line
int cli_handler(char *buf, PrintTerminal *term)
{
	char *arg;
	char cmd[CMD_LEN + 1];
	char wline[LINE_MAX];

	memcpy(cmd, buf, sizeof(cmd));

	if (cmd_is(cmd, "help")) {
		term->print("List of supported commands:\n");
		for(uint8_t i = 0; i < sizeof(cmd_list)/sizeof(cmd_list[0]); i++)
			term->print("   %s\n", cmd_list[i]);
		return 0;
	}

	if (cmd_is(cmd, "set time")) {
		struct tm fixt;
		gps.getFixTime(&fixt);
		sprintf(wline, "date --set=\"%d-%02d-%02d %02d:%02d:%02d\"",
			fixt.tm_year + 2000, fixt.tm_mon, fixt.tm_mday,
			fixt.tm_hour, fixt.tm_min, fixt.tm_sec);
		term->print("%s\n", wline);
		system(wline);
		return 0;
	}

	if (cmd_arg(buf, "gps nmea", &arg)) {
		if (cmd_is(arg, "on"))
			nmea_echo = 1;
		if (cmd_is(arg, "off"))
			nmea_echo = 0;
		if (*arg == '\0')
			term->print("nmea echo is %s\n", nmea_echo ? "on" : "off");
		return 0;
	}

	if (cmd_arg(buf, "gps data", &arg)) {
		if (cmd_is(arg, "on"))
			show_data = 1;
		if (cmd_is(arg, "off"))
			show_data = 0;
		if (*arg == '\0')
			term->print("gps data is %s\n", show_data ? "on" : "off");
		return 0;
	}

	if (cmd_arg(buf, "gps pmtk", &arg)) {
		if (cmd_is(arg, "on"))
			pmtk_echo = 1;
		if (cmd_is(arg, "off"))
			pmtk_echo = 0;
		if (*arg == '\0')
			term->print("pmtk echo is %s\n", pmtk_echo ? "on" : "off");
		return 0;
	}

	if (cmd_arg(buf, "gps release", &arg)) {
		if (*arg == '\0')
			term->print("%s\n", gps.getFWrelease());
		else {
			gps.sendCommand(PMTK_Q_RELEASE);
			if (nmea_echo) term->print(">%s\n", gps.cmd);
		}
		return 0;
	}

	if (cmd_arg(buf, "gps baud", &arg)) {
		if (*arg == '\0') {
			term->print("gps baud rate %u\n", gps.getNmeaBaudRate());
			return 0;
		}
		uint32_t baud = (uint32_t)atoi(arg);
		if (gps.setNmeaBaudRate(baud) == 0) {
			if (nmea_echo) term->print(">%s\n", gps.cmd);
			gps.begin(baud);
			term->print("gps baud rate set to %u\n", baud);
			return 0;
		}
		term->print("baud rate %u not supported\n", baud);
		return -1;
	}

	if (cmd_arg(buf, "gps standby", &arg)) {
		if (cmd_is(arg, "on")) {
			gps.sendCommand(PMTK_CMD_STANDBY_MODE, PMTK_ARG_ON);
			if (nmea_echo) term->print(">%s\n", gps.cmd);
			gps_standby = 1;
		}
		if (cmd_is(arg, "off")) {
			gps.sendCommand(PMTK_CMD_STANDBY_MODE, PMTK_ARG_OFF);
			if (nmea_echo) term->print(">%s\n", gps.cmd);
			gps_standby = 0;
		}
		if (*arg == '\0')
			term->print("gps standby is %s\n", gps_standby ? "on" : "off");
		return 0;
	}

	if (cmd_arg(buf, "pmtk", &arg)) {
		if (*arg != '\0') {
			sprintf(wline, "$PMTK%s", arg);
			gps.sendStr(wline);
			term->print("<%s\n", gps.cmd);
			return 0;
		}
	}

	if (cmd_arg(buf, "system", &arg)) {
		if (*arg == '\0')
			arg = "uname -a";

		FILE *pf = popen(arg, "r");
		if (pf) {
			while (fgets(wline, LINE_MAX, pf) != NULL)
				term->print("%s", wline);
			pclose(pf);
		}
		return 0;
	}

	term->print("Unknown command '%s'\n", cmd);
	return -1;
}
