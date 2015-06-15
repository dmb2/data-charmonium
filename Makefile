CC=$(shell root-config --cxx)

INCDIR=$(PWD)/include
LIBDIR:=$(shell root-config --libdir)
ROOTSYS:=$(shell root-config --prefix)
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

BINS:=$(patsubst %.cxx,%,$(wildcard bin/*.cxx)) bin/skim-tree-response
BINOBJ:=$(patsubst %.cxx,%.o,$(wildcard bin/*.cxx)) bin/skim-tree-response.o
OBJS:=$(patsubst %.cxx,%.o,$(wildcard src/*.cxx))

COMMON_DEPS:=src/root-sugar.o

SKIM_DEPS:=$(COMMON_DEPS) src/tree-utils.o src/simple-parser.o src/Cut.o 

HISTO_DEPS:=$(COMMON_DEPS) src/stack-utils.o src/AtlasStyle.o\
	src/histo-meta-data.o src/histo-utils.o\
	src/plot-utils.o src/color.o src/math.o\
	src/histo-style.o 
.PHONY: all clean 
all: $(BINS)

# KISS
# this is why root sucks
src/dict.cxx: include/LinkDef.h
	rootcint -f $@ -c $(CXXFLAGS) -p $^
src/libDict.so: src/dict.cxx
	$(CC) -shared -fPIC -o$@ $(CXXFLAGS) $^
src/analyze-cut-tree.o: src/analyze-tree.cxx
	$(CC) $(CXXFLAGS) -D__ANALYZE_TREE_CUTFLOW__ -c $< -o $@ 
bin/tree-bug: bin/tree-bug.o src/libDict.so
	$(CC) $^ -o $@ $(LDFLAGS) -L ./src -lDict
bin/make-closure-sample: $(COMMON_DEPS) bin/make-closure-sample.o src/simple-parser.o src/Cut.o
bin/simple-parser-test: bin/simple-parser-test.o src/simple-parser.o src/Cut.o
	$(CC) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
bin/skim-tree:  $(SKIM_DEPS) bin/skim-tree.o src/analyze-tree.o src/libDict.so
	$(CC) $^ -o $@ $(LDFLAGS) -L ./src -l Dict
bin/skim-tree-response: $(SKIM_DEPS) bin/skim-tree.o src/analyze-cut-tree.o src/libDict.so
	$(CC) $^ -o $@ $(LDFLAGS) -L ./src -l Dict
bin/skim-truth-tree:  $(SKIM_DEPS) bin/skim-truth-tree.o src/truth-studies.o
	$(CC) $^ -o $@ $(LDFLAGS) 
bin/print-plot-panels: $(HISTO_DEPS) bin/print-plot-panels.o
	$(CC) $^ -o $@ $(LDFLAGS)
bin/%-plots: $(HISTO_DEPS) bin/%-plots.o
	$(CC) $^ -o $@ $(LDFLAGS)
bin/fit-and-sbs: src/sbs-utils.o src/fit-utils.o $(HISTO_DEPS) bin/fit-and-sbs.o 
	$(CC) $^ -o $@ -lRooFit -lRooFitCore $(LDFLAGS)
bin/test-err-prop: bin/test-err-prop.o src/math.o
	$(CC) $^ -o $@ $(LDFLAGS)
bin/cut-flow-plots: $(HISTO_DEPS) bin/cut-flow-plots.o
	$(CC) $^ -o $@ $(LDFLAGS)
%.o: %.cxx
	$(CC) $(CXXFLAGS) -c $< -o $@
clean:
	-rm $(BINS) $(BINOBJ) $(OBJS) src/dict.cxx src/dict.h src/libDict.so
# DO NOT DELETE
