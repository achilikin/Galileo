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
#include "SerialTerminal.h"

SerialTerminal::SerialTerminal(unsigned maxlen)
{
	len = 0;
	flags = F_ECHO;
	port = NULL;
	esc = idx = 0;

	// allocate buffer ones...
	buffer = (char *)malloc(maxlen+2);

	if (buffer) {
		memset(buffer, 0, maxlen+2);
		len = maxlen;
	}
}

SerialTerminal::~SerialTerminal(void)
{
	if (buffer) {
		free(buffer);
		buffer = NULL;
	}
}

int	SerialTerminal::print(const char *format, ...)
{
	int retval;
	va_list ap;
	va_start(ap, format);

	retval = vsnprintf(buffer, len, format, ap);

	va_end(ap);
	if (retval < 0 || (len > 0 && retval >= len))
		return -1;

	if (port)
		port->print(buffer);

	return retval;
}

void SerialTerminal::putch(uint8_t c)
{
	if (port)
		port->write(c);
}

void SerialTerminal::puts(const char *str)
{
	if (port) {
		while(*str)
			port->write(*str++);
	}
}

int	SerialTerminal::get_char(int ch)
{
	if (ch == 0)
		return 0;
	// ESC sequence state machine for VT100
	if (ch == 27) {
		esc = 1;
		return 0;
	}

	if (esc == 1) {
		if (ch == '[') {
			esc = 2;
			return 0;
		}
	}

	if (esc == 2) {
		if ((ch >= 'A') && (ch <= 'D')) {
			ch = 0x8000 | (ch - 'A' + 1);
			esc = 0;
			return ch;
		}
		if ((ch >= '1') && (ch <= '6')) {
			esc = 3;
			idx = ch - '0';
			return 0;
		}
	}

	if (esc == 3) {
		if (ch == '~') {
			esc = 0;
			ch = 0x8000 | (idx << 8);
			return ch;
		}
		if ((ch >= '0') && (ch <= '9')) {
			idx <<= 4;
			idx |= ch - '0';
			esc = 4;
			return 0;
		}
		esc = 0;
		return 0;
	}

	if (esc == 4) {
		if (ch == '~') {
			esc = 0;
			ch = idx | 0x8000;
			return ch;
		}
		esc = 0;
		return 0;
	}

	esc = 0;
	if (ch == '\r')
		ch = '\n';
	if (flags & F_ECHO)
		putch(ch);

	return ch;
}

int	SerialTerminal::getch(void)
{
	int c = 0;

	if (port && port->available()) {
		c = port->read();
		if (flags & F_DEBUG) {
			if (c < ' ') {
				port->write('\'');
				port->print(c);
				port->write('\'');
			}
			else 
				port->write(c);
			c = 0;	
		}
	}

	return get_char(c);
}

int SerialTerminal::dump(const char *prefix, void *pdump, uint32_t ulen)
{
	char line[70];
	uint32_t i, n;
	static const char *hex = "0123456789ABCDEF";
	const uint8_t *pmem = (const uint8_t *)pdump;

	// initialize output line
	memset(line, ' ', sizeof(line));
	line[24] = '|';	line[50] = '|';	line[68] = '\0';

	for(i = n = 0; n < ulen; n++) {
		line[i++] = hex[(pmem[n] >> 4) & 0x0F];
		line[i++] = hex[pmem[n] & 0x0F];
		line[(n & 0x0F) + 52] = isprint(pmem[n]) ? pmem[n] : '.';
		i++;
		if (i == 24)
			i += 2;
		if (i == 50) {
			i = 0;
			puts(prefix); puts(line); puts("\n");
			// re-initialize output line
			memset(line, ' ', sizeof(line));
			line[24] = '|';	line[50] = '|';	line[68] = '\0';
		}
	}
	if (i != 0) {
		puts(prefix); puts(line); puts("\n");
	}
	return 0;
}
