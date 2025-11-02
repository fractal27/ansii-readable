
include config.mk

SOURCES := main.c ansii.c log.c
HEADERS := ansii.h log.h

ansiigen: $(SOURCES) $(HEADERS)
	$(CC) $(LIBS) $(SOURCES) $(CFLAGS) -o ansiigen

debug:  $(SOURCES) $(HEADERS)
	$(CC) $(LIBS) $(SOURCES) $(DFLAGS) -o ansiigen
