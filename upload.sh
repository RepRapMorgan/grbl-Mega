~/programs/arduino-1.6.13/hardware/tools/avr/bin/avrdude -C ~/programs/arduino-1.6.13/hardware/tools/avr/etc/avrdude.conf -v -patmega2560 -Uflash:w:grbl.hex:i -cwiring  -P /dev/ttyUSB0 -b115200
