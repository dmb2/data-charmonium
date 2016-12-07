#include <map>
#include <string>
#include "histo-style.hh"
class TH1D;
class TH2D;
typedef std::map<std::string,TH2D*> hist_2D_book;
typedef std::map<std::string,TH1D*> hist_book;
void init_hist2D_book(hist_2D_book& Hist2DBook);
void init_hist_book(hist_book& HistBook);
void init_cut_names(std::map<std::string,std::string>& cut_names);
void init_hist_styles(std::map<std::string,aesthetic>& styles);
void fix_axis_labels(TH1* hist);
