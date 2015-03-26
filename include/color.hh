class TStyle;

void heat_gradient(TStyle* style, double* stops, const size_t n_stops);
void rainbow_gradient(TStyle* style, double* stops, const size_t n_stops);
void single_gradient(TStyle* style, const double hue, double* stops, const size_t n_stops);
void heat_gradient(TStyle* style){
  double stops[]={0.0,0.25,0.5,0.75,1.0};
  heat_gradient(style,stops,sizeof stops/sizeof *stops);
}
void rainbow_gradient(TStyle* style){
  double stops[]={0.0,0.25,0.5,0.75,1.0};
  rainbow_gradient(style,stops,sizeof stops/sizeof *stops);
}
void single_gradient(TStyle* style, const double hue){
  double stops[]={0.0,0.25,0.5,0.75,1.0};
  single_gradient(style,hue,stops,sizeof stops/sizeof *stops);
}
std::vector<Int_t> qualitative(const double hue, const size_t n);
std::vector<Int_t> sequential(const double hue, const size_t n);
std::vector<Int_t> diverging(const double hue1, const double hue2, const size_t n);
