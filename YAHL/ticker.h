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
#ifndef __MSEC_TICKER_H__
#define __MSEC_TICKER_H__

/*
	Simple timer handler, create it and then call tick(millis()) periodically
*/

// tick handler, called every time when time interval is expired
typedef int tickerHandler(void *data);

class ticker_t {
public:
	// tick time interval (milliseconds) and tick handler
	ticker_t(uint32_t period, tickerHandler *phanler);

	// checks if time interval expired and call tick handler
	int tick(unsigned long current_millis, void *data);

private:
	uint32_t interval;
	unsigned long last_call;
	tickerHandler *handler;
};

ticker_t::ticker_t(uint32_t period, tickerHandler *phanler)
{
	interval = period;
	handler = phanler;
	last_call = 0;
}

int ticker_t::tick(unsigned long current_millis, void *data)
{
	if ((current_millis - last_call) >= interval) {
		last_call = current_millis;
		return handler(data);
	}
	return 0;
}

#endif
