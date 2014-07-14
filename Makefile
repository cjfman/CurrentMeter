all: main.hex
	
main.elf: main.c
	avr-gcc -mmcu=atmega328p -Wall -O1 -o main.elf main.c

main.hex:main.elf
	avr-objcopy -O ihex main.elf main.hex

program: main.hex
	avrdude -c usbtiny -F -p m328p -e -U flash:w:main.hex

ardisp: main.hex
	avrdude -P /dev/tty.usbmodemfd131 -b 19200 -c avrisp -F -p m328p -e -U \
	flash:w:main.hex

clean:
	rm -rf main.elf main.hex
