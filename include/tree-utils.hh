#pragma once

#include <string>
#include <map>
#include <vector>

#include "Cut.hh"
#include "root-sugar.hh"
#include "TLorentzVector.h"
#include "TTree.h"

typedef std::map<std::string,TTree*> tree_collection;
class TH1D;

struct sample {
  double xsec;
  TTree* tree;
};


void retrieve_values(tree_collection& forest, Long64_t entry);
void initialize_forest(tree_collection& forest);
template<typename LV> 
void store_four_vector(LV vector, double& pt, double& eta, double& phi, double& E){
  TLorentzVector tmp_vec(vector[0],vector[1],vector[2],vector[3]);
  pt=tmp_vec.Pt();
  eta=tmp_vec.Eta();
  phi=tmp_vec.Phi();
  E=tmp_vec.E();
};
template<typename T>
void setup_four_vector(TTree* tree, T& p0, T& p1, T& p2, T& p3, const char* key, const char* vec_names[]){
  char branch_name[50];
  snprintf(branch_name, 50,"%s_%s",key,vec_names[0]);
  tree->SetBranchAddress(branch_name, &p0);
  snprintf(branch_name, 50,"%s_%s",key,vec_names[1]);
  tree->SetBranchAddress(branch_name, &p1);
  snprintf(branch_name, 50,"%s_%s",key,vec_names[2]);
  tree->SetBranchAddress(branch_name, &p2);
  snprintf(branch_name, 50,"%s_%s",key,vec_names[3]);
  tree->SetBranchAddress(branch_name, &p3);
}
template<typename T>
void setup_pt_eta_phi_e(TTree* tree, T& pt, T& eta, T& phi, T& E, const char* key, bool upperE=false){
  const char* names[] = {"pt","eta","phi",upperE ? "E": "e"};
  setup_four_vector(tree,pt,eta,phi,E,key,names);
}
template<typename T>
void setup_px_py_pz_e(TTree* tree, T& px, T& py, T& pz, T& E,const char* key, bool upperE=false){
  const char* names[] = {"px","py","pz",upperE ? "E": "e"};
  setup_four_vector(tree,px,py,pz,E,key,names);
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
double find_closest(const std::vector<TLorentzVector>& jets,
		    TLorentzVector& closest,
		    TLorentzVector& axis, size_t& idx);
// double find_closest(const std::vector<double>& pt,
// 		    const std::vector<double>& eta,
// 		    const std::vector<double>& phi,
// 		    const std::vector<double>& E,
// 		    TLorentzVector& closest,
// 		    TLorentzVector& axis, size_t& idx);
std::vector<size_t> filter_by_pt(const std::vector<double>& pt,
				 const double ptMin);
int passed_trigger(std::vector<std::string>& trigger_names);
std::vector<TLorentzVector> buildMuons(const std::vector<double>* pt, 
				       const std::vector<double>* eta,
				       const std::vector<double>* phi, 
				       const std::vector<double>* e);
std::pair<TLorentzVector,TLorentzVector> buildJPsiCand(const std::vector<TLorentzVector>& muons,
			     const std::vector<int>& charge);
double get_impact_sig(const std::vector<double>& d0, 
		      const std::vector<double>& d0_err,
		      const std::vector<int>& idx);
double total_scale_factor(const std::vector<double>* scale_factors);

TTree* skim_tree(TTree* tree, std::map<std::string,TH1D*> HistBook,const char* variables[],size_t n_var);
