#include <cmath>
#include <cstdio>
template<typename T>
bool is_in(T x,T min,T max){
  return (x >= min && x < max);
}
typedef struct {
  double val;
  double err;
} NumErr;
  
// The good is the enemy of the better
/*
template <typename num_t>
class NumErr{
public:
  NumErr(num_t val, num_t err):
    m_val(val),
    m_err(err)  
  {};
  NumErr():
    m_val(0.),
    m_err(0.)
  {};
  ~NumErr(){};
  num_t Err() const {
    return m_err;
  }
  num_t Val() const {
    return m_val;
  }
  bool operator==(const NumErr &other) const {
    return (m_val == other.m_val &&
	    m_err == other.m_err);
  }
  bool operator!=(const NumErr &other) const{
    return !(*this == other);
  }
  char* str_rep(){
    char buff[128];
    snprintf(buff,sizeof(buff)/sizeof(*buff),
	     "%.7g +/- %.7g",
	     this->m_val, this->m_err);
    return buff;
  }
  //Any reasonable compiler (with optimization on) will simplify
  //pow(x,2) to x*x so there's no reason to mess with that kludge
  //here
  const NumErr &operator+(const NumErr &other) const {
    return NumErr(this->m_val+other.m_val,
		  sqrt(pow(this->m_err,2)+pow(other.m_err,2)));
  }
  const NumErr &operator-(const NumErr &other) const {
    return NumErr(this->m_val-other.m_val,
		  sqrt(pow(this->m_err,2)+pow(other.m_err,2)));
  }
  const NumErr &operator*(const NumErr &y) const {
    const NumErr& x=*this;
    return NumErr(x.m_val*y.m_val,
		  sqrt(pow(y.m_val*x.m_err,2)+
		       pow(x.m_val*y.m_err,2)));
  }
  const NumErr &operator/(const NumErr &y) const {
    const NumErr& x=*this;
    return NumErr(x.m_val/y.m_val,
		  sqrt(pow(x.m_err/y.m_val,2)+
		       pow(x.m_val/pow(y.m_val,2)*y.m_err,2)));
  }
private:
  const num_t m_val;
  const num_t m_err;
};
*/
