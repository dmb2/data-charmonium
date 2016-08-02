#include "TLegend.h"
#include "TPaletteAxis.h"
#include "TPad.h"
#include "TLatex.h"
#include "THStack.h"
#include "TColor.h"
#include "TList.h"
#include "TTree.h"
#include "TROOT.h"
#include "TCanvas.h"

#include "histo-utils.hh"
#include "plot-utils.hh"
#include "stack-utils.hh"
#include "root-sugar.hh"
//#include "histo-meta-data.hh"
TPad* split_canvas(TPad* canvas, float fraction){
  canvas->SetBottomMargin(fraction); canvas->Modified(); canvas->Update();
  TPad* rpad = new TPad("rpad","",0.,0.,1.0,fraction);
  rpad->SetFillStyle(0);
  rpad->SetMargin(canvas->GetLeftMargin(),canvas->GetRightMargin(),0.3,0.01);
  rpad->Draw(); 
  rpad->SetTicks(0,1);
  // rpad->SetGrid(0,1);
  //rpad->cd();
  rpad->Clear();
  return rpad;
}
//TODO refactor
void draw_ratios(TPad* pad,TList* hist_list){
  pad->cd();
  //Clone each histo in the hist_list to a new ratio
  std::vector<TH1*> ratios;
  ratios.reserve(hist_list->GetSize());
  TIter next(hist_list);
  TH1* hist = NULL;
  while((hist=dynamic_cast<TH1*>(next()))){
    ratios.push_back(dynamic_cast<TH1*>(hist->Clone((std::string(hist->GetName())+"_rat").c_str())));
  }
  TH1* base = dynamic_cast<TH1*>(ratios.at(0)->Clone("base"));
  double draw_max(-999),draw_min(999);
  int max_bin(-1),min_bin(-1);
  for(std::vector<TH1*>::iterator hist = ratios.begin(); hist!=ratios.end(); ++hist){
    (*hist)->SetFillStyle(0);
    (*hist)->Divide(base);

    max_bin=(*hist)->GetMaximumBin();
    min_bin=(*hist)->GetMinimumBin();
    if((*hist)->GetBinContent(max_bin)+
       (*hist)->GetBinError(max_bin) > draw_max){
      draw_max = (*hist)->GetBinContent(max_bin)+
	(*hist)->GetBinError(max_bin);
      MSG_DEBUG("Draw max: "<<draw_max);
    }
    if((*hist)->GetBinContent(min_bin)-
       (*hist)->GetBinError(min_bin) < draw_min){
      draw_min = (*hist)->GetBinContent(min_bin)-
	(*hist)->GetBinError(min_bin);
      MSG_DEBUG("Draw min: "<<draw_min);
    }
  }
  if(draw_max > 2){
    draw_max = 2;
  }
  if(draw_min < 0.0){
    draw_min = 0.0;
  }
  MSG_DEBUG("Final draw max: "<<draw_max);
  MSG_DEBUG("Final draw min: "<<draw_min);
  TAxis* y_axis = ratios.front()->GetYaxis();
  TAxis* x_axis = ratios.front()->GetXaxis();
  y_axis->SetTitle("Ratio");
  y_axis->SetTitleSize(0.15);
  y_axis->SetTitleOffset(0.5);
  y_axis->SetLabelSize(0.15);
  y_axis->SetNdivisions(5);
  x_axis->SetTitleSize(0.15);
  x_axis->SetLabelSize(0.15);
  x_axis->SetTickLength(0.07);
  x_axis->SetTitleOffset(0.9);
  ratios.front()->SetMinimum(draw_min);
  ratios.front()->SetMaximum(draw_max);
  ratios.front()->Draw("");
  for(std::vector<TH1*>::iterator hist = ratios.begin()+1;
      hist!=ratios.end(); ++hist){
    (*hist)->Draw("H same");
    // (*hist)->SetMinimum(-0.5);
    // (*hist)->SetMaximum(3.0);
  }
}
void print_ratio_hist(std::map<std::string,TTree*>& samples, const std::string& plot,
		      TH1* base_hist, const std::string& suffix, 
		      const double target_lumi){
  TCanvas canv(("plt_canv_"+plot).c_str(), "Plot", 600,600);
  
  TLatex decorator;
  TLegend* leg=init_legend();
  if(plot=="jet_z"){
    delete leg;
    leg = init_legend(0.25,0.68,0.4,0.92);
  }
  decorator.SetTextSize(0.04);
  std::vector<std::string> cb;
  cb.push_back("");
  THStack* stack = make_stack(base_hist,samples,cb,0,plot,*leg,target_lumi);
  canv.cd();
  double max = norm_stack(*stack);

  TIter next(stack->GetHists());
  TH1* h = NULL;
  while((h = dynamic_cast<TH1*>(next()))){
    h->SetFillStyle(0);
  }

  stack->Draw("H nostack");
  stack->SetMaximum(1.3*max);
  stack->GetHistogram()->GetYaxis()->SetTitle("Arbitrary Units");
  remove_axis(stack->GetHistogram()->GetXaxis());
  leg->Draw();
  if(plot=="jet_z"){
    add_atlas_badge(canv,0.2,0.6,target_lumi);
  }
  else{
    add_atlas_badge(canv,0.2,0.9,target_lumi);
  }
  // decorator.DrawLatexNDC(0.,0.05,base_hist->GetTitle());
  TPad* rpad = split_canvas(&canv,0.3);
  draw_ratios(rpad,stack->GetHists());
  canv.SaveAs((plot + suffix).c_str());
}
