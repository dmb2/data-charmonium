#include <iostream>
#include "simple-parser.hh"
#include "root-sugar.hh"

#include "TTree.h"
#include "TFile.h"
void usage(const char* name){
  MSG("Usage: "<<name<<" cross_sections.conf n_requested [MC Files]");
  MSG("\t cross_sections.conf contains the cross sections for each 6 digit");
  MSG("\t DSID in fb, in the form DSID=xsec on each line. [MC Files] is a ");
  MSG("\t list of files that starts with the 6 digit DSID, ie DSID.foobar.root");
}

int main(const int argc, const char* argv[]){
  if(argc < 4){
    usage(argv[0]);
    return 0;
  }
  option_map str_xsecs;
  get_opts(argv[1],str_xsecs);
  std::map<std::string,double> xsecs;
  for(option_map::const_iterator it=str_xsecs.begin(); it!=str_xsecs.end();
      ++it){
    const std::string& dsid=it->first;
    const std::string& xsec=it->second;
    xsecs[dsid]=atof(xsec.c_str());
    // MSG_DEBUG(dsid<<" -> "<<xsecs[dsid]<<" fb");
  }
  double total_xs=0.0;
  double n_requested=atof(argv[2]);
  double N_MC=0.0;
  // TFile out_file("closure_sample.root","RECREATE");
  std::map<std::string,TTree*> samples;
  for(int i=3; i < argc; i++ ){
    std::string fname(argv[i]);
    std::vector<std::string> parts = split_string(fname,"./");
    std::string dsid=parts[parts.size()-4];
    total_xs+=xsecs[dsid];
    samples[dsid]=retrieve<TTree>(fname.c_str(),"mini");
    N_MC+=samples[dsid]->GetEntries();
  }
  // out_file.cd();
  TTree* out_tree=samples.begin()->second->CloneTree(0);
  MSG_DEBUG("Total cross section: "<<total_xs<<" from "<<N_MC<<" events");
  // for(std::map<std::string,TTree*>::const_iterator it=samples.begin();
  //     it!=samples.end(); ++it){
  //   const std::string& dsid = it->first;
  //   const double& xsec = xsecs[dsid];
  //   const int nentries=xsec/total_xs*n_requested;
  //   MSG_DEBUG("Processing "<<dsid<<", copying "<<nentries<<" events.");
  //   out_tree->CopyAddresses(samples[dsid]);
  //   out_tree->CopyEntries(samples[dsid],nentries);
  //   out_tree->Fill();
  // }
  // out_file.Write();
  // out_file.Close();
  return 0;
}
