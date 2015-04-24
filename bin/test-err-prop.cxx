#include "math.hh"
#include <string>
#include <cassert>
#include <iostream>
int main(void){
  num_err x={2,1};
  num_err y={4,0.5};
  std::cout<<"X: "<<str_rep(x)<<std::endl;
  std::cout<<"Y: "<<str_rep(y)<<std::endl;
  std::cout<<"X+Y: "<<str_rep(add(x,y))<<std::endl;
  std::cout<<"X-Y: "<<str_rep(sub(x,y))<<std::endl;
  std::cout<<"X*Y: "<<str_rep(mul(x,y))<<std::endl;
  std::cout<<"X/Y: "<<str_rep(div(x,y))<<std::endl;
  return 0;
}
