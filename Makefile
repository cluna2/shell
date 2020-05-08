CC = gcc
CFLAGS = -g3 -Wall -Wextra -Wconversion -Wcast-qual -Wcast-align -g
CFLAGS += -Winline -Wfloat-equal -Wnested-externs
CFLAGS += -pedantic -std=gnu99 -Werror -D_GNU_SOURCE
EXECS = 33sh 33noprompt
PROMPT = -DPROMPT
HEADERS =  parse.h built_in.h read.h redirect.h libs.h jobs.h reaper.h
SOURCES = parse.c built_in.c read.c redirect.c sh.c jobs.c reaper.c

.PHONY: all clean

all: $(EXECS)
	/course/cs0330/bin/cs0330_cleanup_shell

#TODO: compile your program, including the -DPROMPT macro
33sh: $(HEADERS) $(SOURCES)
	$(CC) $(CFLAGS) -DPROMPT $(SOURCES) -o 33sh

#TODO: compile your program without the prompt macro
33noprompt: $(HEADERS) $(SOURCES) 
	$(CC) $(CFLAGS) $(SOURCES) -o 33noprompt

#TODO: clean up any executable files that this Makefile has produced
clean:
	/course/cs0330/bin/cs0330_cleanup_shell
	rm -f $(EXECS)
