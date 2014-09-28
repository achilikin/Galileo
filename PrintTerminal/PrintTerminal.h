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
#ifndef __GALILEO_PRINT_TERMINAL_H__
#define __GALILEO_PRINT_TERMINAL_H__

/* virtual class for serial and ip terminals */

class PrintTerminal {
public:
	virtual void putch(uint8_t c) = 0;
	virtual void puts(const char *str) = 0;
	virtual int	 print(const char *format, ...) = 0;
};

#endif
