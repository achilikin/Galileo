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
#include <string.h>

#include "SimpleCli.h"

SimpleCli::SimpleCli(PrintTerminal *term, cmd_handler *processor)
{
	memset(cmd, 0, sizeof(cmd));
	memset(hist, 0, sizeof(cmd));
	cursor = 0;

	this->term = term;
	this->processor = processor;
}

void SimpleCli::attach(PrintTerminal *term)
{
	this->term = term;
}

int SimpleCli::interact(int ch)
{
	if ((ch == ARROW_UP) && (cursor == 0)) {
		// execute last successful command
		for(cursor = 0; ; cursor++) {
			cmd[cursor] = hist[cursor];
			if (cmd[cursor] == '\0')
				break;
		}
		if (term) term->puts(cmd);
		return 0;
	}

	if (ch == '\n' && *cmd) {
		if (processor && (processor(cmd, term) == 0))
			memcpy(hist, cmd, sizeof(cmd));
		for(uint8_t i = 0; i < cursor; i++)
			cmd[i] = '\0';
		cursor = 0;
	}

	// backspace processing
	if ((ch == '\b') || (ch == 127)) { // Backspace coded as 'Ctr+?' == 127
		if (cursor) {
			cursor--;
			cmd[cursor] = '\0';
			if (term) {
				term->putch(' ');
				term->putch('\b');
			}
		}
	}

	if (ch < ' ' || ch == 127)
		return 0;

	cmd[cursor++] = (uint8_t)ch;
	cursor &= CMD_LEN;
	// clean up in case of overflow (command too long)
	if (!cursor) {
		for(uint8_t i = 0; i <= CMD_LEN; i++)
			cmd[i] = '\0';
	}

	return 0;
}

int cmd_is(const char *cmd, const char *str)
{
	return strcmp(cmd, str) == 0;
}

int cmd_arg(char *cmd, const char *str, char **arg)
{
	char *end;
	size_t len = strlen(str);

	if (strncmp(cmd, str, len) == 0) {
		end = cmd + len;
		if (*end > ' ')
			return 0;
		for(; *end <= ' ' && *end != '\0'; end ++)
			*end = '\0';
		*arg = end;
		return 1;
	}
	return 0;
}
