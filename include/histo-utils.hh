#include <map>
#include <string>
#include <vector>

class TH1;
class TH2D;
class TTree;

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
void setup_hist(TH1* hist);
TH2D* setup_response_hist(TH1* hist);
std::vector<std::string> add_prefix(std::string prefix, std::vector<std::string> strings);
std::string str_join(std::string base, const char* strings[],size_t start, size_t end);
void draw_histo(TTree* tree,const char* branch_name, const char* hist_name, 
		const char* cut_expr);
void remove_axis(TAxis* axis);
void set_pad_margins(TVirtualPad* pad,int pad_pos,int n_col=3,int n_row=2);
void make_roc_list(TH1* signal, TH1* background);
TH1* make_response_hist(TH1* base_hist, TTree* tree, 
			const char* cut_branches[],size_t cut_index, 
			const std::string& plot);
TH1* make_normal_hist(TH1* base_hist, TTree* tree, 
		      const char* cut_branches[], size_t cut_index, 
		      const std::string& plot);
TH1* make_ratio_hist(TH1* base_hist, TTree* tree,
		     const char* cut_branches[],size_t cut_index, 
		     const std::string& plot);
TH1* make_response_hist(TH1* base_hist,TTree* tree,const std::string& plot,
		      const char* weight_expr="weight",
		      const std::string& name_suffix="_NOM");
TH1* make_normal_hist(TH1* base_hist,TTree* tree,const std::string& plot,
		      const char* weight_expr="weight",
		      const std::string& name_suffix="_NOM");
void print_hist(TTree* tree, const std::string& plot, 
		TH1* base_hist, const std::string suffix, 
		TH1* (*make_hist)(TH1*,TTree*,const std::string&, const char*,
				  const std::string&));
void print_cut_hist(TTree* tree,const char* cut_branches[],size_t nCuts, 
		const std::string& plot, TH1* base_hist, 
		std::map<std::string,std::string>& CutNames, std::string file_suffix,
		TH1* (*make_hist)(TH1* ,TTree* , const char**, 
				  size_t, const std::string&));
