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
