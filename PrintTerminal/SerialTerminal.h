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
#ifndef __GALILEO_SERIAL_TERMINAL_H__
#define __GALILEO_SERIAL_TERMINAL_H__

#include <stdarg.h>
#include <TTYUART.h>

#include "PrintTerminal.h"

/* serial terminal */
class SerialTerminal:public PrintTerminal {
private:
	enum { F_DEBUG = 0x8000, F_ECHO = 0x0001 };
	uint32_t flags;
	char    *buffer;
	unsigned len;
	TTYUARTClass *port;

	uint8_t esc;
	uint8_t idx;

	int get_char(int ch);

public:
	SerialTerminal(unsigned maxlen); /* maximum length of a terminal string */
	~SerialTerminal(void);

	/* attach to a serial port 
	   on Galileo Serial is usb, Serial1 is RX0/TX1, Serial2 RS232/TTL headers
	*/
	void attach(TTYUARTClass *serial) {
		port = serial;
	}

	/* set attached serial baud rate */
	void begin(const uint32_t dwBaudRate) {
		if (port) port->begin(dwBaudRate); 
	}

	/* detach from the serial port
	   if Serial2 is used, it will be returned to Galileo system terminal
	*/
	void end(void) {
		if (port) port->end();
	}
	/* read serial port and parse arrow and functional keys */
	int getch(void);

	/* send one character to attached serial port */
	virtual void putch(uint8_t c);
	/* send null terminated string to attached serial port */
	virtual void puts(const char *str);
	/* print a string to attached serial port, up to maxlen in size*/
	virtual int	print(const char *format, ...);
	/* dump memory to attached terminal port */
	int dump(const char *prefix, void *pmem, uint32_t ulen);

	/* return last printed string */
	const char *string(void) { return buffer; }

	/* turn on/off debugging - useful when adding parsing for a new esc sequence */
	void debug(bool bOn);
	bool isdebug(void) { return flags & F_DEBUG; }
	/* turn on/off echo mode */
	void echo(bool bOn);
	bool isecho(void) { return flags & F_ECHO; }
};

inline void SerialTerminal::debug(bool bOn)
{
	if (bOn)
		flags |= F_DEBUG;
	else
		flags &= ~F_DEBUG;
}

inline void SerialTerminal::echo(bool bOn)
{
	if (bOn)
		flags |= F_ECHO;
	else
		flags &= ~F_ECHO;
}

// support for functional keys
#define KEY_FUNC    0x8000

// support for arrow keys for very simple one command deep history
#define ARROW_UP    0x8001
#define ARROW_DOWN  0x8002
#define ARROW_RIGHT 0x8003
#define ARROW_LEFT  0x8004

#define KEY_F1      0x8011
#define KEY_F2      0x8012
#define KEY_F3      0x8013
#define KEY_F4      0x8014
#define KEY_F5      0x8015
#define KEY_F6      0x8017
#define KEY_F7      0x8018
#define KEY_F8      0x8019
#define KEY_F9      0x8020
#define KEY_F10     0x8021
#define KEY_F11     0x8023
#define KEY_F12     0x8024

#define KEY_HOME    0x8100
#define KEY_INS     0x8200
#define KEY_DEL     0x8300
#define KEY_END     0x8400
#define KEY_PGUP    0x8500
#define KEY_PGDN    0x8600

#endif
