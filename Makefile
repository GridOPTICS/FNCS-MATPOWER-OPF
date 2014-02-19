all: start_MATPOWER

start_MATPOWER.o: start_MATPOWER.cpp
	g++-4.4 -g -c  -ansi -D_GNU_SOURCE -I/usr/local/MATLAB/MATLAB_Compiler_Runtime/v81/extern/include/ -DUNIX -DX11 -DGLNXA64 -DGCC -pthread  -O -DNDEBUG  "start_MATPOWER.cpp"

matpowerintegrator.o: matpowerintegrator.c
	g++-4.4 -g -c matpowerintegrator.c


start_MATPOWER: start_MATPOWER.o matpowerintegrator.o
	g++-4.4 -O -g -pthread  -o start_MATPOWER  start_MATPOWER.o matpowerintegrator.o -Wl,-rpath-link,/usr/local/MATLAB/MATLAB_Compiler_Runtime/v81/bin/glnxa64 -L/usr/local/MATLAB/MATLAB_Compiler_Runtime/v81/runtime/glnxa64  -lmwmclmcrrt -lm  ./libmpoption.so ./libopf.so -lfncs_api -lzmq

clean:
	rm -f *~
	rm -f *.o
	rm -f *.mod
	rm -f start_MATPOWER
	rm -f *.chk
	rm -f *.out
