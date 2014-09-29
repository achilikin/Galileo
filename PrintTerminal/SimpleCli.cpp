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
