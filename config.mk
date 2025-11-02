
# optional params
CC     := gcc
CFLAGS := -O2
DFLAGS := $(CFLAGS) -ggdb -fsanitize=address
PREFIX := /usr/local


# should be hard-coded
SOURCES := main.c ansii.c log.c
HEADERS := ansii.h log.h



