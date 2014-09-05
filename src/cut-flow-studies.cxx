//System
#include <iostream>
#include <cstdlib>
#include <cstdio>
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

template<typename T>
void setup_four_vector(TTree*& tree, T& pt, T& eta, T& phi, T& E,const char* key){
  char branch_name[50];
  snprintf(branch_name, 50,"%s_pt",key);
  tree->SetBranchAddress(branch_name, &pt);
  snprintf(branch_name, 50,"%s_eta",key);
  tree->SetBranchAddress(branch_name, &eta);
  snprintf(branch_name, 50,"%s_phi",key);
  tree->SetBranchAddress(branch_name, &phi);
  snprintf(branch_name, 50,"%s_e",key);
  tree->SetBranchAddress(branch_name, &E);
}

int process_tree(tree_collection& Forest, real_cuts& CutDefReal, 
		 category_cuts& CutDefCat){
  unsigned int squawk_every = 1000;
  double z(0.), DeltaR(999.), tmp_dR(0.);
  double jpsi_pt(0.), jpsi_eta(0.), jpsi_phi(0.), jpsi_E(0.);
  std::vector<double> *jet_pt=NULL, *jet_eta=NULL, *jet_phi=NULL, *jet_E=NULL;

  //double t_z(0.), t_DeltaR(0.);
  double t_jpsi_pt(0.), t_jpsi_eta(0.),t_jpsi_phi(0.), t_jpsi_E(0.);
  std::vector<double> *t_jet_pt=NULL, *t_jet_eta=NULL, *t_jet_phi=NULL, *t_jet_E=NULL;

  setup_four_vector(Forest["AUX"], jpsi_pt, jpsi_eta, jpsi_phi, jpsi_E, "jpsi");
  setup_four_vector(Forest["AUX"], t_jpsi_pt, t_jpsi_eta, t_jpsi_phi, t_jpsi_E, "truth_jpsi");
  setup_four_vector(Forest["JET"], jet_pt, jet_eta, jet_phi, jet_E, "JET");
  setup_four_vector(Forest["TRUTH_JET"], t_jet_pt, t_jet_eta, t_jet_phi, t_jet_E, "JET");
  
  int has_num_jets=0, has_jpsi_pt=0, has_jpsi_eta=0, has_jet_eta=0, has_delta_r=0, has_jet_pt=0;

  Long64_t nEntries = Forest["AUX"]->GetEntries();
  if(verbose) {
    cout<<"Got "<<nEntries<< " in input tree"<<endl;
  }
  TLorentzVector candJet(0,0,0,0);
  TLorentzVector tmpJet(0,0,0,0);
  TLorentzVector candJPsi(0,0,0,0);
  for(Long64_t entry=0; entry < nEntries; entry++){
    retrieve_values(Forest,entry);
    if(entry%squawk_every==0 && verbose){
      cout <<"Processing entry "<<entry<<endl;
    }
    DeltaR=9999.;
    tmp_dR=0;
    tmpJet.Clear();
    z=9999.;

    int jet_mult(jet_pt->size());
    CutDefCat["Nominal"].pass();
    if(!pass_cut(greater_than_eq, jet_mult, CutDefCat["NumJets"])){
      continue;
    }
    if(!pass_cut(greater_than, jpsi_pt*GeV, CutDefReal["JPsiPt"])){
      continue;
    }
    if(!pass_cut(less_than, fabs(jpsi_eta),CutDefReal["JPsiEta"])){
      continue;
    }
    candJPsi.SetPtEtaPhiE(jpsi_pt*GeV, jpsi_eta, 
			  jpsi_phi, jpsi_E*GeV);
    for(int j=0; j < jet_mult; j++){
      tmpJet.SetPtEtaPhiE(jet_pt->at(j)*GeV, jet_eta->at(j), 
			  jet_phi->at(j), jet_E->at(j)*GeV);
      tmp_dR=candJPsi.DeltaR(tmpJet);
      if(tmp_dR < DeltaR){
	DeltaR=tmp_dR;
	candJet=tmpJet;
      }
    }
    if(!pass_cut(less_than, DeltaR, CutDefReal["DeltaR"])){
      continue;
    }
    if(!pass_cut(less_than, candJet.Eta(), CutDefReal["JetEta"])){
      continue;
    }
    if(!pass_cut(greater_than, candJet.Pt(), CutDefReal["JetPt"])){
      continue;
    }

    z=jpsi_pt/(candJet.Pt()+jpsi_pt);
    if(z){};
  }
  return 0;
}
