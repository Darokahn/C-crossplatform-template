all: compile upload monitor

compile:
	arduino-cli compile --fqbn esp32:esp32:esp32 ./ --verbose

upload:
	arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 ./

monitor:
	screen /dev/ttyUSB0 115200
