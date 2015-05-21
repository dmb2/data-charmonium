#pragma once
#include <map>
#include <string>

class TH1;
class TLegend;
class TTree;
class THStack;

void scale_stack(TH1** hist_list, const size_t n, double sf);
void norm_stack(THStack& stack);
THStack* make_stack(TH1* base_hist, std::map<std::string,TTree*>& samples,
		    const std::vector<std::string>& cut_branches, const int cut_index, 
		    const std::string& plot, TLegend& leg, 
		    const double target_lumi=22.7);
void print_2D_stack(std::map<std::string,TTree*> samples,const std::string& plot,
		    TH1* base_hist, const std::string& suffix, 
		    const double target_lumi);
void print_stack(std::map<std::string,TTree*> samples,const std::string& plot,
		 TH1* base_hist, const std::string& suffix, 
		 const double target_lumi=22.7,
		 const std::vector<std::string>& cut_branches=std::vector<std::string>());
void print_cut_stack(std::map<std::string,TTree*>& samples, 
		     const std::vector<std::string>& cut_branches,
		     const std::string& plot, TH1* base_hist, 
		     std::map<std::string,std::string>& CutNames, 
		     const std::string& file_suffix, const double target_lumi=22.7);
void print_2D_slices(std::map<std::string,TTree*> samples,const std::string& plot,
		     TH1* base_hist, const std::string& suffix, 
		     const double target_lumi,bool norm=false);
