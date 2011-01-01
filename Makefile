all:
	gcc -I/usr/local/include  -std=c99 -shared -O2 -o bs2b.so -lbs2b bs2b.c -fPIC
