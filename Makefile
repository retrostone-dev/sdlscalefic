CFLAGS = -fPIC -O2 -fdata-sections -ffunction-sections -Wl,--as-needed -Wl,--gc-sections -s

all: sdlfix.so

sdlfix.so: scaler_sdl.o
	$(CC) -shared $(CFLAGS) -o $@ $<

clean:
	$(RM) sdlfix.so scaler_sdl.o
