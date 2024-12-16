# MP4LHC: Makefile for executables to generate MC replicas
# Authors: Jun Gao, Pavel Nadolsky, 2015
#
# This code requires the LHAPDF library and headers, and BOOST library headers.
# The corresponding paths must be provided in environmental variables
# LHALIB, LHAINC, and BOOSTINC

CXX=g++

CXXFLAGS=-O0  #optimized compilation 
CXXFLAGS=-g   #debugging

ifeq ($(LHALIB),)
  LHALIB=$(shell lhapdf-config --libdir)
endif

ifeq ($(LHAINC),)
  LHAINC=$(shell lhapdf-config --incdir)
endif

ifeq ($(BOOSTINC),)
  BOOSTINC=/usr/include/boost
endif

mcgen.x: mcgen.cc
	$(CXX) -o mcgen.x $(CXXFLAGS) mcgen.cc -I$(LHAINC) -I$(BOOSTINC) -L$(LHALIB) -lLHAPDF

clean: 
	rm *.x *.o

