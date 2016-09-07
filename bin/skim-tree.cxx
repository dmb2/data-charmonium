// System
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <map>
// Local Includes
#include "simple-parser.hh"
#include "analyze-tree.hh"
#include "Units.hh"
#include "tree-utils.hh"
#include "root-sugar.hh"
// Root Includes
#include "TFile.h"
#include "TTree.h"
using namespace Units;
void usage(const char* prog_name){
  MSG("Usage: "<<prog_name<< " -c config.conf -i inFile -o outfile -x xsec");
  MSG("\t -i, -o, and -x options are optional if specified in config.conf");
}
template<typename T>
void reset_cut_cat(std::map<std::string, cut<T> >& CutCat){
  for(typename std::map<std::string,cut<T> >::iterator cut_pair = CutCat.begin();
      cut_pair != CutCat.end(); ++cut_pair){
    cut_pair->second.reset();
  }
}
void process(const char* outName, tree_collection& Forest,
	     real_cuts& CutDefReals, category_cuts& CutDefCats,
	     const char* mu_system, const char* jet_system,
	     double weight){
    MSG("Opening output file: "<<outName);
    TFile OutFile(outName,"RECREATE");
    OutFile.cd();
    TTree OutTree("mini","mini");
    process_tree(Forest,CutDefReals,CutDefCats,OutTree,mu_system,jet_system,weight);
    print_cut_table(CutDefReals,CutDefCats);
    reset_cut_cat(CutDefReals);
    reset_cut_cat(CutDefCats);
    OutFile.Write();
    OutFile.Close();
}

int main(const int argc, char* const argv[]){
  // if(argc != 2 && argc != 5) {
  //   usage(argv[0]);
  //   return 1;
  // }
  char* cli_in_fname=NULL;
  char* cli_out_fname=NULL;
  char* cli_xsec=NULL;
  char* conf_name=NULL;
  int c;
  while((c = getopt(argc,argv,"i:o:x:c:")) != -1){
    switch(c){
    case 'i':
      cli_in_fname = optarg;
      break;
    case 'o':
      cli_out_fname = optarg;
      break;
    case 'x':
      cli_xsec = optarg;
      break;
    case 'c':
      conf_name = optarg;
      break;
    default:
      abort();
    }
  }
  if(conf_name==NULL){
    usage(argv[0]);
    exit(1);
  }
  std::string inFName;
  std::string outFName;
  bool runSystematics (false);
  real_cuts CutDefReals;
  category_cuts CutDefCats;
  double xsec(0.);
  std::map<std::string,std::string> value_opts;
  get_opts(conf_name,value_opts, CutDefReals, CutDefCats);
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
    else if (opt->first=="doSystematics"){
      runSystematics = (opt->second == "True" || opt->second == "true");
    }
  }
  if(cli_in_fname!=NULL){
    MSG("Overriding input file name: "<<cli_in_fname);
    inFName=cli_in_fname;
  }
  if(cli_out_fname!=NULL){
    MSG("Overriding output file name: "<<cli_out_fname);
    outFName=cli_out_fname;
  }
  if(cli_xsec!=NULL){
    MSG("Overriding cross section: "<<cli_xsec);
    xsec=atof(cli_xsec);
  }
  if(!std::isfinite(xsec) ||
     outFName=="" ||
     inFName==""){
    MSG_ERR("Something went wrong parsing options.");
    exit(1);
  }
  
  MSG("Input File: "<<inFName);
  TFile* file = TFile::Open(inFName.c_str());
  tree_collection Forest; 
  const char* jet_variations[] = {"TrackZFilteredJPsiJets","TrackZSmearedJPsiJets",
				  "TrackZScaledUpJPsiJets", "TrackZScaledDownJPsiJets",
				  "TrackZRadialScaledUpJPsiJets", "TrackZRadialScaledDownJPsiJets"};
  const char* treeNames[] = {"AUX","LCTopoJets", "TrackZJPsiJets",
			     "Mu", "JPsi", "FakeJPsi", "TRIG"};
  for(size_t i=0; i < LEN(treeNames); i++){
    Forest[std::string(treeNames[i])]=retrieve<TTree>(file,treeNames[i]);
  }
  if(runSystematics){
    for(size_t i=0; i < LEN(jet_variations); i++){
      Forest[std::string(jet_variations[i])]=retrieve<TTree>(file,jet_variations[i]);
    }
  }
  if(xsec > 0){
    Forest["TruthJets"]=retrieve<TTree>(file,"TruthJets");
    Forest["JPsiTruthJets"]=retrieve<TTree>(file,"JPsiTruthJets");
    // Forest["MuonTruthJets"]=retrieve<TTree>(file,"MuonTruthJets");
  }
  const double weight=xsec > 0 ? xsec/Forest["AUX"]->GetEntries() : fabs(xsec);
  // const char* muon_systems[] = {"","trkMS","trkMuonExtr","trkInnerExtr","trkComb"};
  const char* muon_variations[] = {"Smeared","SmearedLow","SmearedUp","SmearedIDUp","SmearedMSUp","EfficiencyUp","EfficiencyDown"};
  char outName[100];
  
  //try to extract dsid
  std::vector<std::string> parts = split_string(outFName,'.');
  parts = split_string(parts[0],'/');
  std::string dsid = parts.size()!=0 ? parts.back() : "";
  if (runSystematics){
    // nominal
    // process(outFName.c_str(),Forest,CutDefReals, CutDefCats, "","TrackZJPsiJets",weight);
    // muon variations
    for(size_t i=0; i < LEN(muon_variations); i++){
      snprintf(outName,100,("%s.Muon%s.mini.root"), dsid.c_str(), muon_variations[i]);
      process(outName,Forest,CutDefReals, CutDefCats, muon_variations[i],"TrackZJPsiJets",weight);
    }
    // jet variations
    for(size_t j=0; j < LEN(jet_variations); j++){
      snprintf(outName,100,("%s.%s.mini.root"), dsid.c_str(), jet_variations[j]);
      process(outName,Forest,CutDefReals, CutDefCats, "",jet_variations[j],weight);
    }
  }
  else{
    process(outFName.c_str(),Forest,CutDefReals, CutDefCats, "","TrackZJPsiJets",weight);
  }
  
  for(tree_collection::iterator it=Forest.begin(); it != Forest.end(); ++it){
    delete it->second;
  }
  file->Close();
  if(file) delete file;
  return 0;
}
