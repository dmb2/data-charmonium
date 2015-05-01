#include <map>
#include <string>
#include "histo-style.hh"
class TH1D;
class TH2D;

void init_hist2D_book(std::map<std::string,TH2D*>& Hist2DBook);
void init_hist_book(std::map<std::string,TH1D*>& HistBook);
void init_cut_names(std::map<std::string,std::string>& cut_names);
void init_hist_styles(std::map<std::string,aesthetic>& styles);
void init_leg_names(std::map<std::string,std::string>& leg_map);
void init_colors(std::map<std::string,int>& colors);
