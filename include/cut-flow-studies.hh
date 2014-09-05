#ifndef CUT_FLOW_STUDIES_HH
#define CUT_FLOW_STUDIES_HH

#include <map>
#include "Cut.hh"
class TTree;

typedef std::map<std::string, cut<double> > real_cuts;
typedef std::map<std::string, cut<int> > category_cuts;
typedef std::map<std::string,TTree*> tree_collection;
int process_tree(tree_collection& Forest, real_cuts& CutDefReal, 
		 category_cuts& CutDefCat);
void print_cut_summary(std::string CutName, cut<int> Cut);
void print_cut_summary(std::string CutName, cut<double> Cut);
#endif
