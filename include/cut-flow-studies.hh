#pragma once

#include "tree-utils.hh"
class TTree;
// #define __ANALYZE_TREE_CUTFLOW__

int process_tree(tree_collection& Forest, real_cuts& CutDefReal, 
		 category_cuts& CutDefCat, TTree& OutTree, 
		 const char* muon_system, const std::string& jet_type, const double weight);
