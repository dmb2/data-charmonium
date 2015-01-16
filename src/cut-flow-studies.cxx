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
#include "root-sugar.hh"
#include "Units.hh"

using namespace Units;
using std::cout;
using std::cerr;
using std::endl;
using namespace std;
bool verbose=true;
std::vector<TLorentzVector> buildMuons(const std::vector<double>* pt, const std::vector<double>* eta,
				       const std::vector<double>* phi, const std::vector<double>* e){
  std::vector<TLorentzVector> muons;
  muons.reserve(pt->size());
  TLorentzVector tmp;
  for(size_t i =0; i < pt->size(); i++){
    tmp.SetPtEtaPhiE(pt->at(i), eta->at(i), phi->at(i), e->at(i));
    muons.push_back(tmp);
  }
  return muons;
}

TLorentzVector buildJPsiCand(const std::vector<TLorentzVector>& muons, const std::vector<int>& charge){
  std::vector<TLorentzVector> dimuon_pairs;
  for(size_t i = 0; i < muons.size(); i++){
    for(size_t j = i; j < muons.size(); j++){
      if(charge.at(i)*charge.at(j) < 0){
	dimuon_pairs.push_back(muons.at(i)+ muons.at(j));
      }
    }
  }
  size_t idx=0;
  double max_pt=0;
  if(dimuon_pairs.size()==0){
    return TLorentzVector(0,0,0,0);
  }
  for(std::vector<TLorentzVector>::const_iterator jpsi=dimuon_pairs.begin();
      jpsi!=dimuon_pairs.end(); ++jpsi){
    if(jpsi->Pt() > max_pt){
      idx = jpsi-dimuon_pairs.begin();
      max_pt = jpsi->Pt();
    }
  }
  return dimuon_pairs.at(idx);
}
int process_tree(tree_collection& Forest, real_cuts& CutDefReal, 
		 category_cuts& CutDefCat, TTree& OutTree, 
		 const char* muon_system, const std::string& jet_type, const double weight){
  bool do_truth=(weight != 1.0);
  unsigned int squawk_every = 1e5;
  double pileup(0.);
  double tau1(0),tau2(0),tau3(0),tau21(0),tau32(0);
  double t_tau1(0),t_tau2(0),t_tau3(0),t_tau21(0),t_tau32(0);
  double z(0.), DeltaR(999.);
  double jpsi_lxy(0.);
  double jpsi_m(0.);
  double jpsi_pt(0.), jpsi_eta(0.), jpsi_phi(0.), jpsi_E(0.);
  std::vector<double> *vtx_pt=NULL;
  std::vector<double> *mu_pt=NULL, *mu_eta=NULL, *mu_phi=NULL, *mu_E=NULL;
  std::vector<int> *mu_charge=NULL;
  std::vector<double> *vtx_px=NULL, *vtx_py=NULL, *vtx_pz=NULL, *vtx_m=NULL;
  std::vector<std::vector<double> > *vtx_lxy=NULL;
  std::vector<double> *jet_tau1=NULL, *jet_tau2=NULL, *jet_tau3=NULL;
  std::vector<double> *jet_pt=NULL, *jet_eta=NULL, *jet_phi=NULL, *jet_E=NULL;

  double t_z(0.), t_DeltaR(0.);
  double t_jpsi_m(0.);
  double t_jpsi_pt(0.), t_jpsi_eta(0.),t_jpsi_phi(0.), t_jpsi_E(0.);
  std::vector<double> *t_jet_tau1=NULL, *t_jet_tau2=NULL, *t_jet_tau3=NULL;
  std::vector<double> *t_jet_pt=NULL, *t_jet_eta=NULL, 
    *t_jet_phi=NULL, *t_jet_E=NULL;
  std::vector<std::string>* EF_trigger_names=NULL;
  // setup_four_vector(Forest["AUX"], jpsi_pt, jpsi_eta, jpsi_phi, jpsi_E, "jpsi");
  char muon_prefix[50];
  snprintf(muon_prefix,50,"MU_MU_%s",muon_system);
  setup_pt_eta_phi_e(Forest["MU"],mu_pt,mu_eta,mu_phi,mu_E,muon_prefix);
  Forest["MU"]->SetBranchAddress("MU_MU_charge",&mu_charge);
  if(do_truth){
    setup_pt_eta_phi_e(Forest["AUX"], t_jpsi_pt, t_jpsi_eta, t_jpsi_phi, t_jpsi_E, "truth_jpsi");
  }
  setup_pt_eta_phi_e(Forest[jet_type], jet_pt, jet_eta, jet_phi, jet_E, "JET");
  if(do_truth){
    setup_pt_eta_phi_e(Forest["TRUTH"], t_jet_pt, t_jet_eta, t_jet_phi, t_jet_E, "JET");
  }
  Forest["AUX"]->SetBranchAddress("AvgIntPerXing",&pileup);
  Forest["JPSI"]->SetBranchAddress("VTX_lxy",&vtx_lxy);
  const char* vtx_names[] = {"px","py","pz","mass"};
  setup_four_vector(Forest["JPSI"],vtx_px,vtx_py,vtx_pz,vtx_m,"VTX",vtx_names);


    Forest["JPSI"]->SetBranchAddress("VTX_pt",&vtx_pt);
  Forest["TRIG"]->SetBranchAddress("TRIG_EF_trigger_name",&EF_trigger_names);

  Forest[jet_type]->SetBranchAddress("JET_tau1",&jet_tau1);
  Forest[jet_type]->SetBranchAddress("JET_tau2",&jet_tau2);
  Forest[jet_type]->SetBranchAddress("JET_tau3",&jet_tau3);
    if(do_truth){
    Forest["TRUTH"]->SetBranchAddress("JET_tau1",&t_jet_tau1);
    Forest["TRUTH"]->SetBranchAddress("JET_tau2",&t_jet_tau2);
    Forest["TRUTH"]->SetBranchAddress("JET_tau3",&t_jet_tau3);
  }



  double cand_jet_pt(0.), cand_jet_eta(0.),cand_jet_phi(0.), cand_jet_E(0.);
  double cand_t_jet_pt(0.), cand_t_jet_eta(0.), cand_t_jet_phi(0.), cand_t_jet_E(0.);

  setup_four_vector_output(OutTree,cand_jet_pt, cand_jet_eta, 
			   cand_jet_phi, cand_jet_E, "jet");
  if(do_truth){
    setup_four_vector_output(OutTree,cand_t_jet_pt, cand_t_jet_eta, 
			     cand_t_jet_phi, cand_t_jet_E, "truth_jet");
    OutTree.Branch("truth_jpsi_m",&t_jpsi_m);
    setup_four_vector_output(OutTree,t_jpsi_pt, t_jpsi_eta, t_jpsi_phi, t_jpsi_E, "truth_jpsi");
    OutTree.Branch("truth_tau1",&t_tau1);
    OutTree.Branch("truth_tau2",&t_tau2);
    OutTree.Branch("truth_tau3",&t_tau3);
    OutTree.Branch("truth_tau21",&t_tau21);
    OutTree.Branch("truth_tau32",&t_tau32);
		        
    OutTree.Branch("truth_jet_z",&t_z);
    OutTree.Branch("truth_delta_r",&t_DeltaR);
  }
  OutTree.Branch("jpsi_lxy",&jpsi_lxy);
  OutTree.Branch("jpsi_m",&jpsi_m);
  setup_four_vector_output(OutTree,jpsi_pt, jpsi_eta, jpsi_phi, jpsi_E, "jpsi");
  OutTree.Branch("pileup",&pileup);
  OutTree.Branch("tau1",&tau1);
  OutTree.Branch("tau2",&tau2);
  OutTree.Branch("tau3",&tau3);
  OutTree.Branch("tau21",&tau21);
  OutTree.Branch("tau32",&tau32);
		  
  OutTree.Branch("jet_z",&z);
  OutTree.Branch("delta_r",&DeltaR);


  double w=weight;
  OutTree.Branch("weight", &w);
  /*
  int has_trigger=0, has_num_jets=0, has_jpsi_pt=0, has_jpsi_eta=0, 
    has_jet_eta=0, has_delta_r=0, has_jet_pt=0;
  OutTree.Branch("mu_trigger_p",&has_trigger);
  OutTree.Branch("num_jets_p",&has_num_jets);
  OutTree.Branch("jpsi_pt_p",&has_jpsi_pt);
  OutTree.Branch("jpsi_eta_p",&has_jpsi_eta);
  OutTree.Branch("delta_r_p", &has_delta_r);
  OutTree.Branch("jet_eta_p",&has_jet_eta);
  OutTree.Branch("jet_pt_p",&has_jet_pt);
  */

  Long64_t nEntries = Forest["AUX"]->GetEntries();
  if(verbose) {
    cout<<"Got "<<nEntries<< " entries in input tree"<<endl;
  }
  std::vector<TLorentzVector> jets;
  TLorentzVector candJet(0,0,0,0);
  TLorentzVector candTruthJet(0,0,0,0);
  TLorentzVector candJPsi(0,0,0,0);
  size_t idx=0;
  size_t jpsi_idx=0;

  for(Long64_t entry=0; entry < nEntries; entry++){
    retrieve_values(Forest,entry);

    if(entry%squawk_every==0 && verbose){
      cout <<"Processing entry "<<entry<<endl;
    }
    idx=0;
    jpsi_idx=0;
    DeltaR=-1.;
    z=-1.;
    jets.clear();
    jets.reserve(jet_pt->size());
    CutDefCat["nominal"].pass();
    if(!CutDefCat["trigger"].pass(passed_trigger(*EF_trigger_names),w)){
      continue;
    };

    if(!CutDefCat["num_jets"].pass(int(jet_pt->size()),w)){
      continue;
    };
    std::vector<size_t> good_indices = filter_by_pt(*jet_pt, CutDefReal["jet_pt"].cut_value());
    // MSG_DEBUG("num jets: "<<jet_pt->size()<<" filtered: "<<good_indices.size());
    for(std::vector<size_t>::const_iterator itr=good_indices.begin();
	itr!=good_indices.end(); ++itr){
      TLorentzVector tmp_vec(0,0,0,0);
      tmp_vec.SetPtEtaPhiE(jet_pt->at(*itr)*GeV,
			   jet_eta->at(*itr),
			   jet_phi->at(*itr),
			   jet_E->at(*itr)*GeV);
      jets.push_back(tmp_vec);
    }
    if (vtx_pt->size() > 0){
      /*
      for(size_t i = 0; i < vtx_pt->size(); i++){
	if(vtx_pt->at(i) > vtx_pt->at(jpsi_idx)){
	  jpsi_idx=i;
	}
      }
      double E = TMath::Sqrt(pow(vtx_m->at(jpsi_idx),2)
			     - (  pow(vtx_px->at(jpsi_idx),2)
				  + pow(vtx_py->at(jpsi_idx),2)
				  + pow(vtx_pz->at(jpsi_idx),2)));
      candJPsi.SetPxPyPzE(vtx_px->at(jpsi_idx)*GeV, vtx_py->at(jpsi_idx)*GeV, vtx_pz->at(jpsi_idx)*GeV, E*GeV);
      */
      candJPsi=buildJPsiCand(buildMuons(mu_pt,mu_eta,mu_phi,mu_E),*mu_charge);
      jpsi_pt=candJPsi.Pt();
      jpsi_eta=candJPsi.Eta();
      jpsi_phi=candJPsi.Phi();
      jpsi_E=candJPsi.E();
      jpsi_m=candJPsi.M();
    }
    else{
      continue;
    }
    if(!CutDefReal["jpsi_pt"].pass(jpsi_pt,w)){
      continue;
    }
    if(!CutDefReal["jpsi_eta"].pass(fabs(jpsi_eta),w)){
      continue;
    }
    DeltaR=find_closest(jets,candJet,candJPsi,idx);
    // MSG_DEBUG(candJet.Pt());
    // DeltaR=find_closest(*jet_pt,*jet_eta,*jet_phi,*jet_E, candJet, candJPsi,idx);
    jpsi_lxy = (vtx_lxy->size() > 0) ? vtx_lxy->at(0).at(0) : -99999.;
    if(!CutDefReal["jpsi_lxy"].pass(fabs(jpsi_lxy),w)){
      continue;
    };
    if(!CutDefReal["delta_r"].pass(DeltaR,w)){
      continue;
    };
    if(!CutDefReal["jet_eta"].pass(fabs(candJet.Eta()),w)){
      continue;
    };
    if(!CutDefReal["jet_pt"].pass(candJet.Pt(),w)){
      continue;
    };
    z=(jpsi_pt)/(candJet.Pt()+jpsi_pt);
    if(jet_pt->size()==0){
      continue;
    }
    tau1=jet_tau1->at(idx);
    tau2=jet_tau2->at(idx);
    tau3=jet_tau3->at(idx);

    tau32= (tau3*tau2 > 0) ? tau3/tau2 : -1.;
    tau21= (tau2*tau1 > 0) ? tau2/tau1 : -1.;

    store_four_vector(candJet,cand_jet_pt,cand_jet_eta,cand_jet_phi,cand_jet_E);

    if(do_truth){
      idx=0;
      TLorentzVector tvec(0,0,0,0);
      tvec.SetPtEtaPhiE(t_jpsi_pt,t_jpsi_eta,t_jpsi_phi, t_jpsi_E);
      t_jpsi_m=tvec.M();
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
    }
    OutTree.Fill();
  }

  return 0;
}
