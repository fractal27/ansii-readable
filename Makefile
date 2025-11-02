
include config.mk


all: ansiigen

ansiigen: $(SOURCES) $(HEADERS)
	$(CC) $(LIBS) $(SOURCES) $(CFLAGS) -o ansiigen

debug:  $(SOURCES) $(HEADERS)
	$(CC) $(LIBS) $(SOURCES) $(DFLAGS) -o ansiigen

install: ansiigen
	@# instead of using copy, I use cat because
	@# it doesn't create a new directory if the
	@# file is a directory
	cat ansiigen > $(PREFIX)/bin/ansiigen

clean:
	rm -f ansiigen

.PHONY: debug all
