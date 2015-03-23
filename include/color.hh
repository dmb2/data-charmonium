class TStyle;

void heat_gradient(TStyle* style, double* stops, const size_t n_stops);
void rainbow_gradient(TStyle* style, double* stops, const size_t n_stops);
void single_gradient(TStyle* style, const double hue, double* stops, const size_t n_stops);
