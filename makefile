all: ti83keypad.c
	gcc -Wall -o ti83keypad ti83keypad.c -L/usr/X11R6/lib -lX11 -lXtst -lwiringPi  -lpthread `pkg-config --cflags --libs gtk+-2.0`

clean:
	$(RM) ti83keypad
