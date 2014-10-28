CC=$(shell root-config --cxx)
# Build this library the same way root was compiled/linked
INCDIR=$(PWD)/include
LIBDIR:=$(shell root-config --libdir)
ROOTINCDIR:=$(shell root-config --incdir)
LDFLAGS:=$(shell root-config --libs) #-L ./lib #-lgcov
WFLAGS= -Wextra -Wall 
DFLAGS=-O3 #-fprofile-arcs -ftest-coverage 
CXXFLAGS=$(shell root-config --ldflags) -pg -I$(INCDIR) -I$(ROOTINCDIR)	\
$(DFLAGS) $(WFLAGS) -ansi

BINSRC:=$(wildcard bin/*.cxx)
BINS:=skim-tree skim-truth-tree cut-flow-plots simple-parser-test
BINOBJ:=$(BINSRC:.cxx=.o)

.PHONY: all clean 
all: $(BINS)

debug:
	@echo $(BINSRC)
	@echo $(BINS)
	@echo $(BINOBJ)
	@echo $(CXXFLAGS)
simple-parser-test: bin/simple-parser-test.o AtlasStyle.o simple-parser.o
	$(CC) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
skim-tree: cut-flow-studies.o tree-utils.o AtlasStyle.o bin/skim-tree.o 
	$(CC) $^ -o $@ $(LDFLAGS) 
skim-truth-tree: truth-studies.o tree-utils.o AtlasStyle.o bin/skim-truth-tree.o 
	$(CC) $^ -o $@ $(LDFLAGS) 

$(BINOBJ): $(BINSRC) 
	$(CC) $(CXXFLAGS) -c $<  -o $@
cut-flow-plots: AtlasStyle.o bin/cut-flow-plots.o 
	$(CC) $^  -o $@ $(LDFLAGS) 
%.o: ./src/%.cxx
	$(CC) $(CXXFLAGS) -c $< -o $@
clean:
	-rm *.o $(BINS) bin/*.o 
