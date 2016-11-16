#include "TMatrixD.h"

class TH2F;
class TH2;
class TH1;
class TH1D;
class TTree;

TMatrixD norm_hist_to_mat(TH2* h2);
TH1D* fold_truth(TH1D* truth,  const TMatrixD& response_matrix);
void print_closure_plot(TH2* response_hist, TH1D* truth, 
			TH1D* reco, TH1* unfolded, const std::string& suffix);

void unfold_toy(TH2* (*make_response)(TH1*,TTree*,const int),
		TH1D* make_truth(TH1D*,TTree*,const int),
		TH1D* base_hist, TTree* tree, 
		const int n_itr, const int n_evts,
		const std::string& suffix);
TH1* unfold(TH2* response_hist, TH1* reco, int n_itr, const std::string& name);
int get_iterations(TH1* base_hist,TH2* response_hist,const int n_evts);

TH1D* mc_truth(TH1D* base_hist, TTree* tree, const int n_evts);
TH1D* gauss_truth(TH1D* base_hist, TTree* tree, const int n_evts);
TH1D* dbl_gauss_truth(TH1D* base_hist, TTree* tree, const int n_evts);

TH2* mc_response(TH1* base_hist, TTree* tree, int n_evts);
TH2* linear_response_toy(TH1* base_hist,TTree* tree,const int n_evts);
TH2* quad_response_toy(TH1* base_hist,TTree* tree, const int n_evts);
TH2* width_response_toy(TH1* base_hist, TTree* tree, const int n_evts);

TH1* unfold_syst_err(TH1* reco_hist,TTree* tree, 
		     const std::string& name,
		     const char* num_iter_str,
		     const std::string& suffix);
