all: start_MATPOWER

start_MATPOWER.o: start_MATPOWER.cpp
	g++-4.4 -g -c  -ansi -D_GNU_SOURCE -I/usr/local/MATLAB/MATLAB_Compiler_Runtime/v81/extern/include/cpp -I/usr/local/MATLAB/MATLAB_Compiler_Runtime/v81/extern/include -DUNIX -DX11 -DGLNXA64 -DGCC -pthread  -O0 -DNDEBUG  "start_MATPOWER.cpp"

matpowerintegrator.o: matpowerintegrator.c
	g++-4.4 -g -c matpowerintegrator.c

read_load_profile.o: read_load_profile.cpp
	g++-4.4 -g -c read_load_profile.cpp

read_model_dim.o: read_model_dim.cpp
	g++-4.4 -g -c read_model_dim.cpp

read_model_data.o: read_model_data.cpp
	g++-4.4 -g -c read_model_data.cpp

start_MATPOWER: start_MATPOWER.o matpowerintegrator.o read_load_profile.o read_model_dim.o read_model_data.o
	g++-4.4 -O -g -pthread  -o start_MATPOWER  start_MATPOWER.o matpowerintegrator.o read_load_profile.o read_model_dim.o read_model_data.o -Wl,-rpath-link,/usr/local/MATLAB/MATLAB_Compiler_Runtime/v81/bin/glnxa64 -L/usr/local/MATLAB/MATLAB_Compiler_Runtime/v81/runtime/glnxa64  -lmwmclmcrrt -lm  ./libmpoption.so ./libopf.so ./librunpf.so ./librunopf.so -lfncs -lzmq

clean:
	rm -f *~
	rm -f *.o
	rm -f *.mod
	rm -f start_MATPOWER
	rm -f *.chk
	rm -f *.out
