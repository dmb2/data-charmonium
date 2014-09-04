#include <map>
class TTree;

template<class cut_type>
class cut{
public:
  cut():m_cut_val(),
	m_count(0),
	m_weight(0.) {};

  cut(cut_type cut_val, 
      unsigned int count=0, 
      double weight=0.):
    m_cut_val(cut_val),
    m_count(count),
    m_weight(weight){};
  ~cut(){};
  void pass(double w=1.){
    m_count++;
    m_weight+=w;
  }
  cut_type cut_value() const {return m_cut_val;}
  unsigned int count() const {return m_count;}
  double weight() const {return m_weight;}

private:
  cut_type m_cut_val;
  unsigned int m_count;
  double m_weight;
};
typedef std::map<std::string, cut<double> > real_cuts;
typedef std::map<std::string, cut<int> > category_cuts;
typedef std::map<std::string,TTree*> tree_collection;
int process_tree(tree_collection Forest, real_cuts CutDefReal, 
		 category_cuts CutDefCat);
