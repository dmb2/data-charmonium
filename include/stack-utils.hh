#pragma once
#include <map>
#include <string>

class TH1;
class TLegend;
class TTree;
class THStack;

void scale_stack(TH1** hist_list, const size_t n, double sf);
void norm_stack(THStack& stack);
void style_hist(TH1* hist, TLegend* leg, const int color, const char* leg_name);
THStack* make_stack(TH1* base_hist, std::map<std::string,TTree*>& samples,
		    const char* cut_branches[], const int cut_index, 
		    const std::string& plot, TLegend& leg, 
		    const double target_lumi=22.7);
void print_2D_stack(std::map<std::string,TTree*> samples,const std::string& plot,
		    TH1* base_hist, const std::string& suffix, 
		    const double target_lumi);
void print_stack(std::map<std::string,TTree*> samples,const std::string& plot,
		 TH1* base_hist, const std::string& suffix, 
		 const double target_lumi=22.7,
		 const char* cut_branches[]=NULL, const size_t nCuts=0);
void print_cut_stack(std::map<std::string,TTree*>& samples, 
		     const char* cut_branches[],const size_t nCuts, 
		     const std::string& plot, TH1* base_hist, 
		     const std::map<std::string,std::string>& CutNames, 
		     const std::string file_suffix, const double target_lumi=22.7);
void print_2D_slices(std::map<std::string,TTree*> samples,const std::string& plot,
		     TH1* base_hist, const std::string& suffix, 
		     const double target_lumi);
