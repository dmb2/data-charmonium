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
#include "analyze-tree.hh"
#include "root-sugar.hh"
#include "Units.hh"
//RootCore
#include "TPileupReweighting.h"

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
  double z(0.), delta_r(999.);
  double jpsi_lxy(-99999.);
  double jpsi_vtx_z(99999.);
  double jpsi_tau(0.);

  double jpsi_m(0.), jpsi_rap(0.);
  double jpsi_pt(0.), jpsi_eta(0.), jpsi_phi(0.), jpsi_E(0.);
  // double cand_psi_m(0.);
  int jet_n(0);
  double cand_jet_m(0.);//, emfrac(0.);
  double cand_jet_pt(0.), cand_jet_eta(0.),cand_jet_phi(0.), cand_jet_E(0.);
  double mu1_pt(0.), mu1_eta(0.0);
  double mu2_pt(0.), mu2_eta(0.0);
  
  int vtx_n(0);
  int vtx_ntrk(0);
  double vtx_chi2(0.);
  //Jet systematics 
  double cand_jet_filt_pt(0.), cand_jet_filt_eta(0.),cand_jet_filt_phi(0.), cand_jet_filt_E(0.);
  double cand_jet_smear_pt(0.), cand_jet_smear_eta(0.),cand_jet_smear_phi(0.), cand_jet_smear_E(0.);
  double cand_jet_sup_pt(0.), cand_jet_sup_eta(0.),cand_jet_sup_phi(0.), cand_jet_sup_E(0.);
  double cand_jet_sdown_pt(0.), cand_jet_sdown_eta(0.),cand_jet_sdown_phi(0.), cand_jet_sdown_E(0.);
  double cand_jet_rup_pt(0.), cand_jet_rup_eta(0.),cand_jet_rup_phi(0.), cand_jet_rup_E(0.);
  double cand_jet_rdown_pt(0.), cand_jet_rdown_eta(0.),cand_jet_rdown_phi(0.), cand_jet_rdown_E(0.);


  std::vector<std::vector<int> > *mu_trk_idx = NULL;
  
  std::vector<int> *mu_charge=NULL;
  std::vector<double> *mu_nom_pt = NULL;
  std::vector<double> *mu_pt=NULL, *mu_eta=NULL, *mu_phi=NULL, *mu_E=NULL;
  std::vector<std::vector<double> > *vtx_lxy=NULL; 
  std::vector<double> *vtx_pt=NULL, *vtx_z=NULL;
  // std::vector<double> *psi_m=NULL, *psi_pt=NULL;
  std::vector<double> *vtx_px=NULL, *vtx_py=NULL, *vtx_pz=NULL, *vtx_e=NULL;
  // std::vector<double> *jet_emfrac=NULL;
  std::vector<double> *jet_tau1=NULL, *jet_tau2=NULL, *jet_tau3=NULL;
  std::vector<double> *jet_pt=NULL, *jet_eta=NULL, *jet_phi=NULL, *jet_E=NULL;

  std::vector<double> *jet_filt_pt=NULL, *jet_filt_eta=NULL, *jet_filt_phi=NULL, *jet_filt_E=NULL;
  std::vector<double> *jet_smear_pt=NULL, *jet_smear_eta=NULL, *jet_smear_phi=NULL, *jet_smear_E=NULL;
  std::vector<double> *jet_sup_pt=NULL, *jet_sup_eta=NULL, *jet_sup_phi=NULL, *jet_sup_E=NULL;
  std::vector<double> *jet_sdown_pt=NULL, *jet_sdown_eta=NULL, *jet_sdown_phi=NULL, *jet_sdown_E=NULL;
  std::vector<double> *jet_rup_pt=NULL, *jet_rup_eta=NULL, *jet_rup_phi=NULL, *jet_rup_E=NULL;
  std::vector<double> *jet_rdown_pt=NULL, *jet_rdown_eta=NULL, *jet_rdown_phi=NULL, *jet_rdown_E=NULL;
  
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
    Forest["Mu"]->SetBranchAddress("Mu_MU_pt",&mu_nom_pt);
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

  // Forest["JPsi2Trk"]->SetBranchAddress("VTX_mass",&psi_m);
  // Forest["JPsi2Trk"]->SetBranchAddress("VTX_pt",&psi_pt);
  const char* vtx_names[] = {"px","py","pz","e"};
  MSG_DEBUG("JPsi Tree: "<<Forest["JPsi"]);
  setup_four_vector(Forest["JPsi"],vtx_px,vtx_py,vtx_pz,vtx_e,"VTX",vtx_names);
  Forest["JPsi"]->SetBranchAddress("MUONS_index",&mu_trk_idx);
  Forest["JPsi"]->SetBranchAddress("VTX_pt",&vtx_pt);
  Forest["JPsi"]->SetBranchAddress("VTX_lxy",&vtx_lxy);
  Forest["JPsi"]->SetBranchAddress("VTX_zposition",&vtx_z);
  std::vector<double> *pvtx_chi2(NULL);
  std::vector<int> *pvtx_ntrk(NULL);
  Forest["PRIVX"]->SetBranchAddress("PRIVX_chi2",&pvtx_chi2);
  Forest["PRIVX"]->SetBranchAddress("PRIVX_ntrk",&pvtx_ntrk);
  
  Forest["TRIG"]->SetBranchAddress("TRIG_EF_trigger_name",&EF_trigger_names);
  // Forest["MuTracks"]->SetBranchAddress("MuTracks_TRKS_qOverP",&mu_qbyp);
  // Forest["MuTracks"]->SetBranchAddress("MuTracks_TRKS_d0",&mu_d0);
  // Forest["MuTracks"]->SetBranchAddress("MuTracks_TRKS_d0Err",&mu_d0_err);

  setup_pt_eta_phi_e(Forest[jet_type], jet_pt, jet_eta, jet_phi, jet_E, "JET");
  
  Forest[jet_type]->SetBranchAddress("JET_tau1",&jet_tau1);
  Forest[jet_type]->SetBranchAddress("JET_tau2",&jet_tau2);
  Forest[jet_type]->SetBranchAddress("JET_tau3",&jet_tau3);
  // Forest[jet_type]->SetBranchAddress("JET_emfrac",&jet_emfrac);
  setup_four_vector_output(OutTree,cand_jet_pt, cand_jet_eta, 
			   cand_jet_phi, cand_jet_E, "jet");
  Root::TPileupReweighting* purw=nullptr;
  if(is_MC){
    // FIXME initialize and use this tool
    purw = new Root::TPileupReweighting( "purw" );
    if(purw->initialize("pileup_files/data2012_lumi.root","avgintperbx",
			"pileup_files/MC12Pileup.root","pileup")!=0){
      MSG_ERR("Could not initialize pileup reweighting! Are the files in pileup_files/ and named correctly?");
      exit(-1);
    }
    if(jet_type=="TrackZJPsiJets"){
      setup_pt_eta_phi_e(Forest["TrackZFilteredJPsiJets"], jet_filt_pt, jet_filt_eta, jet_filt_phi, jet_filt_E, "JET");
      setup_pt_eta_phi_e(Forest["TrackZSmearedJPsiJets"], jet_smear_pt, jet_smear_eta, jet_smear_phi, jet_smear_E, "JET");
      setup_pt_eta_phi_e(Forest["TrackZScaledUpJPsiJets"], jet_sup_pt, jet_sup_eta, jet_sup_phi, jet_sup_E, "JET");
      setup_pt_eta_phi_e(Forest["TrackZScaledDownJPsiJets"], jet_sdown_pt, jet_sdown_eta, jet_sdown_phi, jet_sdown_E, "JET");
      setup_pt_eta_phi_e(Forest["TrackZRadialScaledUpJPsiJets"], jet_rup_pt, jet_rup_eta, jet_rup_phi, jet_rup_E, "JET");
      setup_pt_eta_phi_e(Forest["TrackZRadialScaledDownJPsiJets"], jet_rdown_pt, jet_rdown_eta, jet_rdown_phi, jet_rdown_E, "JET");
    }
    setup_pt_eta_phi_e(Forest["AUX"], t_jpsi_pt, t_jpsi_eta, t_jpsi_phi, t_jpsi_E, "truth_jpsi");
    const std::string t_jet_type = (jet_type=="MuonLCTopoJets" /*|| jet_type.find("TrackZ")!=std::string::npos */) ? "MuonTruthJets" : "JPsiTruthJets";
    // MSG_DEBUG("Setting up with tree: "<<t_jet_type<<" using jet type: "<<jet_type);
    setup_pt_eta_phi_e(Forest[t_jet_type], t_jet_pt, t_jet_eta, t_jet_phi, t_jet_E, "JET");
    Forest[t_jet_type]->SetBranchAddress("JET_tau1",&t_jet_tau1);
    Forest[t_jet_type]->SetBranchAddress("JET_tau2",&t_jet_tau2);
    Forest[t_jet_type]->SetBranchAddress("JET_tau3",&t_jet_tau3);
    setup_four_vector_output(OutTree,cand_jet_filt_pt, cand_jet_filt_eta,cand_jet_filt_phi, cand_jet_filt_E, "jet_filt");
    setup_four_vector_output(OutTree,cand_jet_smear_pt, cand_jet_smear_eta,cand_jet_smear_phi, cand_jet_smear_E, "jet_smear");
    setup_four_vector_output(OutTree,cand_jet_sup_pt, cand_jet_sup_eta,cand_jet_sup_phi, cand_jet_sup_E, "jet_sup");
    setup_four_vector_output(OutTree,cand_jet_sdown_pt, cand_jet_sdown_eta,cand_jet_sdown_phi, cand_jet_sdown_E, "jet_sdown");
    setup_four_vector_output(OutTree,cand_jet_rup_pt, cand_jet_rup_eta,cand_jet_rup_phi, cand_jet_rup_E, "jet_rup");
    setup_four_vector_output(OutTree,cand_jet_rdown_pt, cand_jet_rdown_eta,cand_jet_rdown_phi, cand_jet_rdown_E, "jet_rdown");
  
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
  OutTree.Branch("mu1_pt",&mu1_pt);
  OutTree.Branch("mu1_eta",&mu1_eta);
  OutTree.Branch("mu2_pt",&mu2_pt);
  OutTree.Branch("mu2_eta",&mu2_eta);
  
  OutTree.Branch("vtx_n",&vtx_n);
  OutTree.Branch("vtx_chi2",&vtx_chi2);
  OutTree.Branch("vtx_ntrk",&vtx_ntrk);
  
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
  OutTree.Branch("jet_n",&jet_n);
  OutTree.Branch("jet_z",&z);
  // OutTree.Branch("jet_emfrac",&emfrac);
  OutTree.Branch("jet_m",&cand_jet_m);
  OutTree.Branch("delta_r",&delta_r);
  setup_four_vector_output(OutTree,jpsi_pt, jpsi_eta, jpsi_phi, jpsi_E, "jpsi");

  int has_trigger=0, has_num_jets=0, has_jpsi_pt=0, /*has_jpsi_eta=0,*/
    has_mumu_eta,has_mumu_pt,has_jpsi_rap,has_jet_eta=0, has_delta_r=0, has_jet_pt=0;

#ifdef __ANALYZE_TREE_CUTFLOW__  
  OutTree.Branch("mu_trigger_p",&has_trigger);
  OutTree.Branch("num_jets_p",&has_num_jets);
  OutTree.Branch("jpsi_pt_p",&has_jpsi_pt);
  OutTree.Branch("jpsi_rap_p",&has_jpsi_rap);
  OutTree.Branch("mumu_eta_p",&has_mumu_eta);
  OutTree.Branch("mumu_pt_p",&has_mumu_pt);
  // OutTree.Branch("jpsi_eta_p",&has_jpsi_eta);
  OutTree.Branch("delta_r_p", &has_delta_r);
  OutTree.Branch("jet_eta_p",&has_jet_eta);
  OutTree.Branch("jet_pt_p",&has_jet_pt);
#endif
  double SF(1.),SFSystErr(1.),SFStatErr(1.),SFTotalErr(1.);
  double puw(-1);
  double w=weight;
  OutTree.Branch("weight", &w);
  OutTree.Branch("pileup_weight", &puw);
  OutTree.Branch("SF", &SF);
  OutTree.Branch("SFSystErr", &SFSystErr);
  OutTree.Branch("SFStatErr", &SFStatErr);
  OutTree.Branch("SFTotalErr", &SFTotalErr);

  Long64_t nEntries = Forest["AUX"]->GetEntries();
  if(verbose) {
    MSG("Got "<<nEntries<< " entries in input tree");
  }
  std::vector<TLorentzVector> jets;
  std::vector<TLorentzVector> t_jets;
  TLorentzVector candJet(0,0,0,0);
  TLorentzVector candTruthJet(0,0,0,0);
  TLorentzVector candJPsi(0,0,0,0);
  size_t idx=0;
  TLorentzVector mu1(0,0,0,0);
  TLorentzVector mu2(0,0,0,0);
  TLorentzVector nom_mu1(0,0,0,0);
  TLorentzVector nom_mu2(0,0,0,0);
  TLorentzVector jpsi_mu;
  TLorentzVector jpsi_nom_mu;
  double f[4];
  size_t jpsi_idx=0;
  MSG("Weight: "<<weight);
  for(Long64_t entry=0; entry < nEntries; entry++){
    retrieve_values(Forest,entry);
    CutDefCat["nominal"].pass();
    
    if(entry%squawk_every==0 && verbose){
      MSG("Processing entry "<<entry);
    }
    trigger_cat=0;
    idx=0; delta_r=-1.; z=-1.; jpsi_idx=0;
    // SF=1; SFSystErr=0.; SFStatErr=0.; SFTotalErr=0.;
    if(purw!=nullptr && is_MC){
      puw=purw->GetCombinedWeight(0,0,pileup);
      // MSG_DEBUG("Good Weight: "<<puw);
      if(puw < 0 ){
	MSG_DEBUG("Negative pileup re-weight, skipping event!");
	continue;
      }
    }
    else{
      puw=1;
    }
    if(mu_pt->size()==0){
      continue;
    }
    SF=total_scale_factor(MuSF);
    SFStatErr=total_scale_factor(MuSFStatErr);
    SFSystErr=total_scale_factor(MuSFSystErr);
    SFTotalErr=total_scale_factor(MuSFTotalErr);
    
    jets.clear(); jets.reserve(jet_pt->size());
    has_trigger = CutDefCat["trigger"].pass(is_MC || passed_trigger(*EF_trigger_names),w);
    jet_n = int(jet_pt->size());
    has_num_jets = CutDefCat["num_jets"].pass(jet_n,w);
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
    if(mu1_idx >= int(mu_pt->size())){
      MSG_DEBUG("WARNING: Muon 1 index from J/\\psi is out of bounds "<<mu1_idx<<" vs "<<mu_pt->size()<<" !");
      continue;
    }
    if(mu2_idx >= int(mu_pt->size())){
      MSG_DEBUG("WARNING: Muon 2 index from J/\\psi is out of bounds "<<mu2_idx<<" vs "<<mu_pt->size()<<" !");
      continue;
    }
    if(mu_pt->at(mu2_idx) > mu_pt->at(mu1_idx)){
      std::swap(mu2_idx,mu1_idx);
    }
    double mu_max_eta=std::max(fabs(mu_eta->at(mu1_idx)),fabs(mu_eta->at(mu2_idx)));
    has_mumu_eta = CutDefReal["mumu_eta"].pass(mu_max_eta,w);
    CUT_CONTINUE(has_mumu_eta);
    double mu_min_pt = std::min(mu_pt->at(mu1_idx),mu_pt->at(mu2_idx));
    has_mumu_pt = CutDefReal[ "mumu_pt" ].pass(mu_min_pt,w);
    CUT_CONTINUE(has_mumu_pt);
    mu1.SetPtEtaPhiM(mu_pt->at(mu1_idx),mu_eta->at(mu1_idx),mu_phi->at(mu1_idx),0.105658);
    mu2.SetPtEtaPhiM(mu_pt->at(mu2_idx),mu_eta->at(mu2_idx),mu_phi->at(mu2_idx),0.105658);
    mu1_pt=mu1.Pt()*GeV;
    mu1_eta=mu1.Eta();
    mu2_pt=mu2.Pt()*GeV;
    mu2_eta=mu2.Eta();
    if(std::string(muon_variation)!="" && std::string(muon_variation).find("Efficiency")==std::string::npos){
      nom_mu1.SetPtEtaPhiM(mu_nom_pt->at(mu1_idx),mu_eta->at(mu1_idx),mu_phi->at(mu1_idx),0.105658);
      nom_mu2.SetPtEtaPhiM(mu_nom_pt->at(mu2_idx),mu_eta->at(mu2_idx),mu_phi->at(mu2_idx),0.105658);
      jpsi_mu=mu1+mu2;
      jpsi_nom_mu = nom_mu1+nom_mu2;
      f[0]=jpsi_mu.Px()/jpsi_nom_mu.Px();
      f[1]=jpsi_mu.Py()/jpsi_nom_mu.Py();
      f[2]=jpsi_mu.Pz()/jpsi_nom_mu.Pz();
      f[3]=jpsi_mu.E()/jpsi_nom_mu.E();
    }else{
      f[0]=f[1]=f[2]=f[3]=1;
    }
    candJPsi.SetPxPyPzE(f[0]*vtx_px->at(jpsi_idx)*GeV, f[1]*vtx_py->at(jpsi_idx)*GeV, f[2]*vtx_pz->at(jpsi_idx)*GeV, f[3]*vtx_e->at(jpsi_idx)*GeV);
    
    jpsi_pt=candJPsi.Pt();
    jpsi_eta=candJPsi.Eta();
    jpsi_rap=candJPsi.Rapidity();
    jpsi_phi=candJPsi.Phi();
    jpsi_E=candJPsi.E();
    jpsi_m=candJPsi.M();
    has_jpsi_pt=CutDefReal["jpsi_pt"].pass(jpsi_pt,w);
    has_jpsi_rap=CutDefReal["jpsi_rap"].pass(fabs(jpsi_rap),w);
    CUT_CONTINUE(has_jpsi_pt);
    CUT_CONTINUE(has_jpsi_rap);
    delta_r=find_closest(jets,candJet,candJPsi,idx);
    jpsi_lxy = (jpsi_idx < vtx_lxy->size() ) ? vtx_lxy->at(jpsi_idx).at(0) : -99999.;
    jpsi_vtx_z = (jpsi_idx < vtx_z->size() ) ? vtx_z->at(jpsi_idx) : -99999.;
    jpsi_tau = jpsi_lxy*(3096.915*GeV)/jpsi_pt;
    vtx_chi2 = (jpsi_idx < pvtx_chi2->size() ) ? pvtx_chi2->at(jpsi_idx) : -99999.;
    vtx_ntrk = (jpsi_idx < pvtx_ntrk->size() ) ? pvtx_ntrk->at(jpsi_idx) : -1;
    vtx_n = pvtx_chi2->size();

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

    // emfrac=jet_emfrac->at(idx);
    tau1=jet_tau1->at(idx);
    tau2=jet_tau2->at(idx);
    tau3=jet_tau3->at(idx);

    tau32= (tau3*tau2 > 0) ? tau3/tau2 : -1.;
    tau21= (tau2*tau1 > 0) ? tau2/tau1 : -1.;

    cand_jet_m = candJet.M();
    
    store_four_vector(candJet,cand_jet_pt,cand_jet_eta,cand_jet_phi,cand_jet_E);

    if(is_MC){
      if(jet_type=="TrackZJPsiJets"){
	if(idx < jet_filt_pt->size()){
	  cand_jet_filt_pt = jet_filt_pt->at(idx)*GeV;
	  cand_jet_filt_eta = jet_filt_eta->at(idx)*GeV;
	  cand_jet_filt_phi = jet_filt_phi->at(idx)*GeV;
	  cand_jet_filt_E = jet_filt_E->at(idx)*GeV;
	}
	if(idx < jet_smear_pt->size()){
	  cand_jet_smear_pt = jet_smear_pt->at(idx)*GeV;
	  cand_jet_smear_eta = jet_smear_eta->at(idx)*GeV;
	  cand_jet_smear_phi = jet_smear_phi->at(idx)*GeV;
	  cand_jet_smear_E = jet_smear_E->at(idx)*GeV;
	}
	if(idx < jet_sup_pt->size()){
	  cand_jet_sup_pt = jet_sup_pt->at(idx)*GeV;
	  cand_jet_sup_eta = jet_sup_eta->at(idx)*GeV;
	  cand_jet_sup_phi = jet_sup_phi->at(idx)*GeV;
	  cand_jet_sup_E = jet_sup_E->at(idx)*GeV;
	}
	if(idx < jet_sdown_pt->size()){
	  cand_jet_sdown_pt = jet_sdown_pt->at(idx)*GeV;
	  cand_jet_sdown_eta = jet_sdown_eta->at(idx)*GeV;
	  cand_jet_sdown_phi = jet_sdown_phi->at(idx)*GeV;
	  cand_jet_sdown_E = jet_sdown_E->at(idx)*GeV;
	}
	if(idx < jet_rup_pt->size()){
	  cand_jet_rup_pt = jet_rup_pt->at(idx)*GeV;
	  cand_jet_rup_eta = jet_rup_eta->at(idx)*GeV;
	  cand_jet_rup_phi = jet_rup_phi->at(idx)*GeV;
	  cand_jet_rup_E = jet_rup_E->at(idx)*GeV;
	}
	if(idx < jet_rdown_pt->size()){
	  cand_jet_rdown_pt = jet_rdown_pt->at(idx)*GeV;
	  cand_jet_rdown_eta = jet_rdown_eta->at(idx)*GeV;
	  cand_jet_rdown_phi = jet_rdown_phi->at(idx)*GeV;
	  cand_jet_rdown_E = jet_rdown_E->at(idx)*GeV;
	}
      }
      size_t t_idx=-1;
      t_jpsi_pt*=GeV;
      t_jpsi_E*=GeV;
      TLorentzVector tvec(0,0,0,0);
      tvec.SetPtEtaPhiE(t_jpsi_pt,t_jpsi_eta,t_jpsi_phi, t_jpsi_E);
      t_jpsi_rap=tvec.Rapidity();
      t_jpsi_m=tvec.M();
      size_t n_jets = t_jet_pt->size();
      t_jets.clear();
      t_jets.reserve(t_jet_pt->size());
      for(size_t i = 0; i < t_jet_pt->size(); i++){
	TLorentzVector tmp_vec(0,0,0,0);
	tmp_vec.SetPtEtaPhiE(t_jet_pt->at(i)*GeV,
			     t_jet_eta->at(i),
			     t_jet_phi->at(i),
			     t_jet_E->at(i)*GeV);
	t_jets.push_back(tmp_vec);
      }
      find_closest(t_jets,candTruthJet,candJet,t_idx);
      if(t_idx>=n_jets){
	MSG_ERR("No matched truth jet!");
	continue;
      }
      t_delta_r = candTruthJet.DeltaR(tvec);
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
      t_tau1=t_jet_tau1->at(t_idx);
      t_tau2=t_jet_tau2->at(t_idx);
      t_tau3=t_jet_tau3->at(t_idx);
      t_tau32=t_tau3/t_tau2;
      t_tau21=t_tau2/t_tau1;
      t_idx=-1;
    }
    OutTree.Fill();
  }
  // Clean up the forest
  for(tree_collection::iterator tree = Forest.begin(); tree != Forest.end(); ++tree){
    tree->second->ResetBranchAddresses();
  }
  return 0;
}
