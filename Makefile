CC=$(shell root-config --cxx)
# Build this library the same way root was compiled/linked
INCDIR=$(PWD)/include
LIBDIR:=$(shell root-config --libdir)
ROOTINCDIR:=$(shell root-config --incdir)
LDFLAGS:=$(shell root-config --libs) $(shell fastjet-config --libs) -lNsubjettiness#-L ./lib #-lgcov
WFLAGS= -Wextra -Wall 
DFLAGS=-O3 #-fprofile-arcs -ftest-coverage 
CXXFLAGS=$(shell root-config --ldflags) -pg -I$(INCDIR) -I$(ROOTINCDIR)	$(shell fastjet-config --cxxflags)\
$(DFLAGS) $(WFLAGS) -ansi

BINSRC:=$(wildcard bin/*.cxx)
BINS:=skim-tree skim-truth-tree cut-flow-plots simple-parser-test
BINOBJ:=$(BINSRC:.cxx=.o)

.PHONY: all clean 
all: $(BINS)
debug: 
	@echo $(BINSRC)
	@echo $(BINOBJ)
# KISS
simple-parser-test: bin/simple-parser-test.o src/simple-parser.o
	$(CC) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
skim-tree: src/Cut.o src/cut-flow-studies.o src/tree-utils.o bin/skim-tree.o 
	$(CC) $^ -o $@ $(LDFLAGS) 
skim-truth-tree: src/truth-studies.o src/Cut.o src/tree-utils.o bin/skim-truth-tree.o 
	$(CC) $^ -o $@ $(LDFLAGS) 
cut-flow-plots: src/AtlasStyle.o bin/cut-flow-plots.o 
	$(CC) $^ -o $@ $(LDFLAGS) 

bin/%.o: ./bin/%.cxx
	$(CC) $(CXXFLAGS) -c $< -o $@
src/%.o: ./src/%.cxx
	$(CC) $(CXXFLAGS) -c $< -o $@
clean:
	-rm *.o $(BINS) bin/*.o src/*.o
