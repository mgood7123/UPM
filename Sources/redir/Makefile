files = mkdir

test: $(addprefix tests/,$(files))

tests/mkdir: redir tests/mkdir_
	./redir tests/relocation/mkdir/mkdir

redir: redir.o
	gcc redir.o -o redir

redir.o: callname.h
	gcc -c redir.c -I.

tests/mkdir_: mkdir.o
	gcc tests/relocation/mkdir/mkdir.o -o tests/relocation/mkdir/mkdir

mkdir.o:
	gcc -c tests/relocation/mkdir/mkdir.c -o tests/relocation/mkdir/mkdir.o
