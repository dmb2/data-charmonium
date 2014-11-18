#include <iostream>

#include "tree-utils.hh"
#include "Units.hh"

using namespace Units;

void retrieve_values(tree_collection& forest, Long64_t entry){
  for(tree_collection::iterator it=forest.begin(); it != forest.end(); ++it){
    it->second->GetEntry(entry);
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
    // if(pt.at(i)*GeV < 20 || fabs(eta.at(i)) > 4.5){
    //   continue;
    // }
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
