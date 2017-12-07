
CC = gcc

INCLUDES = /UPM/Sources/bash-4.4 /UPM/Sources/bash-4.4/builtins /UPM/Sources/bash-4.4/include
DEFINES = _GNU_SOURCE

LIBFLAGS = -shared -fPIC
CWARN = -Wall
COPT = -O0
ifneq ($(DEBUG),)
	CDEBUG = -ggdb3
else
	CDEBUG =
endif
CINCLUDES = $(foreach d,$(INCLUDES),-I$(d))
CDEFINES = $(foreach d,$(DEFINES),-D$(d))

CFLAGS = $(CWARN) $(COPT) $(CINCLUDES) $(CDEFINES) $(LIBFLAGS) $(CDEBUG)

FUSEFLAGS = $(shell pkg-config fuse --cflags --libs)

booze.so: booze.c
	$(CC) $(CFLAGS) $(FUSEFLAGS) -o $@ $<
