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
bool verbose=true;

void retrieve_values(tree_collection& forest, Long64_t entry){
  for(tree_collection::iterator it=forest.begin(); it != forest.end(); ++it){
    it->second->GetEntry(entry);
  }
}
void initialize_forest(tree_collection& forest){
  for(tree_collection::iterator it=forest.begin(); it!=forest.end(); ++it){
    if(verbose){
      cout << "Processing " << it->first <<" at address "<<it->second <<endl;
    }
    it->second->SetCacheSize(10*MB);
    it->second->SetCacheLearnEntries(5);
  }
}

int process_tree(tree_collection& Forest, real_cuts& CutDefReal, 
		 category_cuts& CutDefCat){
  unsigned int squawk_every = 1000;
  double z(0.), DeltaR(0.);
  double jpsi_pt(0.), jpsi_eta(0.), jpsi_phi(0.), jpsi_E(0.);

  std::vector<double>* jet_pt=NULL;
  std::vector<double>* jet_eta=NULL;
  std::vector<double>* jet_phi=NULL;
  std::vector<double>* jet_E=NULL;

  //double t_z(0.), t_DeltaR(0.);
  double t_jpsi_pt(0.), t_jpsi_eta(0.),t_jpsi_phi(0.), t_jpsi_E(0.);
  std::vector<double>* t_jet_pt=NULL;
  std::vector<double>* t_jet_eta=NULL;
  std::vector<double>* t_jet_phi=NULL;
  std::vector<double>* t_jet_E=NULL;



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
  if(verbose) {
    cout<<"Got "<<nEntries<< " in input tree"<<endl;
  }
  TLorentzVector leadJet(0,0,0,0);
  TLorentzVector leadJPsi(0,0,0,0);
  for(Long64_t i=0; i < nEntries; i++){
    CutDefCat["Nominal"].pass();
    if(i%squawk_every==0 && verbose){
      cout <<"Processing record "<<i<<endl;
    }
    retrieve_values(Forest,i);
    if(int(jet_pt->size()) <  CutDefCat["NumJets"]){
      continue;
    }
    CutDefCat["NumJets"].pass();

    if(jpsi_pt*GeV < CutDefReal["JPsiPt"]){
      continue;
    }
    CutDefReal["JPsiPt"].pass();

    if(fabs(jpsi_eta) > CutDefReal["JPsiEta"]){
      continue;
    }
    CutDefReal["JPsiEta"].pass();

    if(fabs(jet_pt->at(0)) < CutDefReal["JetPt"]){
      continue;
    }
    CutDefReal["JetPt"].pass();

    if(fabs(jet_eta->at(0)) > CutDefReal["JetEta"]){
      continue;
    }
    CutDefReal["JetEta"].pass();
    
    leadJet.SetPtEtaPhiE(jet_pt->at(0),
			jet_eta->at(0),
			jet_phi->at(0),
			jet_E->at(0));
    leadJPsi.SetPtEtaPhiE(jpsi_pt,
			 jpsi_eta,
			 jpsi_phi,
			 jpsi_E);

  }
  return 0;
}
void print_cut_summary(std::string CutName, cut<int> Cut){
  printf("| %-8s | %8d | %6d | \n",CutName.c_str(),Cut.cut_value(),Cut.count());
}
void print_cut_summary(std::string CutName, cut<double> Cut){
  printf("| %-8s | %.2e | %6d | \n",CutName.c_str(),Cut.cut_value(),Cut.count());
}
