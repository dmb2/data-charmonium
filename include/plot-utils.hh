#pragma once
#include <map>
#include <string>

class TH1;
class TLegend;
class TTree;
class TPad;
TPad* split_canvas(TPad* canvas, float fraction);
void draw_ratios(TPad* pad,TList* hist_list);
void print_ratio_hist(std::map<std::string,TTree*>& samples, const std::string& plot,
		      TH1* base_hist, const std::string& suffix, 
		      const double target_lumi=22.7);
