#include "math.hh"

std::string str_rep(const num_err x){
  char buf[50];
  snprintf(buf,sizeof(buf)/sizeof(*buf),"%.7g +/- %.7g",x.val,x.err);
  return std::string(buf);
}
num_err add(const num_err x, const num_err y){
  // return x+y propagating the error correctly
  num_err res;
  res.val = x.val + y.val;
  res.err=sqrt(pow(x.err,2)+pow(y.err,2));
  return res;
}
num_err sub(const num_err x, const num_err y){
  num_err res;
  res.val = x.val - y.val;
  res.err=sqrt(pow(x.err,2)+pow(y.err,2));
  return res;
}
num_err mul(const num_err x, const num_err y){
  num_err res;
  res.val = x.val*y.val;
  res.err=sqrt(pow(y.val*x.err,2)+
	       pow(x.val*y.err,2));
  return res;
}
num_err div(const num_err x, const num_err y){
  num_err res;
  res.val = x.val/y.val;
  res.err=sqrt(pow(x.err/y.val,2)+
	       pow(x.val/pow(y.val,2)*y.err,2));
  return res;
}
