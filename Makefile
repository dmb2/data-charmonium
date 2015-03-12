CC=$(shell root-config --cxx)

INCDIR=$(PWD)/include
LIBDIR:=$(shell root-config --libdir)
ROOTINCDIR:=$(shell root-config --incdir)
LDFLAGS:=$(shell root-config --libs)\
	$(shell fastjet-config --libs)\
	-lNsubjettiness #-L ./lib #-lgcov
WFLAGS= -Wextra -Wall 
DFLAGS=-O0 -g3 -fno-inline #-fprofile-arcs -ftest-coverage 
CXXFLAGS= $(shell root-config --ldflags)\
	-pg -I$(INCDIR) -I$(ROOTINCDIR)\
	$(shell fastjet-config --cxxflags)\
	$(DFLAGS) $(WFLAGS) -ansi

BINS:=bin/skim-tree bin/skim-truth-tree bin/cut-flow-plots\
	bin/truth-study-plots bin/simple-parser-test\
	bin/make-stack-plots bin/make-plots bin/fit-and-sbs\
	bin/skim-tree-response

SKIM_DEPS:=src/tree-utils.o src/simple-parser.o src/Cut.o 
HISTO_DEPS:=src/histo-utils.o src/AtlasStyle.o\
	src/histo-meta-data.o src/stack-utils.o\
	src/plot-utils.o
.PHONY: all clean 
all: $(BINS)

# KISS
# this is why root sucks
src/dict.cxx: include/LinkDef.h
	rootcint -f $@ -c $(CXXFLAGS) -p $^
src/libDict.so: src/dict.cxx
	$(CC) -shared -fPIC -o$@ $(CXXFLAGS) $^
bin/simple-parser-test: bin/simple-parser-test.o src/simple-parser.o src/Cut.o
	$(CC) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
bin/skim-tree:  $(SKIM_DEPS) bin/skim-tree.o src/analyze-tree.o src/libDict.so
	$(CC) $^ -o $@ $(LDFLAGS) -L ./src -l Dict
bin/skim-tree-response: $(SKIM_DEPS) bin/skim-tree.o src/cut-flow-studies.o
	$(CC) $^ -o $@ $(LDFLAGS)
bin/skim-truth-tree:  $(SKIM_DEPS) bin/skim-truth-tree.o src/truth-studies.o
	$(CC) $^ -o $@ $(LDFLAGS) 
bin/cut-flow-plots: bin/cut-flow-plots.o $(HISTO_DEPS)
	$(CC) $^ -o $@ $(LDFLAGS) 
bin/truth-study-plots: $(HISTO_DEPS) bin/truth-study-plots.o 
	$(CC) $^ -o $@ $(LDFLAGS) 
bin/make-stack-plots: $(HISTO_DEPS) bin/make-stack-plots.o 
	$(CC) $^ -o $@ $(LDFLAGS) 
bin/make-plots: $(HISTO_DEPS) bin/make-plots.o
	$(CC) $^ -o $@ $(LDFLAGS)
bin/fit-and-sbs: src/fit-utils.o src/sbs-utils.o $(HISTO_DEPS) bin/fit-and-sbs.o 
	$(CC) $^ -o $@ -lRooFit -lRooFitCore $(LDFLAGS) 
%.o: %.cxx
	$(CC) $(CXXFLAGS) -c $< -o $@
clean:
	-rm $(BINS) bin/*.o src/*.o src/dict.cxx src/dict.h src/libDict.so
