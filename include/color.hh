class TStyle;

typedef std::vector<int> tcolor_list;
typedef struct {
  double chroma[2];
  double lumi[2];
  double pow[2];
  double hue[2];
} hcl_params;
typedef struct {
  float h, c, l;
} HCL;
typedef struct {
  float r, g, b;
} RGB;

void heat_gradient(TStyle* style, double* stops, const size_t n_stops);
void rainbow_gradient(TStyle* style, double* stops, const size_t n_stops);
void single_gradient(TStyle* style, const double hue, double* stops, const size_t n_stops);
// Sets a gradient from yellow to dark red
void heat_gradient(TStyle* style);
// Sets a traditional rainbow gradient
void rainbow_gradient(TStyle* style);
// Sets the gradient to a specific hue \in (0,360), luminance value is
// varied from dark to light
void single_gradient(TStyle* style, const double hue);
enum color_type {DYNAMIC,HARMONIC,COLD,WARM};
std::vector<int> qualitative(color_type color, const size_t n);
std::vector<int> sequential(const double hue, const size_t n);
std::vector<int> diverging(const double hue1, const double hue2, const size_t n);
