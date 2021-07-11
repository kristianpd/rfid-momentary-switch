# rfid-momentary-switch

This library is built for a NodeMCU board (but could be easily adapted to other arduino boards) and allows you to use one or two MRFC522 RFID sensors as momentary switches. 

See https://github.com/miguelbalboa/rfid for pinouts if using other arduino boards.


## Pinout
D0 - Programming pin, momentary switch, pull down resistor

D1 - Status pin 1 (HIGH out if there's an RFID tag on the 1st sensor, LOW otherwise)

D1 - Status pin 2 (HIGH out if there's an RFID tag on the 2nd sensor, LOW otherwise)

D3 - SDA Pin 1st sensor

D4 - RST PIN - both sensors shared

D5 - SCK PIN - both sensors shared

D6 - MISO PIN - both sensors shared

D7 - MOSI PIN - both sensors shared

D8 - SDA Pin 2nd sensor


## To program

D0 - Press once to start programming, put RFID on eithe RFID reader. This reader is now configured to only activate on that tag. 
D0 - Press twice to reset all RFIDs and allow any RFID to work
