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

/*
	yet another blink example using YAHL - yet another helper library 
*/
#include <led.h>
#include <ticker.h>

int blink(void *data);

void setup()
{
	// nothing to setup, will use local variables
}

void loop()
{
	led_t light;
	ticker_t led(1000, blink); // call blink every 1000 msec

	light.attach(13); // onboard LED
	light.off();
	
	while(1) {
		led.tick(millis(), &light);
		delay(10);
	}
}

int blink(void *data)
{
	led_t *pled = (led_t *)data;

	// invert current LED state
	if (pled->state())
		pled->off();
	else
		pled->on();

	return 0;
}
