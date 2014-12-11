#pragma once
#include <map>
#include <string>

class TH1;
class TLegend;
class TTree;
class THStack;

void scale_stack(TH1** hist_list, const size_t n, double sf);
void style_hist(TH1* hist, TLegend* leg, const int color, const char* leg_name);
THStack* make_stack(TH1* base_hist, std::map<std::string,TTree*>& samples,
		    const char* cut_branches[], int cut_index, 
		    const std::string& plot, TLegend& leg, double n_master);
void print_stack(std::map<std::string,TTree*> samples,const std::string& plot,
		 TH1* base_hist, const std::string& suffix);
void print_cut_stack(std::map<std::string,TTree*>& samples, 
		     const char* cut_branches[],size_t nCuts, 
		     const std::string& plot, TH1* base_hist, 
		     std::map<std::string,std::string>& CutNames, std::string file_suffix);



