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

.PHONY: all clean 
all: skim-tree skim-truth-tree cut-flow-plots simple-parser-test

tree-bug: ./bin/tree-bug.cxx
	$(CC) $(CXXFLAGS) $? -o $@ $(LDFLAGS)
simple-parser-test: simple-parser-test.o simple-parser.o
	$(CC) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
simple-parser-test.o: ./bin/simple-parser-test.cxx
	$(CC) $(CXXFLAGS) -c $< -o $@
skim-tree: cut-flow-studies.o skim-tree.o 
	$(CC) $^ -o $@ $(LDFLAGS) 
skim-truth-tree: truth-studies.o skim-truth-tree.o 
	$(CC) $^ -o $@ $(LDFLAGS) 
skim-truth-tree.o: ./bin/skim-truth-tree.cxx
	$(CC) $(CXXFLAGS) -c $< -o $@
skim-tree.o: ./bin/skim-tree.cxx
	$(CC) $(CXXFLAGS) -c $< -o $@
cut-flow-plots: AtlasStyle.o cut-flow-plots.o 
	$(CC) AtlasStyle.o cut-flow-plots.o  -o $@ $(LDFLAGS) 
cut-flow-plots.o: ./bin/cut-flow-plots.cxx 
	$(CC) $(CXXFLAGS) -c $< -o $@
%.o: ./src/%.cxx
	$(CC) $(CXXFLAGS) -c $< -o $@
clean:
	-rm *.o skim-tree
