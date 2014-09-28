## GPS/CLI/YAHL Libraries for Intel Galileo
Some useful (at least for me) libraries for Intel Galileo:

* **MtkGps** - for communication with GPS modules which talk MediaTek PMTK protocol. MtkGps examples use two other libraries, so short intro  to YAHL and PrintTerminal required.   
* **YAHL** - two helper objects
* **PrintTerminal** - implements formatted printing to a serial port and parsing of a few VT100 escape sequences, so you can do something very special with all these very functional F1 to F12 keys. 

To try it out put MtkGPS, PrintTerminal and YAHL folders to your Arduino libraries as usual.
 
## MtkGps - library for GPS modules compatible with MediaTek $PMTK protocol 
Galileo library for GPS modules compatible with MediaTek PMTK protocol. Tested with the Adafruit Ultimate GPS GTop module using MTK3399 chipset:

* [http://www.adafruit.com/products/746](http://www.adafruit.com/products/746)
* [https://learn.adafruit.com/adafruit-ultimate-gps](https://learn.adafruit.com/adafruit-ultimate-gps)

To use Adafruit Ultimate GPS with Galileo connect GPS RX to Galileo TX1, GPS TX to Galileo RX0, GPS Vin to Galileo 5V, GPS GND to Galileo GND. Easy... Someday I will install Fritzing again and add nice and colorful picture here. Someday. 

MtkGps should work with other GPS modules as well, but PMTK packet types might be different and changes in MtkGps.h required, use gps_terminal example to send PMTK commands to your module and check how it replys to them.

Have not implemented data logging (LOCUS) commands, maybe in future. Adding callbacks to parse log data should not be a problem, right?

MtkGps Includes the following examples:
### gps_terminal
Connects to GPS module, parses NMEA messages and prints most common GPS data. Uses `led_t` and `ticker_t` from **YAHL** library, `SimpleCli` and `SerialTerminal` from **PrintTerminal**.

Startup window after command **help** was executed:

![GPS terminal](http://achilikin.com/github/gps_terminal.png)

First thing it does when starts - tries to detect current baud rate of the GPS module. You can always change it with **gps baud** command, but some packets like 9600 only, for example ***PMTKCHN*** sentence can force module to reset and switch to 9600. At least it happend a few times with my GPS. By default all data output is off, use corresponding commads to turn it on or change **gps_terminal.ino**.

Understands a few commands:

    * gps data [on|off] - on/off some general GPS data: time, date, lat, lon, etc...
    * gps nmea [on|off] - on/off NMEA sentences from GPS
    * gps pmtk [on|off] - on/off PMTK sentences from GPS
    * gps baud          - set communication speed, 14400 not supported
    * gps release       - prints GPS module firmware information
    * pmtk <command>    - adds $PMTK to specified command and sends to GPS module
    * set time          - set system time using GPS time of the last fix, use MtkGps::setTimeZone() to add offset to UTC time
    * system [cmd]      - system command line fun


See MtkGps.h for the list of PMTK commands I've found so far. **pmtk** commands session example: 

![GPS terminal](http://achilikin.com/github/gps_term_pmtk.png)

With **gps data** turned on:

![GPS terminal png](http://achilikin.com/github/gps_term_data.png)

### sav_view
Just a simple example how to configure NMEA output and use parsed data. Collects and shows information about visible satellites on Serial2:

![Satellites in view png](http://achilikin.com/github/sat_view.png)  

### bridge
Creates a bridge between RX0/TX1 serial port and USB serial port, so external software running on a PC can be used. Useful if you want to view skyplot, upload EPO or upgrade firmware. Also can monitor what is happening on the bridge and display communication log on system console (connected to RS232 on Galileo v1 or TTL serial headers on Galileo v2).
Will automatically detect if PC application turns on NMEA binary format and switch to dumping mode. For example, hex dump of EPO being uploaded:  

![hex dump of EPO being uploaded](http://achilikin.com/github/Bridge.png)

Tested with MiniGPS and MT3339 GPS Tool from [Adafruit](https://learn.adafruit.com/adafruit-ultimate-gps/downloads-and-resources). Works fine with any general GPS NMEA parsing applications as well. 

I was able to upgrade firmware to the latest version you can find at Adafruit Ultimate GPS [F.A.Q](https://learn.adafruit.com/adafruit-ultimate-gps/faq) page. Just in case if you want to repeat this exercise as well, use the ~~force~~ 9600 baudrate and if you brick your GPS module it is your ~~life~~ brick. 

## YAHL - yet another helper library
Contains only two files: 

* **led.h** - `led.on()` looks better than `digitalWrite(13, HIGH)`, isn't it?
* **ticker.h** - ticks every N milliseconds and calls specified function. Tick, tock...

Includes the only one example: everybody's favourite **blink** 

## PrintTerminal
Why? Just compare:

```
term.attach(&Serial);
term.print("Date: %02d/%02d/20%02d\n", tfix.tm_mday, tfix.tm_mon, tfix.tm_year);
```

and

```
Serial.print("Date: ");
Serial.print(tfix.tm_mday);
Serial.print("/");
Serial.print(tfix.tm_mon);
Serial.print("/20");
Serial.println(tfix.tm_year);
```

In addition to just printing it can dump hex data, see **bridge** example above.

Contains `SerialTerminal`, `SimpleCli` objects and **cli** example showing how to use them:

![cli example](http://achilikin.com/github/cli.png)

**system** with no argument will show `uname -a` output. **system top -n 1** - one page of `top` output, and so on and so forth... Don't forget to add **-n 1** for **system top** or it will run until you re-upload the sketch. 

**reset console** command will reset system console (Serial2) and return it back to the system. Useful if you switch between different examples and eventually system console got blocked because `Serial2.end()` was not called.

Have fun!
