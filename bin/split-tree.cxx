#include <libgen.h>
#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include "root-sugar.hh"

void usage(const char* name){
  MSG("Usage: "<< name << "-i input.root -t tree_name [cut_name:\"cut expr\"]");
}


int main(const int argc, char* const argv[]){
  char* inFName=NULL;
  char* tree_name=NULL;
  int c;
  while((c = getopt(argc,argv,"i:t:"))!= -1){
    switch(c){
    case 'i':
      inFName=optarg;
      break;
    case 't':
      tree_name=optarg;
      break;
    default:
      abort();
    }
  }
  if(inFName==NULL || tree_name==NULL ){
    usage(argv[0]);
    exit(1);
  }
  
  std::map<std::string,std::string> cut_regions;
  std::vector<std::string> parts;
  for(int i=optind; i < argc; i++){
    printf("%s\n",argv[i]);
    parts=split_string(argv[i],':');
    //MSG_DEBUG("Cut Name:"<< parts.at(0)<<" Cut Expr:"<<parts.at(1));
    cut_regions[parts.at(0)]=parts.at(1);
  }
  //double loop over bins, build cut_string and filename using
  //input.root as a basename.
  TTree* tree = retrieve<TTree>(inFName,tree_name);
  std::string base_name(split_string(inFName,'.').at(0));
  MSG_DEBUG(base_name);
  char outFName[256];
  // char cutstring[256];
  /*
  cut_regions["mass_sb_1"] = "(2.17 < jpsi_m && jpsi_m < 2.841) && (-1 < jpsi_tau && jpsi_tau < 0.25)";
  cut_regions["mass_sb_2"] = "(3.345 < jpsi_m && jpsi_m < 3.764) && (-1 < jpsi_tau && jpsi_tau < 0.25)";
  cut_regions["tau_sb"] = "(2.841 < jpsi_m && jpsi_m < 3.345) && (jpsi_tau > 0.25)";
  cut_regions["signal_region"] = "(2.841 < jpsi_m && jpsi_m < 3.345) && (-1 < jpsi_tau && jpsi_tau < 0.25)";

  cut_regions["mu36_tight"]="trigger_category == 1";
  cut_regions["mu24i_tight"]="trigger_category == 2";
  */
  for(std::map<std::string,std::string>::const_iterator it=cut_regions.begin(); it!=cut_regions.end(); ++it){
    const std::string& name = it->first;
    const std::string& cut_expr = it->second;
    MSG_DEBUG("Processing: "<<name<< " cut_expr: "<<cut_expr);
    snprintf(outFName,LEN(outFName),"%s.%s.%s.root",base_name.c_str(),name.c_str(),tree_name);
    MSG_DEBUG("Writing to: "<<outFName);
    MSG_DEBUG("Reading from tree: "<<tree);
    MSG_DEBUG("Selection string: "<<cut_expr);
    TFile* outFile = new TFile(outFName,"RECREATE");
    outFile->cd();
    TTree* outTree = tree->CopyTree(cut_expr.c_str());
    outTree->Write();
    outFile->Write();
    outFile->Close();
    delete outFile;
  }
  return 0;
}
