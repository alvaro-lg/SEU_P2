CFLAGS=-Wall

PROGRAMAS= cpuid key key-bf key-bits rtc rtc-bits rtc-bf lAPIC-MAL lAPIC lAPIC-bf lAPIC-bits size getPCI-b0-s0-f0 gpio gpio-bits

all: $(PROGRAMAS)

key-bits rtc-bits lAPIC-bits gpio-bits: bits.h

clean:
	rm -f $(PROGRAMAS) *.o
