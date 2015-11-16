/*	BSD License
	Copyright (c) 2015 Andrey Chilikin https://github.com/achilikin

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
#ifndef __MSEC_TICKER_H__
#define __MSEC_TICKER_H__

/*
	Simple timer handler, create it and then call tick(millis()) periodically
*/

#define TICK_SEC(x)  (((uint32_t)x)*1000l)
#define TICK_MIN(x)  (((uint32_t)x)*60l*1000l)
#define TICK_HOUR(x) (((uint32_t)x)*60l*60l*1000l)

// tick handler, called every time when time interval is expired
typedef int8_t tickerHandler(void *data);

class ticker_t {
public:
	// tick time interval (milliseconds) and tick handler
	ticker_t(uint32_t period, tickerHandler *phanler);

	// checks if time interval expired and call tick handler
	int8_t tick(unsigned long current_millis, void *data);

private:
	uint32_t interval;
	uint32_t last_call;
	tickerHandler *handler;
public:
	uint32_t set_interval(uint32_t span) { uint32_t current = interval;  interval = span; return current; }
	uint32_t get_interval(void) { return interval; }
	void reset(void) { last_call = 0; }
};

ticker_t::ticker_t(uint32_t period, tickerHandler *phanler)
{
	interval = period;
	handler = phanler;
	last_call = 0;
}

int8_t ticker_t::tick(uint32_t current_millis, void *data)
{
	uint32_t span = current_millis - last_call;
	if (span >= interval) {
		last_call = current_millis - (span - interval);
		return handler(data);
	}
	return 0;
}

#endif
