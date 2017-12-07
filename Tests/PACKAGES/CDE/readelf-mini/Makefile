# pgbovine - try to be compatible with older glibc (use "objdump -x" to see)
# pgbovine - also for backwards-compatibility:
#   see http://sites.google.com/site/avinesh/binaryincompatibilitybetweenrhel4andrhel
CFLAGS = -g -O2 -fno-stack-protector -U_FORTIFY_SOURCE -D_GNU_SOURCE -Wl,--hash-style=both

%.o : %.c	
	gcc $(CFLAGS) -c -Iinclude -Wall $< -o $@

libreadelf-mini.a: readelf-mini.o version.o unwind-ia64.o dwarf.o
	ar rcs libreadelf-mini.a readelf-mini.o version.o unwind-ia64.o dwarf.o

readelf-mini: readelf-mini.o version.o unwind-ia64.o dwarf.o
	gcc -o readelf-mini readelf-mini.o version.o unwind-ia64.o dwarf.o

all: libreadelf-mini.a

clean:
	rm -f libreadelf-mini.a readelf-mini *.o
