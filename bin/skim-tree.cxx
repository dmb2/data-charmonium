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
#include "root-sugar.hh"
// Root Includes
#include "TFile.h"
#include "TTree.h"
using namespace Units;
void usage(const char* prog_name){
  MSG("Usage: "<<prog_name<< " config.conf [inFile] [outfile] [xsec]");
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
  double target_lumi(22.1);
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
    else if(opt->first=="targetLumi"){
      target_lumi = atof(opt->second.c_str());
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
  MSG("Target Lumi: "<<target_lumi);
  MSG("Input Cross Section: " << xsec);
  MSG("Input File: "<<inFName);
  
  TFile* file = new TFile(inFName.c_str());
  tree_collection Forest; 
  const char* treeNames[] = {"AUX","JET","MU","JPSI",
			    "PRIVX","SEL_TRACKS",
			    "TRIG","TRUTH_JET"};
  for(size_t i=0; i < sizeof(treeNames)/sizeof(*(treeNames)); i++){
    Forest[std::string(treeNames[i])]=retrieve<TTree>(file,treeNames[i]);
  }
  MSG("Opening output file: "<<outFName);
  TFile OutFile(outFName.c_str(),"RECREATE");

  OutFile.cd();
  TTree OutTree("mini","mini");
  const double weight=(xsec*target_lumi)/Forest["AUX"]->GetEntries();
  process_tree(Forest,CutDefReals,CutDefCats,OutTree,weight);
  // const char* CutNames[]={ "nominal", "trigger","num_jets", "jpsi_pt", "jpsi_eta",
  // 			   "jet_eta","delta_r","jet_pt"}; 
  print_cut_table(CutDefReals,CutDefCats);
  for(tree_collection::iterator it=Forest.begin(); it != Forest.end(); ++it){
    if(it->second) delete it->second;
  }
  file->Close();
  if(file) delete file;
  OutFile.Write();
  OutFile.Close();
  return 0;
}
