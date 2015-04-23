#include "math.hh"
#include <string>
#include <cassert>
#include <iostream>
int main(void){
  NumErr<double> a(2.0,1.0);
  NumErr<double> b(4.0,0.5);
  NumErr<double> res=a+b;
  std::cout<<"A: "<<a.Val()<<" +/- "<<a.Err()<<std::endl;
  std::cout<<"B: "<<b.Val()<<" +/- "<<b.Err()<<std::endl;
  std::cout<<"A+B: "<<res.Val()<<" +/- "<<res.Err()<<std::endl;
  std::cout<<res.str_rep()<<std::endl;
  assert(std::string((a+b).str_rep())==std::string(NumErr<double>(6.0,1.11803398875).str_rep()));
  assert(a-b==NumErr<double>(-2.0,1.11803398875));
  assert(a/b==NumErr<double>(0.5,0.257694101601));
  assert(a*b==NumErr<double>(8.0,4.12310562562));
  std::cout<<"All Arithmetic tests passed!"<<std::endl;
  return 0;
}
