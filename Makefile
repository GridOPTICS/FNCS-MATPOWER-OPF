all: start_MATPOWER

start_MATPOWER.o: start_MATPOWER.cpp
	g++-4.4 -g -c  -ansi -D_GNU_SOURCE -I/usr/local/matlab_2013a/extern/include/cpp -I/usr/local/matlab_2013a/extern/include -DUNIX -DX11 -DGLNXA64 -DGCC -pthread  -O -DNDEBUG  "start_MATPOWER.cpp"

matpowerintegrator.o: matpowerintegrator.c
	g++-4.4 -g -c matpowerintegrator.c


start_MATPOWER: start_MATPOWER.o matpowerintegrator.o
	g++-4.4 -O -g -pthread  -o start_MATPOWER  start_MATPOWER.o matpowerintegrator.o -Wl,-rpath-link,/usr/local/matlab_2013a/bin/glnxa64 -L/usr/local/matlab_2013a/runtime/glnxa64  -lmwmclmcrrt -lm  ./libmpoption.so ./libopf.so ./librunpf.so ./librunopf.so -lfncs -lzmq

clean:
	rm -f *~
	rm -f *.o
	rm -f *.mod
	rm -f start_MATPOWER
	rm -f *.chk
	rm -f *.out
