CC=$(shell root-config --cxx)

INCDIR=$(PWD)/include
LIBDIR:=$(shell root-config --libdir)
ROOTINCDIR:=$(shell root-config --incdir)
LDFLAGS:=$(shell root-config --libs)  $(shell fastjet-config --libs) -lNsubjettiness #-L ./lib #-lgcov
WFLAGS= -Wextra -Wall 
DFLAGS=-O3 #-fprofile-arcs -ftest-coverage 
CXXFLAGS= $(shell root-config --ldflags)  -pg -I$(INCDIR) -I$(ROOTINCDIR)	$(shell fastjet-config --cxxflags)\
$(DFLAGS) $(WFLAGS) -ansi

BINS:=skim-tree skim-truth-tree cut-flow-plots truth-study-plots simple-parser-test make-stack-plots make-plots fit-and-sbs

SKIM_DEPS:=src/tree-utils.o src/simple-parser.o src/Cut.o 
HISTO_DEPS:=src/histo-utils.o src/AtlasStyle.o src/histo-meta-data.o src/stack-utils.o src/plot-utils.o
.PHONY: all clean 
all: $(BINS)
debug:
	@echo $(LDFLAGS)
# KISS
simple-parser-test: bin/simple-parser-test.o src/simple-parser.o
	$(CC) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
skim-tree:  $(SKIM_DEPS) bin/skim-tree.o src/cut-flow-studies.o
	$(CC) $^ -o $@ $(LDFLAGS) 
skim-truth-tree:  $(SKIM_DEPS) bin/skim-truth-tree.o src/truth-studies.o
	$(CC) $^ -o $@ $(LDFLAGS) 
cut-flow-plots: bin/cut-flow-plots.o $(HISTO_DEPS)
	$(CC) $^ -o $@ $(LDFLAGS) 
truth-study-plots: $(HISTO_DEPS) bin/truth-study-plots.o 
	$(CC) $^ -o $@ $(LDFLAGS) 
make-stack-plots: $(HISTO_DEPS) bin/make-stack-plots.o 
	$(CC) $^ -o $@ $(LDFLAGS) 
make-plots: $(HISTO_DEPS) bin/make-plots.o
	$(CC) $^ -o $@ $(LDFLAGS)
fit-and-sbs: src/fit-utils.o src/sbs-utils.o bin/fit-and-sbs.o 
	$(CC) $^ -o $@ -lRooFit -lRooFitCore $(LDFLAGS) 
%.o: %.cxx
	$(CC) $(CXXFLAGS) -c $< -o $@
clean:
	-rm $(BINS) bin/*.o src/*.o
