CC=$(shell root-config --cxx)

INCDIR=$(PWD)/include
LIBDIR:=$(shell root-config --libdir)
ROOTSYS:=$(shell root-config --prefix)
ROOTINCDIR:=$(shell root-config --incdir)
LDFLAGS:=$(shell root-config --libs)
WFLAGS= -Wextra -Wall 
DFLAGS=-O0 -g3 -fno-inline #-fprofile-arcs -ftest-coverage 
CXXFLAGS:=$(shell root-config --cflags)\
	-pg -I$(INCDIR)\
	$(DFLAGS) $(WFLAGS) 

BINS:=$(patsubst %.cxx,%,$(wildcard bin/*.cxx)) bin/skim-tree-response
BINOBJ:=$(patsubst %.cxx,%.o,$(wildcard bin/*.cxx)) bin/skim-tree-response.o

COMMON_DEPS:=src/root-sugar.o

SKIM_DEPS:=$(COMMON_DEPS) src/tree-utils.o src/simple-parser.o src/Cut.o 

HISTO_DEPS:=$(COMMON_DEPS) src/stack-utils.o src/AtlasStyle.o\
	src/histo-meta-data.o src/histo-utils.o\
	src/plot-utils.o src/color.o src/math.o\
	src/histo-style.o 
.PHONY: all clean install-plots install-roounfold
all: $(BINS)

install-plots: 
	mv *corr.pdf ~/Documents/JPsiJetSubstructure/plots/correlation/
	mv *_sbs_p8.pdf ~/Documents/JPsiJetSubstructure/plots/
	mv *_splot.pdf ~/Documents/JPsiJetSubstructure/plots/
	mv *_bkg.pdf ~/Documents/JPsiJetSubstructure/plots/splot-bkg/
install-roounfold:
	svn co https://svnsrv.desy.de/public/unfolding/RooUnfold/trunk RooUnfold
	@$(MAKE) -C RooUnfold
	cp RooUnfold/src/*.h ./include
	cp RooUnfold/*RooUnfold* ./src/
src/dict.cxx: $(INCDIR)/LinkDef.h
	rootcint -f $@ -c -I$(INCDIR) -p $^
src/libDict.so: src/dict.cxx
	$(CC) -shared -fPIC -o$@ $(CXXFLAGS) $^
src/analyze-cut-tree.o: src/analyze-tree.cxx
	$(CC) $(CXXFLAGS) -D__ANALYZE_TREE_CUTFLOW__ -c $< -o $@ 
bin/skim-tree:  $(SKIM_DEPS) bin/skim-tree.o src/analyze-tree.o src/libDict.so
	$(CC) $^ -o $@ $(LDFLAGS) -L ./src -l Dict
bin/skim-tree-response: $(SKIM_DEPS) bin/skim-tree.o src/analyze-cut-tree.o src/libDict.so
	$(CC) $^ -o $@ $(LDFLAGS) -L ./src -l Dict
# TODO these should all get their own rule
bin/fit-and-sbs: src/sbs-utils.o src/fit-utils.o $(HISTO_DEPS)  bin/fit-and-sbs.o 
	$(CC) $^ -o $@ -lRooFit -lRooFitCore $(LDFLAGS) -lRooStats
bin/fit: src/sbs-utils.o src/fit-utils.o $(HISTO_DEPS) src/tree-utils.o bin/fit.o 
	$(CC) $^ -o $@ -lRooFit -lRooFitCore $(LDFLAGS) -lRooStats
bin/sbs: src/sbs-utils.o src/fit-utils.o $(HISTO_DEPS) bin/sbs.o 
	$(CC) $^ -o $@ -lRooFit -lRooFitCore $(LDFLAGS) -lRooStats
bin/splot: src/sbs-utils.o src/fit-utils.o $(HISTO_DEPS) src/tree-utils.o bin/splot.o 
	$(CC) $^ -o $@ -lRooFit -lRooFitCore $(LDFLAGS) -lRooStats
bin/unfold: src/unfolding-utils.o $(HISTO_DEPS) src/tree-utils.o bin/unfold.o 
	$(CC) $^ -o $@ $(LDFLAGS) -lRooUnfold -L./RooUnfold
bin/unfolding-closure: src/unfolding-utils.o $(HISTO_DEPS) src/tree-utils.o bin/unfolding-closure.o 
	$(CC) $^ -o $@ $(LDFLAGS) -lRooUnfold -L./RooUnfold -lRooFit -lRooFitCore
bin/test-err-prop: bin/test-err-prop.o src/math.o
	$(CC) $^ -o $@ $(LDFLAGS)
bin/make-closure-sample: $(COMMON_DEPS) bin/make-closure-sample.o src/simple-parser.o src/Cut.o
bin/simple-parser-test: bin/simple-parser-test.o src/simple-parser.o src/Cut.o
bin/%: $(HISTO_DEPS) bin/%.o
	$(CC) $^ -o $@ $(LDFLAGS)
%.o: %.cxx
	$(CC) $(CXXFLAGS) -c $< -o $@
clean:
	-rm $(BINS) $(wildcard bin/*.o) $(wildcard src/*.o) src/dict.cxx src/dict_rdict.pcm src/libDict.so
# DO NOT DELETE
