//System
#include <iostream>
#include <cstdlib>
#include <cstdio>
//ROOT 
#include "TFile.h"
#include "TLorentzVector.h"
#include "TTree.h"

// Fastjet
#include "fastjet/PseudoJet.hh"
#include "fastjet/ClusterSequence.hh"
#include "fastjet/contrib/Nsubjettiness.hh"
#include "fastjet/contrib/Njettiness.hh"
using fastjet::contrib::Nsubjettiness;
using fastjet::contrib::Njettiness;


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
void make_particles(const std::vector<double>& px,
		    const std::vector<double>& py,
		    const std::vector<double>& pz,
		    const std::vector<double>& E,
		    const std::vector<int>& pdgId,
		    std::vector<fastjet::PseudoJet>& particles){
  for(size_t i=0; i < px.size(); i++){
    const int pid=pdgId.at(i);
    if(abs(pid) == 12 || abs(pid)==13 || abs(pid) == 14 || abs(pid) == 16) { //neutrinos+muons
      continue;
    }
    particles.push_back(fastjet::PseudoJet(px[i]*GeV,py[i]*GeV,pz[i]*GeV,E[i]*GeV));
    particles.back().set_user_index(pid);
  }
}
double find_closest(const std::vector<fastjet::PseudoJet> jets,
		    fastjet::PseudoJet& closest,
		    const TLorentzVector& axis){
  double dR(0.);
  double DeltaR(99.);
  const fastjet::PseudoJet p_axis(axis.Px(),axis.Py(),axis.Pz(),axis.E());
  for(std::vector<fastjet::PseudoJet>::const_iterator j=jets.begin();
      j!=jets.end(); ++j){
    if(fabs(j->eta()) > 4.5){
      continue;
    }
    dR=p_axis.delta_R(*j);
    if(dR < DeltaR){
      DeltaR=dR;
      closest = *j;
    }
  }
  return DeltaR;
}
int process_tree(TTree& tree, real_cuts& CutDefReal, 
		 category_cuts& CutDefCat, TTree& OutTree,const double weight){
  unsigned int squawk_every = 1000;
  double tau1(-1),tau2(-1),tau3(-1),tau21(-1),tau32(-1);
  double z(0.), DeltaR(999.);
  double jpsi_pt(0.), jpsi_eta(0.), jpsi_phi(0.), jpsi_E(0.);

  std::vector<double> *jet_pt=NULL, *jet_eta=NULL, *jet_phi=NULL, *jet_E=NULL;

  std::vector<double> *mu_pt=NULL, *mu_eta=NULL, *mu_phi=NULL, *mu_E=NULL;
  std::vector<int> *mu_charge=NULL;

  std::vector<double> *mc_px=NULL, *mc_py=NULL, *mc_pz=NULL, *mc_E=NULL;
  std::vector<int> *mc_pdgId=NULL;
  tree.SetBranchAddress("mu_muid_charge",&mu_charge);
  tree.SetBranchAddress("mc_pdgId",&mc_pdgId);
  tree.SetBranchAddress("mc_px",&mc_px);
  tree.SetBranchAddress("mc_py",&mc_py);
  tree.SetBranchAddress("mc_pz",&mc_pz);
  tree.SetBranchAddress("mc_E",&mc_E);

  double cand_jet_pt(0.), cand_jet_eta(0.),cand_jet_phi(0.), cand_jet_E(0.);
  setup_pt_eta_phi_e(&tree, jet_pt, jet_eta, jet_phi, jet_E, "jet_AntiKt4TruthJets",false);
  setup_pt_eta_phi_e(&tree, mu_pt, mu_eta, mu_phi, mu_E, "mu_muid",false);
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
  double w(weight);
  OutTree.Branch("weight",&w);
    
  int /*has_trigger=1,*/ has_num_jets=0, has_jpsi_pt=0, has_jpsi_eta=0, 
    has_jet_eta=0, has_delta_r=0, has_jet_pt=0;
  //OutTree.Branch("mu_trig_p",&has_trigger);
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
  fastjet::PseudoJet candJet(0,0,0,0);
  TLorentzVector candJPsi(0,0,0,0);
  TLorentzVector truthCandJet(0,0,0,0);
  std::vector<fastjet::PseudoJet> particles;
  const double jetR=0.4;
  const double beta=1.0;
  Nsubjettiness OneSubJCalc(1,Njettiness::kt_axes, beta, jetR, jetR);
  Nsubjettiness TwoSubJCalc(2,Njettiness::kt_axes, beta, jetR, jetR);
  Nsubjettiness ThreeSubJCalc(3,Njettiness::kt_axes, beta, jetR, jetR);
  size_t idx(0);
  for(Long64_t entry=0; entry < nEntries; entry++){
    tree.GetEntry(entry);
    if(entry%squawk_every==0 && verbose){
      cout <<"Processing entry "<<entry<<endl;
    }
    particles.clear();
    particles.reserve(mc_px->size());
    make_particles(*mc_px,*mc_py,*mc_pz,*mc_E,*mc_pdgId, particles);
    fastjet::ClusterSequence c_seq = fastjet::ClusterSequence(particles,fastjet::JetDefinition(fastjet::antikt_algorithm,jetR));
    std::vector<fastjet::PseudoJet> jets = c_seq.inclusive_jets(20*GeV);
    DeltaR=-1.;
    z=-1.;
    find_jpsi(*mu_pt,*mu_eta,*mu_phi,*mu_E,*mu_charge,candJPsi);
    store_four_vector(candJPsi,jpsi_pt,jpsi_eta,jpsi_phi,jpsi_E);

    DeltaR=find_closest(*jet_pt,*jet_eta,*jet_phi,*jet_E, truthCandJet, candJPsi,idx);
    find_closest(jets,candJet,truthCandJet);
    store_four_vector(truthCandJet,cand_jet_pt,cand_jet_eta,cand_jet_phi,cand_jet_E);
    
    CutDefCat["nominal"].pass(0,w);//a little awkward
    
    has_num_jets=CutDefCat["num_jets"].pass(int(jets.size()),w);
    has_jpsi_pt=CutDefReal["jpsi_pt"].pass(jpsi_pt,w);
    has_jpsi_eta=CutDefReal["jpsi_eta"].pass(fabs(jpsi_eta),w);
    has_delta_r=CutDefReal["delta_r"].pass(DeltaR,w);
    has_jet_eta=CutDefReal["jet_eta"].pass(fabs(truthCandJet.Eta()),w);
    has_jet_pt=CutDefReal["jet_pt"].pass(truthCandJet.Pt(),w);
    
    z=(jpsi_pt)/(truthCandJet.Pt()+jpsi_pt);
    tau1=OneSubJCalc(candJet);
    tau2=TwoSubJCalc(candJet);
    tau3=ThreeSubJCalc(candJet);
    tau32= (tau3*tau2 > 0) ? tau3/tau2 : -1.;
    tau21= (tau2*tau1 > 0) ? tau2/tau1 : -1.;

    OutTree.Fill();
  }
  return 0;
}
