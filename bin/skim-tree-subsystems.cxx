// System
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <map>
// Local Includes
#include "simple-parser.hh"
#include "cut-flow-studies.hh"
#include "Units.hh"
#include "tree-utils.hh"
#include "root-sugar.hh"
// Root Includes
#include "TFile.h"
#include "TTree.h"
using namespace Units;
void usage(const char* prog_name){
  MSG("Usage: "<<prog_name<< " config.conf [inFile] [outfile] [xsec]");
}
template<typename T>
void reset_cut_cat(std::map<std::string, cut<T> >& CutCat){
  for(typename std::map<std::string,cut<T> >::iterator cut_pair = CutCat.begin();
      cut_pair != CutCat.end(); ++cut_pair){
    cut_pair->second.reset();
  }
}
int main(const int argc, const char* argv[]){
  if(argc != 2 && argc != 5) {
    usage(argv[0]);
    return 1;
  }
  std::string inFName;
  std::string outFName;
  real_cuts CutDefReals;
  category_cuts CutDefCats;
  double xsec(0.);
  std::map<std::string,std::string> value_opts;
  get_opts(argv[1],value_opts, CutDefReals, CutDefCats);
  MSG("Parsed opts, checking for meta info");
  for(std::map<std::string,std::string>::const_iterator opt=value_opts.begin();
      opt!=value_opts.end(); ++opt){
    //MSG_DEBUG(opt->first <<"->"<<opt->second);
    if(opt->first=="inFile"){
      inFName=opt->second;
    }
    else if(opt->first=="outFile"){
      outFName=opt->second;
    }
    else if(opt->first=="crossSection"){
      xsec = atof(opt->second.c_str());
    }
  }
  if(argc==5){
    MSG("Overriding options in config file!");
    inFName=argv[2];
    outFName=argv[3];
    xsec=atof(argv[4]);
  }
  MSG("Input Cross Section: " << xsec);
  MSG("Input File: "<<inFName);
  
  TFile* file = TFile::Open(inFName.c_str());
  tree_collection Forest; 
  const char* treeNames[] = {"AUX","LCTOPO","TOPOEM","TRACKZ","MU","JPSI",
			    "PRIVX","SEL_TRACKS", "TRIG"};
  for(size_t i=0; i < sizeof(treeNames)/sizeof(*(treeNames)); i++){
    Forest[std::string(treeNames[i])]=dynamic_cast<TTree*>(file->Get(treeNames[i]));//retrieve<TTree>(file,treeNames[i]);
  }
  if(xsec > 0){
    Forest["TRUTH"]=retrieve<TTree>(file,"TRUTH");
  }
  const double weight=xsec > 0 ? xsec/Forest["AUX"]->GetEntries() : 1.;
  const char* muon_systems[] = {"","trkMS","trkMuonExtr","trkInnerExtr","trkComb"};
  const char* jet_systems[] = {"TRACKZ","LCTOPO","TOPOEM"};
  char outName[100];
  for(size_t i=0; i < sizeof(muon_systems)/sizeof(*muon_systems); i++){
    for(size_t j=0; j < sizeof(jet_systems)/sizeof(*jet_systems); j++){
      snprintf(outName,100,("208004.%s.%s.mini.root"),muon_systems[i],jet_systems[j]);
      MSG("Opening output file: "<<outName);
      TFile OutFile(outName,"RECREATE");
      OutFile.cd();
      TTree OutTree("mini","mini");
      process_tree(Forest,CutDefReals,CutDefCats,OutTree,muon_systems[i],jet_systems[j],weight);
      print_cut_table(CutDefReals,CutDefCats);
      reset_cut_cat(CutDefReals);
      reset_cut_cat(CutDefCats);
      OutFile.Write();
      OutFile.Close();
    }
  }
  for(tree_collection::iterator it=Forest.begin(); it != Forest.end(); ++it){
    if(it->second) delete it->second;
  }
  file->Close();
  if(file) delete file;
  return 0;
}
