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

#include <SerialTerminal.h>
#include <SimpleCli.h>
#include <led.h>

led_t led;

// terminal to print to and for cli handler
#define MAX_TERM_STR_LEN 512

SerialTerminal term(MAX_TERM_STR_LEN);

extern int cli_handler(char *buf, PrintTerminal *term);
SimpleCli cli(&term, cli_handler);

void setup()
{
	// use USB serial as a terminal
	term.attach(&Serial);
	term.begin(115200);

	// we'll control onboard LED
	led.attach(13);
	led.off();
}

void loop()
{
	int ch;

	// check terminal input
	while((ch = term.getch()) != 0)
		cli.interact(ch);
	
	delay(50);
}

#define LINE_MAX 512

// list of supported commands 
const char *cmd_list[] = {
	"help",			// be nice and provide some help
	"led [on|off]", // LED manipulation
	"system [cmd]", // 'system format C:' for example ;)
	"reset console" // reset system console
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

	if (cmd_arg(cmd, "reset", &arg)) {
		// ignore arg...
		Serial2.begin(115200);
		delay(100);
		term->print("reseting...\n");
		Serial2.println("\nreseting...");
		delay(100);
		Serial2.end();
		return 0;
	}

	if (cmd_arg(cmd, "led", &arg)) {
		if (cmd_is(arg, "on")) {
			led.on();
			return 0;
		}
		if (cmd_is(arg, "off")) {
			led.off();
			return 0;
		}
		// no argument provided - show current state
		if (*arg == '\0') {
			term->print("led is %s\n", led.state() ? "on" : "off");
			return 0;
		}
	}

	if (cmd_arg(buf, "system", &arg)) {
		if (*arg == '\0') // by default show system name
			arg = "uname -a";
		FILE *pf = popen(arg, "r");
		if (pf) {
			while (fgets(wline, LINE_MAX, pf) != NULL) {
				term->print("%s", wline);
			}
			pclose(pf);
		}
		return 0;
	}

	term->print("Unknown command '%s'\n", buf);
	return -1;
}
