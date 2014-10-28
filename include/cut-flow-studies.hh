#pragma once

#include "tree-utils.hh"
#include "Cut.hh"
class TTree;

int process_tree(tree_collection& Forest, real_cuts& CutDefReal, 
		 category_cuts& CutDefCat, TTree& OutTree);
void print_cut_summary(std::string CutName, cut<int> Cut);
void print_cut_summary(std::string CutName, cut<double> Cut);
void print_cut_table(real_cuts& CutDefReals,category_cuts& CutDefCats, const char* CutNames[],size_t nCuts);
