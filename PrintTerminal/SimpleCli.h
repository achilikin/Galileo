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
#ifndef __TERMINAL_CLI_H__
#define __TERMINAL_CLI_H__

/*
	Simple command line handler, uses PrintTerminal class to print feedback.
	Last successful command is stored in the history buffer and can be
	recalled by ARROW_UP key press.
*/
#include "SerialTerminal.h"

// command line max length, longer commands will reset the cursor
#define CMD_LEN 0x07F

// command processor, returns 0 on success
typedef int cmd_handler(char *buf, PrintTerminal *term);

class SimpleCli {
public:
	// args: terminal to print any feedback on, command processor
	SimpleCli(PrintTerminal *term, cmd_handler *processor);

	// attach this CLI to the terminal
	void attach(PrintTerminal *term);
	// process new character from the terminal
	int interact(int ch);

private:
	uint32_t cursor;
	char cmd[CMD_LEN + 1];
	char hist[CMD_LEN + 1];

	PrintTerminal *term;
	cmd_handler *processor;
};

// helper functions for commands handler
int cmd_is(const char *cmd, const char *str);
int cmd_arg(char *cmd, const char *str, char **arg);

#endif
