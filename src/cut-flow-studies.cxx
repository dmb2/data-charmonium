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
using namespace std;
bool verbose=true;

int process_tree(tree_collection& Forest, real_cuts& CutDefReal, 
		 category_cuts& CutDefCat, TTree& OutTree){
  unsigned int squawk_every = 1000;
  double pileup(0.);
  double tau1(0),tau2(0),tau3(0),tau21(0),tau32(0);
  double t_tau1(0),t_tau2(0),t_tau3(0),t_tau21(0),t_tau32(0);
  double z(0.), DeltaR(999.);
  double jpsi_pt(0.), jpsi_eta(0.), jpsi_phi(0.), jpsi_E(0.);
  std::vector<double> *jet_tau1=NULL, *jet_tau2=NULL, *jet_tau3=NULL;
  std::vector<double> *jet_pt=NULL, *jet_eta=NULL, *jet_phi=NULL, *jet_E=NULL;

  double t_z(0.), t_DeltaR(0.);
  double t_jpsi_pt(0.), t_jpsi_eta(0.),t_jpsi_phi(0.), t_jpsi_E(0.);
  std::vector<double> *t_jet_tau1=NULL, *t_jet_tau2=NULL, *t_jet_tau3=NULL;
  std::vector<double> *t_jet_pt=NULL, *t_jet_eta=NULL, 
    *t_jet_phi=NULL, *t_jet_E=NULL;
  std::vector<std::string>* EF_trigger_names=NULL;

  setup_four_vector(Forest["AUX"], jpsi_pt, jpsi_eta, jpsi_phi, jpsi_E, "jpsi");
  setup_four_vector(Forest["AUX"], t_jpsi_pt, t_jpsi_eta, t_jpsi_phi, t_jpsi_E, "truth_jpsi");
  setup_four_vector(Forest["JET"], jet_pt, jet_eta, jet_phi, jet_E, "JET");
  setup_four_vector(Forest["TRUTH_JET"], t_jet_pt, t_jet_eta, t_jet_phi, t_jet_E, "JET");
  Forest["AUX"]->SetBranchAddress("AvgIntPerXing",&pileup);
  Forest["TRIG"]->SetBranchAddress("TRIG_EF_trigger_name",&EF_trigger_names);
  Forest["JET"]->SetBranchAddress("JET_tau1",&jet_tau1);
  Forest["JET"]->SetBranchAddress("JET_tau2",&jet_tau2);
  Forest["JET"]->SetBranchAddress("JET_tau3",&jet_tau3);

  Forest["TRUTH_JET"]->SetBranchAddress("JET_tau1",&t_jet_tau1);
  Forest["TRUTH_JET"]->SetBranchAddress("JET_tau2",&t_jet_tau2);
  Forest["TRUTH_JET"]->SetBranchAddress("JET_tau3",&t_jet_tau3);


  double cand_jet_pt(0.), cand_jet_eta(0.),cand_jet_phi(0.), cand_jet_E(0.);
  double cand_t_jet_pt(0.), cand_t_jet_eta(0.), cand_t_jet_phi(0.), cand_t_jet_E(0.);

  setup_four_vector_output(OutTree,cand_jet_pt, cand_jet_eta, 
			   cand_jet_phi, cand_jet_E, "jet");
  setup_four_vector_output(OutTree,cand_t_jet_pt, cand_t_jet_eta, 
			   cand_t_jet_phi, cand_t_jet_E, "truth_jet");

  setup_four_vector_output(OutTree,t_jpsi_pt, t_jpsi_eta, t_jpsi_phi, t_jpsi_E, "truth_jpsi");
  setup_four_vector_output(OutTree,jpsi_pt, jpsi_eta, jpsi_phi, jpsi_E, "jpsi");
  OutTree.Branch("pileup",&pileup);
  OutTree.Branch("jet_z",&z);
  OutTree.Branch("delta_r",&DeltaR);
  OutTree.Branch("tau1",&tau1);
  OutTree.Branch("tau2",&tau2);
  OutTree.Branch("tau3",&tau3);
  OutTree.Branch("tau21",&tau21);
  OutTree.Branch("tau32",&tau32);

  OutTree.Branch("truth_tau1",&t_tau1);
  OutTree.Branch("truth_tau2",&t_tau2);
  OutTree.Branch("truth_tau3",&t_tau3);
  OutTree.Branch("truth_tau21",&t_tau21);
  OutTree.Branch("truth_tau32",&t_tau32);

  OutTree.Branch("truth_jet_z",&t_z);
  OutTree.Branch("truth_delta_r",&t_DeltaR);

  int has_trigger=0, has_num_jets=0, has_jpsi_pt=0, has_jpsi_eta=0, 
    has_jet_eta=0, has_delta_r=0, has_jet_pt=0;
  OutTree.Branch("mu_trigger_p",&has_trigger);
  OutTree.Branch("num_jets_p",&has_num_jets);
  OutTree.Branch("jpsi_pt_p",&has_jpsi_pt);
  OutTree.Branch("jpsi_eta_p",&has_jpsi_eta);
  OutTree.Branch("delta_r_p", &has_delta_r);
  OutTree.Branch("jet_eta_p",&has_jet_eta);
  OutTree.Branch("jet_pt_p",&has_jet_pt);

  Long64_t nEntries = Forest["AUX"]->GetEntries();
  if(verbose) {
    cout<<"Got "<<nEntries<< " entries in input tree"<<endl;
  }
  TLorentzVector candJet(0,0,0,0);
  TLorentzVector candTruthJet(0,0,0,0);
  TLorentzVector candJPsi(0,0,0,0);
  size_t idx=0;
  for(Long64_t entry=0; entry < nEntries; entry++){
    retrieve_values(Forest,entry);
    if(entry%squawk_every==0 && verbose){
      cout <<"Processing entry "<<entry<<endl;
    }
    DeltaR=-1.;
    z=-1.;
    jpsi_E*=GeV;
    jpsi_pt*=GeV;
    t_jpsi_E*=GeV;
    t_jpsi_pt*=GeV;
    CutDefCat["nominal"].pass();
    has_trigger=CutDefCat["mu_trigger"].pass(passed_trigger(*EF_trigger_names));
    has_num_jets=CutDefCat["num_jets"].pass(int(jet_pt->size()));
    has_jpsi_pt=CutDefReal["jpsi_pt"].pass(jpsi_pt);
    has_jpsi_eta=CutDefReal["jpsi_eta"].pass(fabs(jpsi_eta));
    candJPsi.SetPtEtaPhiE(jpsi_pt, jpsi_eta, 
			  jpsi_phi, jpsi_E);
    DeltaR=find_closest(*jet_pt,*jet_eta,*jet_phi,*jet_E, candJet, candJPsi,idx);
    has_delta_r=CutDefReal["delta_r"].pass(DeltaR);
    has_jet_eta=CutDefReal["jet_eta"].pass(fabs(candJet.Eta()));
    has_jet_pt=CutDefReal["jet_pt"].pass(candJet.Pt());

    z=(jpsi_pt)/(candJet.Pt()+jpsi_pt);
    tau1=jet_tau1->at(idx);
    tau2=jet_tau2->at(idx);
    tau3=jet_tau3->at(idx);
    tau32= (tau3*tau2 > 0) ? tau3/tau2 : -1.;
    tau21= (tau2*tau1 > 0) ? tau2/tau1 : -1.;
    store_four_vector(candJet,cand_jet_pt,cand_jet_eta,cand_jet_phi,cand_jet_E);
    idx=0;
    t_DeltaR=find_closest(*t_jet_pt,*t_jet_eta,*t_jet_phi,*t_jet_E, 
			  candTruthJet, candJet,idx);
    t_z=t_jpsi_pt/(candTruthJet.Pt()+t_jpsi_pt);
    store_four_vector(candTruthJet, cand_t_jet_pt, cand_t_jet_eta, 
		      cand_t_jet_phi, cand_t_jet_E);

    t_tau1=t_jet_tau1->at(idx);
    t_tau2=t_jet_tau2->at(idx);
    t_tau3=t_jet_tau3->at(idx);
    t_tau32=t_tau3/t_tau2;
    t_tau21=t_tau2/t_tau1;


    OutTree.Fill();
  }
  return 0;
}
