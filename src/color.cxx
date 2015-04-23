// Ported from http://www.physi.uni-heidelberg.de//Forschung/ANP/Cascade/files/colorTableRoot.cpp
// Found here: https://root.cern.ch/drupal/content/rainbow
// Implements: http://dx.doi.org/10.1016/j.csda.2008.11.033
#include "TStyle.h"
#include "TColor.h"
#include "color.hh"
#include "math.hh"
#include <iostream>
#include <algorithm>

double scale_pos(double factor, double expt, double min, double max){
  return expt != 1.0 ? 
    (min+(pow(factor,expt)*(max-min))) : 
    (min+factor*(max-min));
}
double scale_neg(double factor, double expt, double min, double max){
  return expt != 1.0 ? 
    (max-(pow(factor,expt)*(max-min))) : 
    (max-factor*(max-min));
}
RGB HCLtoRGB(HCL color){
  RGB rgb={0.0, 0.0, 0.0};
  if(!is_in(color.h,float(0.),float(360.0))){
    return rgb;
  }
  // double r=0,g=0,b=0;
  double c=color.l*color.c;
  double m=color.l-c;
  double h0 = color.h/60;
  double x = c*(1- fabs(fmod(h0,2) - 1));
  if(is_in(h0,0.0,1.0)){
    rgb.r=c; rgb.g=x; rgb.b=0;
  }
  else if(is_in(h0,1.0,2.0)){
    rgb.r=x; rgb.g=c; rgb.b=0;
  }
  else if(is_in(h0,2.0,3.0)){
    rgb.r=0; rgb.g=c; rgb.b=x;
  }
  else if(is_in(h0,3.0,4.0)){
    rgb.r=0; rgb.g=x; rgb.b=c;
  }
  else if(is_in(h0,4.0,5.0)){
    rgb.r=x; rgb.g=0; rgb.b=c;
  }
  else if(is_in(h0,5.0,6.0)){
    rgb.r=c; rgb.g=0; rgb.b=x;
  }
  rgb.r+=m; rgb.g+=m; rgb.b+=m;
  return rgb;
}
tcolor_list build_list(const size_t n, hcl_params pars, RGB (*make_color)(double,hcl_params)){
  tcolor_list result(n,0);
  RGB rgb;
  for(size_t i=0; i < n; i++){
    rgb=make_color(double(i)/n,pars);
    result[i]=TColor::GetColor(rgb.r,rgb.g,rgb.b);
  }
  return result;
}
RGB make_qualitative(double i, hcl_params pars){
  HCL color;
  color.h=scale_pos(i,pars.pow[0],pars.hue[0],pars.hue[1]);
  color.c=pars.chroma[0];
  color.l=pars.lumi[0];
  return HCLtoRGB(color);
}
tcolor_list qualitative(color_type color_feeling, const size_t n){
  hcl_params pars={{0.5,0.0},
		   {0.7,0.0},
		   {1.0,0.0},
		   {0.0,0.0}};
  switch(color_feeling){
  case DYNAMIC:
    pars.hue[0]=30; pars.hue[1]=300;
    break;
  case HARMONIC:
    pars.hue[0]=60; pars.hue[1]=240;
    break;
  case COLD:
    pars.hue[0]=150; pars.hue[1]=270;
    break;
  case WARM:
    pars.hue[0]=90; pars.hue[1]=330;
    break;
  default:
    pars.hue[0]=30; pars.hue[1]=300;
    break;
  }
  return build_list(n,pars,make_qualitative);
}
RGB make_sequential(double i, hcl_params pars){
  HCL color;
  color.h=pars.hue[0];
  color.c=scale_pos(i,pars.pow[0],pars.chroma[0],pars.chroma[1]);
  color.l=scale_neg(i,pars.pow[1],pars.lumi[0],pars.lumi[1]);
  return HCLtoRGB(color);
}
tcolor_list sequential(const double hue, const size_t n){
  //TODO add an enum for strategy, or infer it from the parameters
  // passed 

  //one possible strategy
  hcl_params pars={{0,1.0},
		   {0.3,0.9},
		   {1.5,1.5},
		   {hue,hue}};
  return build_list(n,pars,make_sequential);
}
tcolor_list diverging(const double hue1, const double hue2, const size_t n){
  size_t m=n;
  if(n%2!=0){
    m++;
  }
  else{
    std::cout<<" Warning! Even number of diverging colors requested, returning "<<n-1<<" colors instead"<<std::endl;;
  }
  tcolor_list part1 = sequential(hue1,m/2);
  tcolor_list part2 = sequential(hue2,m/2);
  tcolor_list result(m,0);
  std::reverse(part1.begin(),part1.end());
  for(size_t i=0; i < m/2; i++){
    result[i]=part1[i];
  }
  for(size_t i=m/2+1; i < m; i++){
    result[i]=part2[i-m/2];
  }
  return result;
}

void set_gradient(TStyle* style,const hcl_params pars, double* stops, const size_t n_stops){
  const int ncont = 200;
  
  std::vector< RGB > rgb_vals;
  HCL color; 
  for(size_t i = 0; i < n_stops; i++){
    color.h=scale_neg(stops[i],1.0,pars.hue[0],pars.hue[1]);
    color.c=scale_neg(stops[i],pars.pow[0],pars.chroma[0],pars.chroma[1]);
    color.l=scale_neg(stops[i],pars.pow[1],pars.lumi[0],pars.lumi[1]);
    rgb_vals.push_back(HCLtoRGB(color));
  }
  double* red = new double[n_stops];
  double* green = new double[n_stops];
  double* blue = new double[n_stops];
  size_t i=0;
  for(std::vector< RGB >::const_iterator rgb=rgb_vals.begin(); 
      rgb!=rgb_vals.end(); ++rgb, i++){
    red[i]=rgb->r; green[i]=rgb->g; blue[i]=rgb->b;
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
void heat_gradient(TStyle* style){
  double stops[]={0.0,0.25,0.5,0.75,1.0};
  heat_gradient(style,stops,sizeof stops/sizeof *stops);
}

void rainbow_gradient(TStyle* style, double* stops, const size_t n_stops){
  hcl_params pars={{0.9,0.91},
		   {0.85,0.95},
		   {0.4,2.2},
		   {0.0,250.0}};
  set_gradient(style,pars,stops,n_stops);
}
void rainbow_gradient(TStyle* style){
  double stops[]={0.0,0.25,0.5,0.75,1.0};
  rainbow_gradient(style,stops,sizeof stops/sizeof *stops);
}
void single_gradient(TStyle* style, const double hue, double* stops, const size_t n_stops){
  hcl_params pars = {{0.3,1.0},
		     {0.3,0.9},
		     {1.5,1.5},
		     {hue,hue}};
  set_gradient(style,pars,stops,n_stops);
}
void single_gradient(TStyle* style, const double hue){
  double stops[]={0.0,0.25,0.5,0.75,1.0};
  single_gradient(style,hue,stops,sizeof stops/sizeof *stops);
}
