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
#ifndef __LED_CONTROL_H__
#define __LED_CONTROL_H__

/*
	It is so boring to type degitalWrite() every time when you want control a LED
*/

struct led_t {
	int pin;

	void attach(int ledpin) {
		pin = ledpin;
		pinMode(pin, OUTPUT);
	}

	void on(void) {
		digitalWrite(pin, HIGH);
	}

	void off(void) {
		digitalWrite(pin, LOW);
	}
	
	int state(void) {
		return digitalRead(pin);
	}
};

#endif
