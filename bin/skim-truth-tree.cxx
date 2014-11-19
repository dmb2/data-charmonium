// System
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <map>
// Local Includes
#include "truth-studies.hh"
#include "simple-parser.hh"
#include "root-sugar.hh"
#include "Units.hh"
// Root Includes
#include "TFile.h"
#include "TTree.h"

using namespace Units;
using std::cout;
using std::endl;
void usage(const char* name){
  cout <<"Usage: "<< name << " config.conf"<<endl;
}
int main(const int argc, const char* argv[]){
  if(argc != 2) {
    usage(argv[0]);
    return 1;
  }
  std::string inFName;
  std::string outFName;
  real_cuts CutDefReals;
  category_cuts CutDefCats;
  double target_lumi(0.);
  double xsec(0.);
  std::map<std::string,std::string> value_opts;
  get_opts(argv[1],value_opts, CutDefReals, CutDefCats);
  for(std::map<std::string,std::string>::const_iterator opt=value_opts.begin();
      opt!=value_opts.end(); ++opt){
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
  MSG("Target Lumi: "<<target_lumi);
  MSG("Input Cross Section:" << xsec);
  TTree* tree = retrieve<TTree>(inFName.c_str(),"truth");
  TFile OutFile(outFName.c_str(),"RECREATE");
  OutFile.cd();
  TTree OutTree("mini","mini");

  const double weight=(xsec*target_lumi)/tree->GetEntries();
  process_tree(*tree,CutDefReals,CutDefCats,OutTree,weight);
  print_cut_table(CutDefReals,CutDefCats);
  if(tree){ 
    delete tree;
  }
  OutFile.Write();
  OutFile.Close();
  return 0;
}
