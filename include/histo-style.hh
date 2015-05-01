#include "Rtypes.h"
class TH1;
class TLegend;

typedef struct {
  char* leg_label;
  char* leg_style;
  Color_t color;
  Style_t fill_style;
  Style_t marker;
  Size_t marker_size;
  Style_t line_style;
} aesthetic;

void style_hist(TH1* hist,aesthetic style);
void style_legend(TLegend* leg, const TH1* hist, const aesthetic style);
aesthetic make_style(const char* leg_label, const char* leg_style,
		     const Color_t color, const Style_t fill_style,
		     const Style_t marker, const Size_t marker_size,
		     const Style_t line_style);
aesthetic hist_aes(const char* leg_label, const Color_t color, 
		   const Style_t fill_style, const Style_t line_style);
aesthetic data_aes(const char* leg_label, const Color_t color, 
		   const Style_t marker, const Style_t line_style);
