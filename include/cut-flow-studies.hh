#pragma once

#include "tree-utils.hh"
class TTree;

int process_tree(tree_collection& Forest, real_cuts& CutDefReal, 
		 category_cuts& CutDefCat, TTree& OutTree, 
		 const char* muon_system, const std::string& jet_type, const double weight);
/*
int process_tree(tree_collection& Forest, real_cuts& CutDefReal, 
		 category_cuts& CutDefCat, TTree& OutTree,const double weight=1.);
*/
