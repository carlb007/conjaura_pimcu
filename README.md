# conjaura_pimcu
 
Intermediary between the RPI and the Panel MCUs
This MCU Exists due to the raspberry pi's inability to run the SPI bus in slave mode (or at least that i know of). 
This poses a problem for returning data back to the pi as our RS485 BUS is only half duplex - so we cant simply MISO the databack whilst receiving LED data - or at least we cant without adding additional wires and RS485 tranceivers which bumps panel cost up.
The PI connected MCU gets around this issue by allowing us to buffer up panel return data and then batch send it back to the PI via a full duplex SPI MOSI | MISO connection.
