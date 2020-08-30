CC = icc # use whatever C++ compiler, e.g GNU g++
CXX = icpc
OPTIMIZE_FLAGS = -O3 -finline-functions -fargument-noalias-global -restrict -fno-alias
CXXFLAGS = -std=c++11 -Wall
INCS = # -I /path/to/fastflow
LDFLAGS = -pthread
#FFTRACE = -DTRACE_FASTFLOW
#TSTATS	= -DSTATS

TARGET = nq_thread_mmic nq_ff_mmic nq_thread nq_ff nq_serial_mmic nq_serial

#SOURCES = nq-bitpattern-ff.cpp  nq-bitpattern-threads.cpp

all: $(TARGET)

nq_serial_mmic: nq-serial.cpp

$(CXX) $(CXXFLAGS) -O3 -mmic -o $@ $^

nq_serial: nq-serial.cpp

$(CXX) $(CXXFLAGS) -O3 -o $@ $^

nq_thread_mmic: nq-bitpattern-threads.cpp

$(CXX) $(TSTATS) $(OPTIMIZE_FLAGS) $(CXXFLAGS) -mmic -o $@ $^ $(LDFLAGS)

nq_ff_mmic: nq-bitpattern-ff.cpp
$(CXX) $(FFTRACE) $(OPTIMIZE_FLAGS) $(CXXFLAGS) -DNO_DEFAULT_MAPPING $(INCS) -mmic -o $@ $^ $(LDFLAGS)

nq_thread: nq-bitpattern-threads.cpp
$(CXX) $(TSTATS) $(OPTIMIZE_FLAGS) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

nq_ff: nq-bitpattern-ff.cpp
$(CXX) $(FFTRACE) $(OPTIMIZE_FLAGS) $(CXXFLAGS) -DNO_DEFAULT_MAPPING $(INCS) -o $@ $^ $(LDFLAGS)

clean:
-rm -fr *.o *~
cleanall: clean
-rm -fr $(TARGET) *.d
