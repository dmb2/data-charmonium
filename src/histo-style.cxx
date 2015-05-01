#include "histo-style.hh"
#include "TH1.h"
#include "TLegend.h"


void style_hist(TH1* hist,aesthetic style){
  hist->SetLineColor(style.color);
  hist->SetFillColor(style.color);
  hist->SetMarkerColor(style.color);

  hist->SetFillStyle(style.fill_style);
  hist->SetMarkerStyle(style.marker);
  hist->SetMarkerSize(style.marker_size);
  hist->SetLineStyle(style.line_style);
  hist->SetLineWidth(2);
}
void add_to_legend(TLegend* leg, const TH1* hist, const aesthetic style){
  leg->AddEntry(hist,style.leg_label,style.leg_style);
}
aesthetic make_style(const char* leg_label, const char* leg_style,
		     const Color_t color, const Style_t fill_style,
		     const Style_t marker, const Size_t marker_size,
		     const Style_t line_style){
  aesthetic style={.leg_label=leg_label,
		   .leg_style=leg_style,
		   .color=color,
		   .fill_style=fill_style,
		   .marker=marker,
		   .marker_size=marker_size,
		   .line_style=line_style};
  return style;
}
aesthetic hist_aes(const char* leg_label, const Color_t color, 
		   const Style_t fill_style, const Style_t line_style){
  return make_style(leg_label, "lf", color, fill_style,
		    kDot,0,line_style);
}
aesthetic data_aes(const char* leg_label, const Color_t color, 
		   const Style_t marker, const Style_t line_style){
  return make_style(leg_label,"lp",color,0,marker,
		    1,line_style);
}
