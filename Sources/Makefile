INSTALL_PATH = /home/vizproject/simbins/bin
MCR_PATH = /usr/local/MATLAB/MATLAB_Compiler_Runtime/v81
CXX = g++-4.4

CXXFLAGS =
CXXFLAGS += -g
CXXFLAGS += -ansi
CXXFLAGS += -pthread
CXXFLAGS += -O0

CPPFLAGS =
CPPFLAGS += -I$(MCR_PATH)/extern/include/cpp
CPPFLAGS += -I$(MCR_PATH)/extern/include
CPPFLAGS += -D_GNU_SOURCE
CPPFLAGS += -DUNIX
CPPFLAGS += -DX11
CPPFLAGS += -DGLNXA64
CPPFLAGS += -DGCC
CPPFLAGS += -DNDEBUG

LDFLAGS =
LDFLAGS += -Wl,-rpath-link,$(MCR_PATH)/bin/glnxa64
LDFLAGS += -L$(MCR_PATH)/runtime/glnxa64
LDFLAGS += -L.

LIBS =
LIBS += -lmwmclmcrrt
LIBS += -lm
LIBS += -lmpoption
LIBS += -lopf
LIBS += -lrunpf
LIBS += -lrunopf
LIBS += -lfncs
LIBS += -lzmq


all: start_MATPOWER

start_MATPOWER.o: start_MATPOWER.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS) $(CPPFLAGS)

matpowerintegrator.o: matpowerintegrator.c
	$(CXX) -c $< -o $@ $(CXXFLAGS) $(CPPFLAGS)

read_load_profile.o: read_load_profile.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS) $(CPPFLAGS)

read_model_dim.o: read_model_dim.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS) $(CPPFLAGS)

read_model_data.o: read_model_data.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS) $(CPPFLAGS)

start_MATPOWER: start_MATPOWER.o matpowerintegrator.o read_load_profile.o read_model_dim.o read_model_data.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS) $(LIBS)

clean:
	rm -f *~
	rm -f *.o
	rm -f *.mod
	rm -f start_MATPOWER
	rm -f *.chk
	rm -f *.out

install:
	cp *.so $(INSTALL_PATH)
	cp start_MATPOWER $(INSTALL_PATH)
