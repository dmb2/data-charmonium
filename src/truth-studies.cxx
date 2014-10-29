//System
#include <iostream>
#include <cstdlib>
#include <cstdio>
//ROOT 
#include "TFile.h"
#include "TLorentzVector.h"
#include "TTree.h"

//Local
#include "truth-studies.hh"
#include "Units.hh"

using namespace Units;
using std::cout;
using std::cerr;
using std::endl;
using namespace std;
bool verbose=true;
void find_jpsi(const std::vector<double>& pt,
	      const std::vector<double>& eta,
	      const std::vector<double>& phi,
	      const std::vector<double>& E,
	      const std::vector<int>& charge,
		    TLorentzVector& jpsi_cand){
  double maxPt(0.);
  const double jpsi_m = 3.096916;
  double deltaM=1e4;
  TLorentzVector cand(0,0,0,0);
  TLorentzVector m1;
  TLorentzVector m2;
  for(size_t i=0; i < pt.size(); i++){
    m1.SetPtEtaPhiE(pt.at(i)*GeV, eta.at(i),phi.at(i),E.at(i)*GeV);
    for(size_t j=0; j < pt.size(); j++){
      m2.SetPtEtaPhiE(pt.at(j)*GeV, eta.at(j),phi.at(j),E.at(j)*GeV);
      cand = m1 + m2;
      if(cand.Pt() > maxPt && fabs(cand.M() - jpsi_m) < deltaM &&
	 charge.at(i)*charge.at(j) < 0){
	jpsi_cand=cand;
	maxPt=cand.Pt();
	deltaM=fabs(cand.M()-jpsi_m);
      }
    }
  }
}

int process_tree(TTree& tree, real_cuts& CutDefReal, 
		 category_cuts& CutDefCat, TTree& OutTree){
  unsigned int squawk_every = 1000;
  double tau1(0),tau2(0),tau3(0),tau21(0),tau32(0);
  double z(0.), DeltaR(999.);
  double jpsi_pt(0.), jpsi_eta(0.), jpsi_phi(0.), jpsi_E(0.);

  std::vector<double> *jet_pt=NULL, *jet_eta=NULL, *jet_phi=NULL, *jet_E=NULL;
  std::vector<double> *mu_pt=NULL, *mu_eta=NULL, *mu_phi=NULL, *mu_E=NULL;
  std::vector<int> *mu_charge=NULL;
  tree.SetBranchAddress("mu_muid_charge",&mu_charge);
  double cand_jet_pt(0.), cand_jet_eta(0.),cand_jet_phi(0.), cand_jet_E(0.);
  setup_four_vector(&tree, jet_pt, jet_eta, jet_phi, jet_E, "jet_AntiKt4TruthJets",false);
  setup_four_vector(&tree, mu_pt, mu_eta, mu_phi, mu_E, "mu_muid",false);
  setup_four_vector_output(OutTree,cand_jet_pt, cand_jet_eta, 
			   cand_jet_phi, cand_jet_E, "jet");

  setup_four_vector_output(OutTree,jpsi_pt, jpsi_eta, jpsi_phi, jpsi_E, "jpsi");
  OutTree.Branch("jet_z",&z);
  OutTree.Branch("delta_r",&DeltaR);
  OutTree.Branch("tau1",&tau1);
  OutTree.Branch("tau2",&tau2);
  OutTree.Branch("tau3",&tau3);
  OutTree.Branch("tau21",&tau21);
  OutTree.Branch("tau32",&tau32);

  int has_trigger=1, has_num_jets=0, has_jpsi_pt=0, has_jpsi_eta=0, 
    has_jet_eta=0, has_delta_r=0, has_jet_pt=0;
  OutTree.Branch("mu_trigger_p",&has_trigger);
  OutTree.Branch("num_jets_p",&has_num_jets);
  OutTree.Branch("jpsi_pt_p",&has_jpsi_pt);
  OutTree.Branch("jpsi_eta_p",&has_jpsi_eta);
  OutTree.Branch("delta_r_p", &has_delta_r);
  OutTree.Branch("jet_eta_p",&has_jet_eta);
  OutTree.Branch("jet_pt_p",&has_jet_pt);

  Long64_t nEntries = tree.GetEntries();
  if(verbose) {
    cout<<"Got "<<nEntries<< " entries in input tree"<<endl;
  }
  TLorentzVector candJet(0,0,0,0);
  TLorentzVector candJPsi(0,0,0,0);
  size_t idx=0;
  for(Long64_t entry=0; entry < nEntries; entry++){
    tree.GetEntry(entry);
    if(entry%squawk_every==0 && verbose){
      cout <<"Processing entry "<<entry<<endl;
    }
    DeltaR=-1.;
    z=-1.;
    find_jpsi(*mu_pt,*mu_eta,*mu_phi,*mu_E,*mu_charge,candJPsi);
    jpsi_pt=candJPsi.Pt();
    jpsi_eta=candJPsi.Eta();
    jpsi_phi=candJPsi.Phi();
    jpsi_E=candJPsi.E();
    
    CutDefCat["nominal"].pass();
    has_num_jets=pass_cut(greater_than_eq, int(jet_pt->size()), CutDefCat["num_jets"]);
    has_jpsi_pt=pass_cut(greater_than, jpsi_pt, CutDefReal["jpsi_pt"]);
    has_jpsi_eta=pass_cut(less_than, fabs(jpsi_eta),CutDefReal["jpsi_eta"]);
    DeltaR=find_closest(*jet_pt,*jet_eta,*jet_phi,*jet_E, candJet, candJPsi,idx);
    has_delta_r=pass_cut(less_than, DeltaR, CutDefReal["delta_r"]);
    has_jet_eta=pass_cut(less_than, candJet.Eta(), CutDefReal["jet_eta"]);
    has_jet_pt=pass_cut(greater_than, candJet.Pt(), CutDefReal["jet_pt"]);

    z=(jpsi_pt)/(candJet.Pt()+jpsi_pt);
    /*
    tau1=jet_tau1->at(idx);
    tau2=jet_tau2->at(idx);
    tau3=jet_tau3->at(idx);
    tau32= (tau3*tau2 > 0) ? tau3/tau2 : -1.;
    tau21= (tau2*tau1 > 0) ? tau2/tau1 : -1.;
    */
    cand_jet_pt=candJet.Pt();
    cand_jet_eta=candJet.Eta();
    cand_jet_phi=candJet.Phi();
    cand_jet_E=candJet.E();
    idx=0;

    OutTree.Fill();
  }
  return 0;
}
