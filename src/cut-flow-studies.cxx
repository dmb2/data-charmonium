//System
#include <iostream>
#include <cstdlib>

//ROOT 
#include "TFile.h"
#include "TLorentzVector.h"
#include "TTree.h"

//Local
#include "cut-flow-studies.hh"
#include "Units.hh"

using namespace Units;
using std::cout;
using std::cerr;
using std::endl;


int process_tree(tree_collection Forest, real_cuts CutDefReal, 
		 category_cuts CutDefCat){
  bool verbose=true;
  CutDefCat["Nominal"].pass();
  unsigned int squawk_every = 1000;
  double z(0.), DeltaR(0.);
  double jpsi_pt(0.), jpsi_eta(0.), jpsi_phi(0.), jpsi_E(0.);

  std::vector<double>* jet_pt=NULL;
  std::vector<double>* jet_eta=NULL;
  std::vector<double>* jet_phi=NULL;
  std::vector<double>* jet_E=NULL;

  double t_z(0.), t_DeltaR(0.);
  double t_jpsi_pt(0.), t_jpsi_eta(0.),t_jpsi_phi(0.), t_jpsi_E(0.);
  std::vector<double>* t_jet_pt=NULL;
  std::vector<double>* t_jet_eta=NULL;
  std::vector<double>* t_jet_phi=NULL;
  std::vector<double>* t_jet_E=NULL;

  for(tree_collection::iterator it=Forest.begin(); it!=Forest.end(); ++it){
    if(verbose){
      cout << "Processing " << it->first <<" at address "<<it->second <<endl;
    }
    it->second->SetCacheSize(10*MB);
    it->second->SetCacheLearnEntries(5);
  }


  Forest["AUX"]->SetBranchAddress("jpsi_e",&jpsi_E);
  Forest["AUX"]->SetBranchAddress("jpsi_pt",&jpsi_pt);
  Forest["AUX"]->SetBranchAddress("jpsi_eta",&jpsi_eta);
  Forest["AUX"]->SetBranchAddress("jpsi_phi",&jpsi_phi);

  Forest["AUX"]->SetBranchAddress("truth_jpsi_e",&t_jpsi_E);
  Forest["AUX"]->SetBranchAddress("truth_jpsi_pt",&t_jpsi_pt);
  Forest["AUX"]->SetBranchAddress("truth_jpsi_eta",&t_jpsi_eta);
  Forest["AUX"]->SetBranchAddress("truth_jpsi_phi",&t_jpsi_phi);

  Forest["JET"]->SetBranchAddress("JET_e",&jet_E);
  Forest["JET"]->SetBranchAddress("JET_pt",&jet_pt);
  Forest["JET"]->SetBranchAddress("JET_eta",&jet_eta);
  Forest["JET"]->SetBranchAddress("JET_phi",&jet_phi);

  Forest["TRUTH_JET"]->SetBranchAddress("JET_e",&t_jet_E);
  Forest["TRUTH_JET"]->SetBranchAddress("JET_pt",&t_jet_pt);
  Forest["TRUTH_JET"]->SetBranchAddress("JET_eta",&t_jet_eta);
  Forest["TRUTH_JET"]->SetBranchAddress("JET_phi",&t_jet_phi);
  Long64_t nEntries = Forest["AUX"]->GetEntries();
  cout<<"Got "<<nEntries<< " in input tree"<<endl;
  for(Long64_t i=0; i < nEntries; i++){
    for(tree_collection::iterator it=Forest.begin(); it != Forest.end(); ++it){
      it->second->GetEntry(i);
    }

    if(i%squawk_every==0 && verbose){
      cout <<"Processing record "<<i<<endl;
    }
    
  }
  return 0;
}
