#
# Example Makefile for ROOTANA-based projects
#

CXXFLAGS = -g -O2 -Wall -Wuninitialized

# required ZLIB library

CXXFLAGS += -DHAVE_LIBZ

# required ROOTANA library

ifndef ROOTANASYS
ROOTANASYS=..
endif

CXXFLAGS += -I$(ROOTANASYS)/include
LIBS += -L$(ROOTANASYS)/lib -lrootana

# ROOT libraries

ifdef ROOTSYS

ROOTCFLAGS   := $(shell root-config --cflags)
ROOTFEATURES := $(shell root-config --features)
ROOTGLIBS    := $(shell root-config --glibs) -lThread
HAVE_ROOT_HTTP := $(findstring http,$(ROOTFEATURES))
HAVE_ROOT_XML  := $(findstring xml,$(ROOTFEATURES))

CXXFLAGS  += -DHAVE_ROOT $(ROOTCFLAGS)

ifdef HAVE_ROOT_XML
CXXFLAGS  += -DHAVE_ROOT_XML
ROOTGLIBS += -lXMLParser
endif

ifdef HAVE_ROOT_HTTP
CXXFLAGS  += -DHAVE_ROOT_HTTP -DHAVE_THTTP_SERVER
ROOTGLIBS += -lRHTTP
endif

endif # ROOTSYS

# optional MIDAS library

ifdef MIDASSYS

MIDASLIBS = $(MIDASSYS)/lib/libmidas.a -lutil -lrt
CXXFLAGS += -DHAVE_MIDAS -DOS_LINUX -Dextname -I$(MIDASSYS)/include

UNAME := $(shell uname)
ifeq ($(UNAME),Darwin)
MIDASLIBS = $(MIDASSYS)/lib/libmidas.a
endif

LIBS += $(MIDASLIBS)

endif # MIDASSYS

OBJS:= TTreeMaker.o TDT743RawData.o

all: $(OBJS)  midas2root_mppc_simpler.exe midas2root_mppc.exe midas2root_citiroc.exe

midas2root_mppc_simpler.exe: midas2root_mppc_simpler.cxx $(OBJS) 
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LIBS) $(ROOTGLIBS) -lm -lz -lpthread -lssl -lutil

midas2root_mppc.exe: midas2root_mppc.cxx $(OBJS) 
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LIBS) $(ROOTGLIBS) -lm -lz -lpthread -lssl -lutil

midas2root_citiroc.exe: midas2root_citiroc.cxx $(OBJS)
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LIBS) $(ROOTGLIBS) -lm -lz -lpthread -lssl -lutil

# midas2root_test.exe: midas2root_test.cxx $(OBJS)
# 	$(CXX) -o $(CXXFLAGS) $^ $(LIBS) $(ROOTGLIBS) -lm -lz -lpthread -lssl -lutil


%.o: %.cxx
	$(CXX) -o $@ $(CXXFLAGS) -c $<

dox:
	doxygen

clean::
	-rm -f *.o *.a
	-rm -f *.exe
	-rm -rf *.exe.dSYM

# end