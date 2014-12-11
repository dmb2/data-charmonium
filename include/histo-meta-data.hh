#include <map>
#include <string>
class TH1D;

void init_hist_book(std::map<std::string,TH1D*>& HistBook);
void init_cut_names(std::map<std::string,std::string>& cut_names);
void init_leg_names(std::map<std::string,std::string>& leg_map);
void init_colors(std::map<std::string,int>& colors);
