#pragma once
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <algorithm>
class Selector{
public:
  virtual bool pass(const double w) = 0;
  // virtual unsigned int count() = 0;
  virtual void reset() = 0;
};


template<class cut_type>
class cut : public Selector{
public:
  bool operator== (const cut_type& obs){
    return m_cut_val == obs;
  }
  bool operator!= (const cut_type& obs){
    return m_cut_val != obs;
  }
  bool operator< (const cut_type& obs){
    return m_cut_val < obs;
  }
  bool operator>= (const cut_type& obs){
    return !(m_cut_val < obs);
  }
  bool operator> (const cut_type& obs){
    return m_cut_val > obs;
  }
  bool operator<= (const cut_type& obs){
    return !(m_cut_val > obs);
  }

  cut():m_cut_val(),
	m_count(0),
	m_weight(0.) {};

  cut(cut_type cut_val, std::string cut_op_str,
      unsigned int count=0, 
      double weight=0.):
    m_cut_val(cut_val),
    m_count(count),
    m_weight(weight) {

    if(cut_op_str=="=="){
      m_comp_fn=&cut<cut_type>::eq;
    }
    else if(cut_op_str=="!="){
      m_comp_fn=&cut<cut_type>::neq;
    }
    else if(cut_op_str==">="){
      m_comp_fn=&cut<cut_type>::greater_than_eq;
    }
    else if(cut_op_str=="<="){
      m_comp_fn=&cut<cut_type>::less_than_eq;
    }
    else if(cut_op_str=="<"){
      m_comp_fn=&cut<cut_type>::less_than;
    }
    else if(cut_op_str==">"){
      m_comp_fn=&cut<cut_type>::greater_than;
    }

  };
  
  ~cut(){};
  bool pass(const double w=1.){
    m_count++;
    m_weight+=w;
    return true;
  }
  bool pass(const cut_type obs_val,const double w=1.){
    bool result = (this->*m_comp_fn)(obs_val);
    if(result){
      m_count++;
      m_weight+=w;
    }
    return result;
  }
  cut_type cut_value() const {return m_cut_val;}
  unsigned int count() const {return m_count;}
  double weight() const {return m_weight;}
  void reset(){ m_count=0; m_weight=0; }
  bool operator==(const cut &other) const {
    return (m_cut_val == other.m_cut_val &&
	    m_weight == other.m_weight &&
	    m_count == other.m_count);
  }
  bool operator!=(const cut &other) const {
    return !(*this == other);
  }
  
private:
  bool (cut<cut_type>::*m_comp_fn)(const cut_type& obs);
  cut_type m_cut_val;
  unsigned int m_count;
  double m_weight;
  bool eq(const cut_type& obs_val){
    return m_cut_val==obs_val;
  }
  bool neq(const cut_type& obs_val){
    return m_cut_val!=obs_val;
  }
  bool less_than(const cut_type& obs_val){
    return obs_val < m_cut_val;
  }
  bool less_than_eq(const cut_type& obs_val){
    return obs_val <= m_cut_val;
  }
  bool greater_than(const cut_type& obs_val){
    return obs_val > m_cut_val;
  }
  bool greater_than_eq(const cut_type& obs_val){
    return obs_val >= m_cut_val;
  }
};

typedef std::map<std::string, cut<double> > real_cuts;
typedef std::map<std::string, cut<int> > category_cuts;
void print_cut_summary(std::string CutName, cut<int> Cut);
void print_cut_summary(std::string CutName, cut<double> Cut);
void print_cut_table(real_cuts& CutDefReals,category_cuts& CutDefCats);
class cut_container{
public:
  Selector& operator[](std::string key){return m_real_cuts[key]; };
  void print_cut_table(){};
private:
  real_cuts m_real_cuts;
  category_cuts m_cat_cuts;
  
};
