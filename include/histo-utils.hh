#include <map>
#include <string>
#include <vector>

class TH1;
class TAxis;
class TH2D;
class TTree;
class TLegend;
class TCanvas;
class TVirtualPad;

template<class T1,class T2>
inline std::vector<T1> map_keys(std::map<T1,T2>& inputMap){
  std::vector<T1> keys;
  for(typename std::map<T1,T2>::const_iterator i=inputMap.begin();
      i!=inputMap.end(); ++i){
    keys.push_back(i->first);
  }
  return keys;
}
template<class T1,class T2>
inline std::vector<T2> map_values(std::map<T1,T2>& inputMap){
  std::vector<T2> values;
  for (typename std::map<T1,T2>::const_iterator i=inputMap.begin();
       i!=inputMap.end(); ++i){
    values.push_back(i->second);
  }
  return values;
}
enum status_t {
  PRELIMINARY,
  APPROVED,
  INTERNAL,
  WIP};
void setup_global_style();
void setup_hist(TH1* hist);
TH2D* setup_res_vtxz_hist(TH1* hist);
TH2D* setup_res_dif_hist(TH1* hist);
TH2D* setup_rel_res_hist(TH1* hist);
TH2D* setup_response_hist(TH1* hist);
void draw_histo(TTree* tree,const char* branch_name, const char* hist_name, 
		const char* cut_expr);
TLegend* init_legend(double x1=0.70,double y1=0.60, double x2=0.92,double y2=0.92);
void remove_axis(TAxis* axis);
void set_pad_margins(TVirtualPad* pad,int pad_pos,int N_hists,int n_col=3,int n_row=2,bool y_axis=true);
TH1* make_response_hist(TH1* base_hist, TTree* tree, 
			const std::vector<std::string>& cut_branches,size_t cut_index, 
			const std::string& plot);
TH1* make_normal_hist(TH1* base_hist, TTree* tree, 
		      const std::vector<std::string>& cut_branches, size_t cut_index, 
		      const std::string& plot);
TH1* make_ratio_hist(TH1* base_hist, TTree* tree,
		     const std::vector<std::string>& cut_branches, 
		     size_t cut_index, const std::string& plot);
TH1* make_res_vtxz_hist(TH1* base_hist,TTree* tree,const std::string& plot,
			const char* weight_expr, const std::string& name_suffix);
TH1* make_res_dif_hist(TH1* base_hist,TTree* tree,const std::string& plot,
		       const char* weight_expr, const std::string& name_suffix);
TH1* make_rel_res_hist(TH1* base_hist,TTree* tree,const std::string& plot,
		      const char* weight_expr="weight",
		      const std::string& name_suffix="_NOM");
TH1* make_response_hist(TH1* base_hist,TTree* tree,const std::string& plot,
		      const char* weight_expr="weight",
		      const std::string& name_suffix="_NOM");
TH1* make_normal_hist(TH1* base_hist,TTree* tree,const std::string& plot,
		      const char* weight_expr="weight",
		      const std::string& name_suffix="_NOM");
void scale_errors(TH1* hist);

void add_err(TH1* hista, TH1* histb);
bool has_non_zero_error(TH1* hist);

TH1* build_syst_err_hist(TH1* base_hist, const std::string& samp_name,
			 const char* cut_expr);

void add_atlas_badge(TVirtualPad& canv,const double x, const double y, 
		     const double lumi_fb, const status_t status=INTERNAL);
void print_profile_hist(TH1* base_hist,TTree* tree,const std::string& plot,
			const std::string& suffix,
			TH1* (*make_hist)(TH1*,TTree*,
					  const std::string&,const char*,
					  const std::string&));
void print_hist(TTree* tree, const std::string& plot, 
		TH1* base_hist, const std::vector<std::string>& cut_branches,
		const std::string suffix, 
		TH1* (*make_hist)(TH1*,TTree*,const std::string&, const char*,
				  const std::string&));
void print_cut_hist(TTree* tree, const std::vector<std::string>& cut_branches,
		    const std::string& plot, TH1* base_hist, 
		    std::map<std::string,std::string>& CutNames, std::string file_suffix,
		    TH1* (*make_hist)(TH1* ,TTree* , 
				      const std::vector<std::string>&, size_t,
				      const std::string&));
