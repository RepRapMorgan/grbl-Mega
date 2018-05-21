~/programs/arduino-1.8.5/hardware/tools/avr/bin/avrdude -C ~/programs/arduino-1.8.5/hardware/tools/avr/etc/avrdude.conf -v -patmega2560 -D -Uflash:w:grbl.hex:i -cwiring  -P $1 -b115200


