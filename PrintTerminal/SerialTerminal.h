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
