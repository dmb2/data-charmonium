#include "TMatrixD.h"

class TH2F;
class TH2;
class TH1D;


TMatrixD norm_hist_to_mat(TH2* h2);
TH1D* fold_truth(TH1D* truth, const TMatrixD& response_matrix, int n_events);
void print_closure_plot(TH2* response_hist, TH1D* truth, TH1D* reco, TH1D* unfolded, const std::string& suffix);
void unfold_toy(TH2* response_hist, TH1D* reco, TH1D* truth, int n_itr,
		const std::string& suffix);
TH2F* linear_response_toy(TH1D* base_hist,int n_evts);
// TH2F* quad_response_toy(double x_min,double x_max);
// TH2F* width_response_toy(double x_min,double x_max);
