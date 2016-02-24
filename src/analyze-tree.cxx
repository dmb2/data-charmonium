//System
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
//ROOT 
#include "TFile.h"
#include "TLorentzVector.h"
#include "TTree.h"

//Local
#include "cut-flow-studies.hh"
#include "root-sugar.hh"
#include "Units.hh"

#ifndef __ANALYZE_TREE_CUTFLOW__
#define CUT_CONTINUE(cut) if(!(cut)){continue;};
#else
#define CUT_CONTINUE(cut) {};
#endif

using namespace Units;
bool verbose=true;

int process_tree(tree_collection& Forest, real_cuts& CutDefReal, 
		 category_cuts& CutDefCat, TTree& OutTree, 
		 const char* muon_variation, const std::string& jet_type, 
		 const double weight){
  bool is_MC=(weight != 1.0);
  unsigned int squawk_every = 1e3;
  std::vector<std::string>* EF_trigger_names=NULL;
  int trigger_cat(0);
  double pileup(0.);
  double tau1(0),tau2(0),tau3(0),tau21(0),tau32(0);
  double jpsi_s(0.);
  double z(0.), delta_r(999.);
  double jpsi_lxy(-99999.);
  double jpsi_vtx_z(99999.);
  double jpsi_tau(0.);

  double jpsi_m(0.), jpsi_rap(0.);
  double jpsi_pt(0.), jpsi_eta(0.), jpsi_phi(0.), jpsi_E(0.);
  double cand_psi_m(0.);
  double cand_jet_m(0.), emfrac(0.);
  double cand_jet_pt(0.), cand_jet_eta(0.),cand_jet_phi(0.), cand_jet_E(0.);
  std::vector<std::vector<int> > *mu_trk_idx = NULL;
  
  std::vector<int> *mu_charge=NULL;
  std::vector<double> *mu_pt=NULL, *mu_eta=NULL, *mu_phi=NULL, *mu_E=NULL;
  std::vector<std::vector<double> > *vtx_lxy=NULL; 
  std::vector<double> *vtx_pt=NULL, *vtx_z=NULL;
  std::vector<double> *psi_m=NULL, *psi_pt=NULL;
  std::vector<double> *vtx_px=NULL, *vtx_py=NULL, *vtx_pz=NULL, *vtx_e=NULL;
  std::vector<double> *jet_emfrac=NULL;
  std::vector<double> *jet_tau1=NULL, *jet_tau2=NULL, *jet_tau3=NULL;
  std::vector<double> *jet_pt=NULL, *jet_eta=NULL, *jet_phi=NULL, *jet_E=NULL;
  
  double t_z(0.), t_delta_r(0.);
  double t_jpsi_m(0.), t_jpsi_rap(0.);
  double t_jpsi_pt(0.), t_jpsi_eta(0.),t_jpsi_phi(0.), t_jpsi_E(0.);
  double cand_t_jet_m(0.);
  double cand_t_jet_pt(0.), cand_t_jet_eta(0.), cand_t_jet_phi(0.), cand_t_jet_E(0.);

  double t_tau1(0),t_tau2(0),t_tau3(0),t_tau21(0),t_tau32(0);
  std::vector<double> *t_jet_tau1=NULL, *t_jet_tau2=NULL, *t_jet_tau3=NULL;
  std::vector<double> *t_jet_pt=NULL, *t_jet_eta=NULL, 
    *t_jet_phi=NULL, *t_jet_E=NULL;

  char muon_prefix[50];
  snprintf(muon_prefix,50,"Mu_MU");
  setup_pt_eta_phi_e(Forest["Mu"],mu_pt,mu_eta,mu_phi,mu_E,muon_prefix);
  if(std::string(muon_variation)!="" && std::string(muon_variation).find("Efficiency")==std::string::npos){
    MSG_DEBUG("Overriding mu_pt branch for variation: "<<muon_variation);
    snprintf(muon_prefix,50,"Mu_MU_pt%s",muon_variation);
    Forest["Mu"]->SetBranchAddress(muon_prefix,&mu_pt);
  }
    
  
  std::vector<double> *MuSF(NULL),*MuSFSystErr(NULL),*MuSFStatErr(NULL),*MuSFTotalErr(NULL);
  Forest["Mu"]->SetBranchAddress("Mu_MU_charge",&mu_charge);
  Forest["Mu"]->SetBranchAddress("Mu_MU_SF",&MuSF);
  Forest["Mu"]->SetBranchAddress("Mu_MU_SFSystErr",&MuSFSystErr);
  Forest["Mu"]->SetBranchAddress("Mu_MU_SFStatErr",&MuSFStatErr);
  Forest["Mu"]->SetBranchAddress("Mu_MU_SFTotalErr",&MuSFTotalErr);
  Forest["AUX"]->SetBranchAddress("AvgIntPerXing",&pileup);

  Forest["JPsi2Trk"]->SetBranchAddress("VTX_mass",&psi_m);
  Forest["JPsi2Trk"]->SetBranchAddress("VTX_pt",&psi_pt);
  const char* vtx_names[] = {"px","py","pz","e"};
  MSG_DEBUG("JPsi Tree: "<<Forest["JPsi"]);
  setup_four_vector(Forest["JPsi"],vtx_px,vtx_py,vtx_pz,vtx_e,"VTX",vtx_names);
  Forest["JPsi"]->SetBranchAddress("MUONS_index",&mu_trk_idx);
  Forest["JPsi"]->SetBranchAddress("VTX_pt",&vtx_pt);
  Forest["JPsi"]->SetBranchAddress("VTX_lxy",&vtx_lxy);
  Forest["JPsi"]->SetBranchAddress("VTX_zposition",&vtx_z);
  Forest["TRIG"]->SetBranchAddress("TRIG_EF_trigger_name",&EF_trigger_names);
  // Forest["MuTracks"]->SetBranchAddress("MuTracks_TRKS_qOverP",&mu_qbyp);
  // Forest["MuTracks"]->SetBranchAddress("MuTracks_TRKS_d0",&mu_d0);
  // Forest["MuTracks"]->SetBranchAddress("MuTracks_TRKS_d0Err",&mu_d0_err);

  setup_pt_eta_phi_e(Forest[jet_type], jet_pt, jet_eta, jet_phi, jet_E, "JET");
  Forest[jet_type]->SetBranchAddress("JET_tau1",&jet_tau1);
  Forest[jet_type]->SetBranchAddress("JET_tau2",&jet_tau2);
  Forest[jet_type]->SetBranchAddress("JET_tau3",&jet_tau3);
  Forest[jet_type]->SetBranchAddress("JET_emfrac",&jet_emfrac);
  if(is_MC){
    setup_pt_eta_phi_e(Forest["AUX"], t_jpsi_pt, t_jpsi_eta, t_jpsi_phi, t_jpsi_E, "truth_jpsi");
    const std::string t_jet_type = (jet_type=="MuonLCTopoJets" /*|| jet_type.find("TrackZ")!=std::string::npos */) ? "MuonTruthJets" : "TruthJets";
    // MSG_DEBUG("Setting up with tree: "<<t_jet_type<<" using jet type: "<<jet_type);
    setup_pt_eta_phi_e(Forest[t_jet_type], t_jet_pt, t_jet_eta, t_jet_phi, t_jet_E, "JET");
    Forest[t_jet_type]->SetBranchAddress("JET_tau1",&t_jet_tau1);
    Forest[t_jet_type]->SetBranchAddress("JET_tau2",&t_jet_tau2);
    Forest[t_jet_type]->SetBranchAddress("JET_tau3",&t_jet_tau3);
  }

  setup_four_vector_output(OutTree,cand_jet_pt, cand_jet_eta, 
			   cand_jet_phi, cand_jet_E, "jet");
  if(is_MC){
    setup_four_vector_output(OutTree,cand_t_jet_pt, cand_t_jet_eta, 
			     cand_t_jet_phi, cand_t_jet_E, "truth_jet");
    OutTree.Branch("truth_jpsi_rap",&t_jpsi_rap);
    OutTree.Branch("truth_jpsi_m",&t_jpsi_m);
    setup_four_vector_output(OutTree,t_jpsi_pt, t_jpsi_eta, t_jpsi_phi, t_jpsi_E, "truth_jpsi");
    OutTree.Branch("truth_tau1",&t_tau1);
    OutTree.Branch("truth_tau2",&t_tau2);
    OutTree.Branch("truth_tau3",&t_tau3);
    OutTree.Branch("truth_tau21",&t_tau21);
    OutTree.Branch("truth_tau32",&t_tau32);
		        
    OutTree.Branch("truth_jet_z",&t_z);
    OutTree.Branch("truth_jet_m",&cand_t_jet_m);
    OutTree.Branch("truth_delta_r",&t_delta_r);
  }
  OutTree.Branch("trigger_category",&trigger_cat);
  OutTree.Branch("psi_m",&cand_psi_m);
  OutTree.Branch("jpsi_s",&jpsi_s);
  OutTree.Branch("jpsi_lxy",&jpsi_lxy);
  OutTree.Branch("jpsi_vtx_z",&jpsi_vtx_z);
  OutTree.Branch("jpsi_tau",&jpsi_tau);
  OutTree.Branch("jpsi_m",&jpsi_m);
  OutTree.Branch("jpsi_rap",&jpsi_rap);
  OutTree.Branch("pileup",&pileup);
  OutTree.Branch("tau1",&tau1);
  OutTree.Branch("tau2",&tau2);
  OutTree.Branch("tau3",&tau3);
  OutTree.Branch("tau21",&tau21);
  OutTree.Branch("tau32",&tau32);
  OutTree.Branch("jet_z",&z);
  OutTree.Branch("jet_emfrac",&emfrac);
  OutTree.Branch("jet_m",&cand_jet_m);
  OutTree.Branch("delta_r",&delta_r);
  setup_four_vector_output(OutTree,jpsi_pt, jpsi_eta, jpsi_phi, jpsi_E, "jpsi");

  int has_trigger=0, has_num_jets=0, has_jpsi_pt=0, /*has_jpsi_eta=0,*/
    has_mumu_eta,has_jpsi_rap,has_jet_eta=0, has_delta_r=0, has_jet_pt=0;

#ifdef __ANALYZE_TREE_CUTFLOW__  
  OutTree.Branch("mu_trigger_p",&has_trigger);
  OutTree.Branch("num_jets_p",&has_num_jets);
  OutTree.Branch("jpsi_pt_p",&has_jpsi_pt);
  OutTree.Branch("jpsi_rap_p",&has_jpsi_rap);
  OutTree.Branch("mumu_eta_p",&has_mumu_eta);
  // OutTree.Branch("jpsi_eta_p",&has_jpsi_eta);
  OutTree.Branch("delta_r_p", &has_delta_r);
  OutTree.Branch("jet_eta_p",&has_jet_eta);
  OutTree.Branch("jet_pt_p",&has_jet_pt);
#endif
  double SF(1.),SFSystErr(1.),SFStatErr(1.),SFTotalErr(1.);
  double w=weight;
  OutTree.Branch("weight", &w);
  OutTree.Branch("SF", &SF);
  OutTree.Branch("SFSystErr", &SFSystErr);
  OutTree.Branch("SFStatErr", &SFStatErr);
  OutTree.Branch("SFTotalErr", &SFTotalErr);

  Long64_t nEntries = Forest["AUX"]->GetEntries();
  if(verbose) {
    MSG("Got "<<nEntries<< " entries in input tree");
  }
  std::vector<TLorentzVector> jets;
  TLorentzVector candJet(0,0,0,0);
  TLorentzVector candTruthJet(0,0,0,0);
  TLorentzVector candJPsi(0,0,0,0);
  size_t idx=0;
  std::pair<TLorentzVector,TLorentzVector> jpsi_muons;
  // size_t jpsi_idx=0;
  MSG("Weight: "<<weight);
  for(Long64_t entry=0; entry < nEntries; entry++){
    retrieve_values(Forest,entry);
    
    if(entry%squawk_every==0 && verbose){
      MSG("Processing entry "<<entry);
    }
    trigger_cat=0;
    idx=0; delta_r=-1.; z=-1.;//jpsi_idx=0
    jpsi_muons=std::pair<TLorentzVector,TLorentzVector>(TLorentzVector(0,0,0,0),
							TLorentzVector(0,0,0,0));
    // SF=1; SFSystErr=0.; SFStatErr=0.; SFTotalErr=0.;
    if(mu_pt->size()==0){
      continue;
    }
    SF=total_scale_factor(MuSF);
    SFStatErr=total_scale_factor(MuSFStatErr);
    SFSystErr=total_scale_factor(MuSFSystErr);
    SFTotalErr=total_scale_factor(MuSFTotalErr);
    
    jets.clear(); jets.reserve(jet_pt->size());
    CutDefCat["nominal"].pass();
    has_trigger = CutDefCat["trigger"].pass(is_MC || passed_trigger(*EF_trigger_names),w);
    has_num_jets = CutDefCat["num_jets"].pass(int(jet_pt->size()),w);
    CUT_CONTINUE(has_trigger);
    CUT_CONTINUE(has_num_jets);
    for(std::vector<std::string>::const_iterator name=EF_trigger_names->begin();
	name!=EF_trigger_names->end(); ++name){

      if(*name == "EF_mu36_tight"){
	trigger_cat=1;
	break;
      }
      if(*name == "EF_mu24i_tight"){
	trigger_cat=2;
	break;
      }
    }
    std::vector<size_t> good_indices = filter_by_pt(*jet_pt, CutDefReal["jet_pt"].cut_value());
    for(std::vector<size_t>::const_iterator itr=good_indices.begin();
	itr!=good_indices.end(); ++itr){
      TLorentzVector tmp_vec(0,0,0,0);
      tmp_vec.SetPtEtaPhiE(jet_pt->at(*itr)*GeV,
			   jet_eta->at(*itr),
			   jet_phi->at(*itr),
			   jet_E->at(*itr)*GeV);
      jets.push_back(tmp_vec);
    }
    /*
    int mu1_idx=-1, mu2_idx=-1;
    CUT_CONTINUE((vtx_pt->size() > 0));
    for(size_t i = 0; i < vtx_pt->size(); i++){
      if(vtx_pt->at(i) > vtx_pt->at(jpsi_idx)){
	jpsi_idx=i;
      }
    }
    if (mu_trk_idx->size() && mu_trk_idx->at(jpsi_idx).size() == 2){
      mu1_idx=mu_trk_idx->at(jpsi_idx)[0];
      mu2_idx=mu_trk_idx->at(jpsi_idx)[1];
    }
    if(mu1_idx < 0 || mu2_idx < 0){
#ifndef __ANALYZE_TREE_CUTFLOW__      
      MSG_DEBUG("WARNING: J/\\psi candidate is missing muon track indices");
#endif
      continue;
    }
    double mu_max_eta=std::max(fabs(mu_eta->at(mu1_idx)),fabs(mu_eta->at(mu2_idx)));
    */
    // candJPsi.SetPxPyPzE(vtx_px->at(jpsi_idx)*GeV, vtx_py->at(jpsi_idx)*GeV, vtx_pz->at(jpsi_idx)*GeV, vtx_e->at(jpsi_idx)*GeV);
    // size_t mu_n = mu_pt->size();
    jpsi_muons=buildJPsiCand(buildMuons(mu_pt,mu_eta,mu_phi,mu_E),*mu_charge);
    double mu_max_eta=std::max(fabs(jpsi_muons.first.Eta()),
			fabs(jpsi_muons.second.Eta()));
    has_mumu_eta = CutDefReal["mumu_eta"].pass(mu_max_eta,w);
    CUT_CONTINUE(has_mumu_eta);

    candJPsi=jpsi_muons.first + jpsi_muons.second;
    jpsi_pt=candJPsi.Pt();
    jpsi_eta=candJPsi.Eta();
    jpsi_rap=candJPsi.Rapidity();
    jpsi_phi=candJPsi.Phi();
    jpsi_E=candJPsi.E();
    jpsi_m=candJPsi.M();
    has_jpsi_pt=CutDefReal["jpsi_pt"].pass(jpsi_pt,w);
    // has_jpsi_eta=CutDefReal["jpsi_eta"].pass(fabs(jpsi_eta),w);
    has_jpsi_rap=CutDefReal["jpsi_rap"].pass(fabs(jpsi_rap),w);
    CUT_CONTINUE(has_jpsi_pt);
    // CUT_CONTINUE(has_jpsi_eta);
    CUT_CONTINUE(has_jpsi_rap);
    if(psi_m->size() > 0){
      double tmp_psi_pt(0.);
      for(size_t i = 0; i < psi_m->size(); i++){
	if(psi_pt->at(i) > tmp_psi_pt){
	  tmp_psi_pt=psi_pt->at(i);
	  cand_psi_m = psi_m->at(i);
	}
      }
    }
    cand_psi_m*=GeV;
    delta_r=find_closest(jets,candJet,candJPsi,idx);
    jpsi_lxy = (vtx_lxy->size() > 0) ? vtx_lxy->at(0).at(0) : -99999.;
    jpsi_vtx_z = (vtx_z->size() > 0) ? vtx_z->at(0) : -99999.;
    jpsi_tau = jpsi_lxy*(3096.915*GeV)/jpsi_pt;

    has_delta_r=CutDefReal["delta_r"].pass(delta_r,w);
    has_jet_eta=CutDefReal["jet_eta"].pass(fabs(candJet.Eta()),w);
    has_jet_pt=CutDefReal["jet_pt"].pass(candJet.Pt(),w);
    CUT_CONTINUE(has_delta_r);
    CUT_CONTINUE(has_jet_eta);
    CUT_CONTINUE(has_jet_pt);
    if(jet_type.find("TrackZ")!=std::string::npos || jet_type == "MuonLCTopoJets"){
      z=(jpsi_pt)/candJet.Pt();
    }
    else {
      z=(jpsi_pt)/(candJet.Pt()+jpsi_pt);
    }
    
    if(jet_pt->size()==0){
      continue;
    }

    emfrac=jet_emfrac->at(idx);
    tau1=jet_tau1->at(idx);
    tau2=jet_tau2->at(idx);
    tau3=jet_tau3->at(idx);

    tau32= (tau3*tau2 > 0) ? tau3/tau2 : -1.;
    tau21= (tau2*tau1 > 0) ? tau2/tau1 : -1.;

    cand_jet_m = candJet.M();
    store_four_vector(candJet,cand_jet_pt,cand_jet_eta,cand_jet_phi,cand_jet_E);

    if(is_MC){
      idx=0;
      t_jpsi_pt*=GeV;
      t_jpsi_E*=GeV;
      TLorentzVector tvec(0,0,0,0);
      tvec.SetPtEtaPhiE(t_jpsi_pt,t_jpsi_eta,t_jpsi_phi, t_jpsi_E);
      t_jpsi_rap=tvec.Rapidity();
      t_jpsi_m=tvec.M();
      t_delta_r=find_closest(*t_jet_pt,*t_jet_eta,*t_jet_phi,*t_jet_E, 
			    candTruthJet, candJet,idx);
      if(jet_type.find("TrackZ")!=std::string::npos  || jet_type == "MuonLCTopoJets"){
	t_z=(t_jpsi_pt)/candTruthJet.Pt();
      }
      else {
	t_z=(t_jpsi_pt)/(candTruthJet.Pt()+t_jpsi_pt);
      }
      cand_t_jet_m=candTruthJet.M();
      store_four_vector(candTruthJet, cand_t_jet_pt, cand_t_jet_eta, 
			cand_t_jet_phi, cand_t_jet_E);
      if(t_jet_tau1->size()==0){
	continue;
      }
      t_tau1=t_jet_tau1->at(idx);
      t_tau2=t_jet_tau2->at(idx);
      t_tau3=t_jet_tau3->at(idx);
      t_tau32=t_tau3/t_tau2;
      t_tau21=t_tau2/t_tau1;
    }
    OutTree.Fill();
  }
  // Clean up the forest
  for(tree_collection::iterator tree = Forest.begin(); tree != Forest.end(); ++tree){
    tree->second->ResetBranchAddresses();
  }
  return 0;
}
