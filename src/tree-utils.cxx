#include <iostream>

#include "root-sugar.hh"
#include "tree-utils.hh"
#include "Units.hh"

using namespace Units;

void retrieve_values(tree_collection& forest, Long64_t entry){
  for(tree_collection::iterator it=forest.begin(); it != forest.end(); ++it){
    if(it->second){
      it->second->GetEntry(entry);
    }
    else{
      MSG_ERR("Could not retrieve entry: "<< entry<<" from "<<it->first);
      exit(-2);
    }
  }
}
void initialize_forest(tree_collection& forest){
  for(tree_collection::iterator it=forest.begin(); it!=forest.end(); ++it){
    it->second->SetCacheSize(10*MB);
    it->second->SetCacheLearnEntries(5);
  }
}
double find_closest(const std::vector<double>& pt,
		    const std::vector<double>& eta,
		    const std::vector<double>& phi,
		    const std::vector<double>& E,
		    TLorentzVector& closest,
		    TLorentzVector& axis, size_t& idx){
  double dR(0.);
  double DeltaR(99.);
  TLorentzVector ptcl(0,0,0,0);
  for(size_t i=0; i < pt.size(); i++){
    if(pt.at(i)*GeV < 20 || fabs(eta.at(i)) > 4.5){
      continue;
    }
    ptcl.SetPtEtaPhiE(pt.at(i)*GeV, eta.at(i), phi.at(i), E.at(i)*GeV);
    dR=axis.DeltaR(ptcl);
    if(dR < DeltaR){
      DeltaR = dR;
      closest=ptcl;
      idx=i;
    }
  }
  return DeltaR;
}
double find_closest(const std::vector<TLorentzVector>& jets,
		    TLorentzVector& closest,
		    TLorentzVector& axis, size_t& idx){
  double dR(0.);
  double DeltaR(99.);
  size_t i=0;
  for(std::vector<TLorentzVector>::const_iterator jet = jets.begin();
      jet != jets.end(); ++jet,++i){

    dR = axis.DeltaR(*jet);
    if (dR < DeltaR){
      DeltaR = dR;
      closest=*jet;
      idx=i;
    }
  }
  return DeltaR;
}
std::vector<size_t> filter_by_pt(const std::vector<double>& pt,
				 const double ptMin){
  //return a vector of indices when pt is bigger than ptMin
  std::vector<size_t> passed_indices;
  if(pt.size()==0){
    return passed_indices;
  }
  // MSG_DEBUG(pt.size());
  passed_indices.reserve(pt.size());
  for(size_t i=0; i < pt.size(); i++){
    if(pt.at(i)*GeV > ptMin){
      // MSG_DEBUG(pt.at(i)*GeV<<"> ptMin: "<< ptMin<<" "<<i);
      passed_indices.push_back(i);
    }
  }
  return passed_indices;
}
int passed_trigger(std::vector<std::string>& trigger_names){
  //cf https://twiki.cern.ch/twiki/bin/view/Atlas/MuonTriggerPhysicsTriggerRecommendations2012
  for(std::vector<std::string>::const_iterator name=trigger_names.begin();
      name!=trigger_names.end(); ++name){
    if(*name == "EF_mu36_tight" || *name == "EF_mu24i_tight"){
      return 1;
    }
  }
  return 0; 
}
std::vector<TLorentzVector> buildMuons(const std::vector<double>* pt, 
				       const std::vector<double>* eta,
				       const std::vector<double>* phi, 
				       const std::vector<double>* e){
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
  TLorentzVector cand;
  for(size_t i = 0; i < muons.size(); i++){
    for(size_t j = i; j < muons.size(); j++){
      cand = muons.at(i) + muons.at(j);
      if(charge.at(i)*charge.at(j) < 0 && cand.M() > 2e3 && cand.M() < 6e3 ){
	dimuon_pairs.push_back(cand);
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
double get_impact_sig(const std::vector<double>& d0, const std::vector<double>& d0_err,
		      const std::vector<int>& idx){
  if(idx.size() != 2){
    MSG_ERR("Expected two track indices, got: "<<idx.size()<<" returning 99.");
    return 99;
  }
  return pow(d0[idx[0]]/d0_err[idx[0]],2)+pow(d0[idx[1]]/d0_err[idx[1]],2);
}
