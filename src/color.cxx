// Ported from http://www.physi.uni-heidelberg.de//Forschung/ANP/Cascade/files/colorTableRoot.cpp
// Found here: https://root.cern.ch/drupal/content/rainbow
// Implements: http://dx.doi.org/10.1016/j.csda.2008.11.033
#include "TStyle.h"
#include "TColor.h"

typedef struct params{
  double chroma[2];
  double lumi[2];
  double pow[2];
  double hue[2];
} hcl_params;
// Probably should move this somewhere more useful someday
template<typename T>
bool is_in(T x,T min,T max){
  return (x >= min && x < max);
}

double scaleC(double factor, double expt, double min, double max){
  return expt != 1.0 ? 
    (max-(pow(factor,expt)*(max-min))) : 
    (max-factor*(max-min));
}
std::vector<double> RGBfromHCL(double h, double c0, double l0){
  std::vector<double> rgb(3,0.);
  if(!is_in(h,0.0,360.0)){
    return rgb;
  }
  double r=0,g=0,b=0;
  double c=l0*c0;
  double m=l0-c;
  double h0 = h/60;
  double x = c*(1- fabs(fmod(h0,2) - 1));
  if(is_in(h0,0.0,1.0)){
    r=c; g=x; b=0;
  }
  else if(is_in(h0,1.0,2.0)){
    r=x; g=c; b=0;
  }
  else if(is_in(h0,2.0,3.0)){
    r=0; g=c; b=x;
  }
  else if(is_in(h0,3.0,4.0)){
    r=0; g=x; b=c;
  }
  else if(is_in(h0,4.0,5.0)){
    r=x; g=0; b=c;
  }
  else if(is_in(h0,5.0,6.0)){
    r=c; g=0; b=x;
  }
  rgb[0]=r+m;
  rgb[1]=g+m;
  rgb[2]=b+m;
  return rgb;
}
void set_gradient(TStyle* style,const hcl_params pars, double* stops, const size_t n_stops){
  const int ncont = 200;
  
  std::vector<std::vector<double> > rgb_vals;
  for(size_t i = 0; i < n_stops; i++){
    rgb_vals.push_back(RGBfromHCL(scaleC(stops[i],1.0,pars.hue[0],pars.hue[1]),
				  scaleC(stops[i],pars.pow[0],pars.chroma[0],pars.chroma[1]),
				  scaleC(stops[i],pars.pow[1],pars.lumi[0],pars.lumi[1])));
  }
  double* red = new double[n_stops];
  double* green = new double[n_stops];
  double* blue = new double[n_stops];
  size_t i=0;
  for(std::vector<std::vector<double> >::const_iterator rgb=rgb_vals.begin(); 
      rgb!=rgb_vals.end(); ++rgb, i++){
    red[i]=rgb->at(0);
    green[i]=rgb->at(1);
    blue[i]=rgb->at(2);
  }
  TColor::CreateGradientColorTable(n_stops,stops,red,green,blue,ncont);
  style->SetNumberContours(ncont);
  delete red; delete green; delete blue;
}
void heat_gradient(TStyle* style, double* stops, const size_t n_stops){
  hcl_params pars = {{1.0,0.6},  // chroma
		     {0.5,0.9},  // lumi
		     {0.4,2.2},  // pow
		     {0.0,60.}}; // hue
  set_gradient(style,pars,stops,n_stops);
}
void rainbow_gradient(TStyle* style, double* stops, const size_t n_stops){
  hcl_params pars={{0.9,0.91},
		   {0.85,0.95},
		   {0.4,2.2},
		   {0.0,250.0}};
  set_gradient(style,pars,stops,n_stops);
}
void single_gradient(TStyle* style, const double hue, double* stops, const size_t n_stops){
  hcl_params pars = {{0.3,1.0},
		     {0.3,0.9},
		     {1.5,1.5},
		     {hue,hue}};
  set_gradient(style,pars,stops,n_stops);
}
