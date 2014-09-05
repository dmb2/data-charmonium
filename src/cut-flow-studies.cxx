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
template<typename T>
void setup_four_vector_output(TTree& tree, T& pt, T& eta, T& phi, T& E,const char* key){
  char branch_name[50];
  snprintf(branch_name, 50,"%s_pt",key);
  tree.Branch(branch_name, &pt);
  snprintf(branch_name, 50,"%s_eta",key);
  tree.Branch(branch_name, &eta);
  snprintf(branch_name, 50,"%s_phi",key);
  tree.Branch(branch_name, &phi);
  snprintf(branch_name, 50,"%s_e",key);
  tree.Branch(branch_name, &E);
}

int process_tree(tree_collection& Forest, real_cuts& CutDefReal, 
		 category_cuts& CutDefCat, TTree& OutTree){
  unsigned int squawk_every = 1000;
  double z(0.), DeltaR(999.), tmp_dR(0.);
  double jpsi_pt(0.), jpsi_eta(0.), jpsi_phi(0.), jpsi_E(0.);
  std::vector<double> *jet_pt=NULL, *jet_eta=NULL, *jet_phi=NULL, *jet_E=NULL;

  double t_z(0.), t_DeltaR(0.);
  double t_jpsi_pt(0.), t_jpsi_eta(0.),t_jpsi_phi(0.), t_jpsi_E(0.);
  std::vector<double> *t_jet_pt=NULL, *t_jet_eta=NULL, *t_jet_phi=NULL, *t_jet_E=NULL;

  setup_four_vector(Forest["AUX"], jpsi_pt, jpsi_eta, jpsi_phi, jpsi_E, "jpsi");
  setup_four_vector(Forest["AUX"], t_jpsi_pt, t_jpsi_eta, t_jpsi_phi, t_jpsi_E, "truth_jpsi");
  setup_four_vector(Forest["JET"], jet_pt, jet_eta, jet_phi, jet_E, "JET");
  setup_four_vector(Forest["TRUTH_JET"], t_jet_pt, t_jet_eta, t_jet_phi, t_jet_E, "JET");

  double cand_jet_pt(0.), cand_jet_eta(0.),cand_jet_phi(0.), cand_jet_E(0.);
  double cand_t_jet_pt(0.), cand_t_jet_eta(0.), cand_t_jet_phi(0.), cand_t_jet_E(0.);

  setup_four_vector_output(OutTree,cand_jet_pt, cand_jet_eta, 
			   cand_jet_phi, cand_jet_E, "jet");
  setup_four_vector_output(OutTree,cand_t_jet_pt, cand_t_jet_eta, 
			   cand_t_jet_phi, cand_t_jet_E, "truth_jet");

  setup_four_vector_output(OutTree,t_jpsi_pt, t_jpsi_eta, t_jpsi_phi, t_jpsi_E, "truth_jpsi");
  setup_four_vector_output(OutTree,jpsi_pt, jpsi_eta, jpsi_phi, jpsi_E, "jpsi");

  OutTree.Branch("jet_z",&z);
  OutTree.Branch("delta_r",&DeltaR);
  OutTree.Branch("truth_jet_z",&t_z);
  OutTree.Branch("truth_delta_r",&t_DeltaR);

  int has_num_jets=0, has_jpsi_pt=0, has_jpsi_eta=0, has_jet_eta=0, has_delta_r=0, has_jet_pt=0;
  OutTree.Branch("num_jets_p",&has_num_jets);
  OutTree.Branch("jpsi_pt_p",&has_jpsi_pt);
  OutTree.Branch("jpsi_eta_p",&has_jpsi_eta);
  OutTree.Branch("delta_r_p", &has_delta_r);
  OutTree.Branch("jet_eta_p",&has_jet_eta);
  OutTree.Branch("jet_pt_p",&has_jet_pt);

  Long64_t nEntries = Forest["AUX"]->GetEntries();
  if(verbose) {
    cout<<"Got "<<nEntries<< " in input tree"<<endl;
  }
  TLorentzVector candJet(0,0,0,0);
  TLorentzVector candTruthJet(0,0,0,0);
  TLorentzVector tmpTruthJet(0,0,0,0);
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
    has_num_jets=pass_cut(greater_than_eq, jet_mult, CutDefCat["NumJets"]);
    has_jpsi_pt=pass_cut(greater_than, jpsi_pt*GeV, CutDefReal["JPsiPt"]);
    has_jpsi_eta=pass_cut(less_than, fabs(jpsi_eta),CutDefReal["JPsiEta"]);
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
    has_delta_r=pass_cut(less_than, DeltaR, CutDefReal["DeltaR"]);
    has_jet_eta=pass_cut(less_than, candJet.Eta(), CutDefReal["JetEta"]);
    has_jet_pt=pass_cut(greater_than, candJet.Pt(), CutDefReal["JetPt"]);
    z=jpsi_pt/(candJet.Pt()+jpsi_pt);

    cand_jet_pt=candJet.Pt();
    cand_jet_eta=candJet.Eta();
    cand_jet_phi=candJet.Phi();
    cand_jet_E=candJet.E();
    //loop over truth jets and pick the jet closest to the candidate jet
    tmp_dR=9999;
    //TODO Refactor this to a function and simplify above loop
    for(int j=0; j < t_jet_pt->size(); j++){
      tmpTruthJet.SetPtEtaPhiE(t_jet_pt->at(j)*GeV,t_jet_eta->at(j),
			       t_jet_phi->at(j), t_jet_E->at(j)*GeV);
      tmp_dR=candJet.DeltaR(tmpTruthJet);
      if(tmp_dR < t_DeltaR){
	t_DeltaR = tmp_dR;
	candTruthJet=tmpTruthJet;
      }
    }
    cand_t_jet_pt=candTruthJet.Pt();
    cand_t_jet_eta=candTruthJet.Eta();
    cand_t_jet_phi=candTruthJet.Phi();
    cand_t_jet_E=candTruthJet.E();

    OutTree.Fill();
  }
  return 0;
}
void print_cut_summary(std::string CutName, cut<int> Cut){
  printf("| %-8s | %8d | %6d | \n",CutName.c_str(),Cut.cut_value(),Cut.count());
}
void print_cut_summary(std::string CutName, cut<double> Cut){
  printf("| %-8s | %.2e | %6d | \n",CutName.c_str(),Cut.cut_value(),Cut.count());
}
