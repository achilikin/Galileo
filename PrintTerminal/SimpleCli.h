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
