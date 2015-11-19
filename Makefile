CC=$(shell root-config --cxx)

INCDIR=$(PWD)/include
LIBDIR:=$(shell root-config --libdir)
ROOTSYS:=$(shell root-config --prefix)
ROOTINCDIR:=$(shell root-config --incdir)
LDFLAGS:=$(shell root-config --libs)\
	-lRooFit -lRooFitCore
WFLAGS= -Wextra -Wall 
DFLAGS=-O3 #-O0 -g3 -fno-inline #-fprofile-arcs -ftest-coverage 
CXXFLAGS:=$(shell root-config --cflags)\
	-pg -I$(INCDIR)\
	$(DFLAGS) $(WFLAGS) 

BINS:=$(patsubst %.cxx,%,$(wildcard bin/*.cxx)) bin/skim-tree-response
BINOBJ:=$(patsubst %.cxx,%.o,$(wildcard bin/*.cxx)) bin/skim-tree-response.o
OBJS:=$(patsubst %.cxx,%.o,$(wildcard src/*.cxx))

.PHONY: all clean 
all: $(BINS)

# this is why root sucks
src/dict.cxx: include/LinkDef.h
	rootcint -f $@ -c $(CXXFLAGS) -p $^
src/libDict.so: src/dict.cxx
	$(CC) -shared -fPIC -o$@ $(CXXFLAGS) $^
src/analyze-cut-tree.o: src/analyze-tree.cxx
	$(CC) $(CXXFLAGS) -D__ANALYZE_TREE_CUTFLOW__ -c $< -o $@ 
# KISS
bin/%: bin/%.o $(OBJS) src/libDict.so
	$(CC) $^ -o $@ $(LDFLAGS) -L ./src -lDict
bin/skim-tree-response: src/root-sugar.o src/tree-utils.o src/simple-parser.o src/Cut.o bin/skim-tree.o src/analyze-cut-tree.o src/libDict.so
	$(CC) $^ -o $@ $(LDFLAGS) -L ./src -l Dict
%.o: %.cxx
	$(CC) $(CXXFLAGS) -c $< -o $@
clean:
	-rm $(BINS) $(BINOBJ) $(OBJS) src/dict.cxx src/dict.h src/libDict.so
