#include <map>
#include <string>
#include <iostream>
#include <cmath>

#include "TTree.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TColor.h"

#include "histo-utils.hh"

using namespace std;

void setup_hist(TH1* hist){
  hist->Sumw2();
  hist->SetMarkerStyle(1);
  hist->SetLineWidth(1.);
  hist->SetDrawOption("H");
}
TH2D* setup_response_hist(TH1* hist){
  TColor* color = new TColor(1756,0.0,0.0,0.0,"tran_black",0.75);
  const TAxis* axis = hist->GetXaxis();
  TH2D* hist2D = new TH2D((string(hist->GetName())+ "_rsp").c_str(), hist->GetTitle(),
			  axis->GetNbins(), axis->GetXmin(), axis->GetXmax(),
			  axis->GetNbins(), axis->GetXmin(), axis->GetXmax());
  hist2D->SetMarkerStyle(6);
  hist2D->SetMarkerColor(color->GetNumber());

  hist2D->GetXaxis()->SetTitle("Truth");
  hist2D->GetYaxis()->SetTitle("Reconstructed");
  delete color;
  return hist2D;
}
vector<string> add_prefix(string prefix, vector<string> strings){
  vector<string> result;
  result.reserve(strings.size());
  for(vector<string>::const_iterator str=strings.begin(); str!=strings.end(); ++str){
    result.push_back(prefix + *str);
  }
  return result;
}
string str_join(string base, const char* strings[],size_t start, size_t end){
  string result(strings[start]);
  if(start==end){
    return "";
  }
  for(size_t i=(start+1); i < end; i++){
    result+=(base + string(strings[i]));
  }
  return result;
}
void draw_histo(TTree* tree,const char* branch_name, const char* hist_name, 
		const char* cut_expr){
  char branch_expr[200];
  snprintf(branch_expr,200,"%s>>%s", branch_name,hist_name);
  tree->Draw(branch_expr,cut_expr,"goff");
}
void remove_axis(TAxis* axis){
  axis->SetLabelSize(0);
  axis->SetTitle("");
  axis->SetLabelOffset(999);
}
void set_pad_margins(TVirtualPad* pad,int pad_pos,int n_col,int n_row){
  pad->SetRightMargin(0);
  pad->SetTopMargin(0);
  if (pad_pos < n_col*(n_row-1)+1){
    pad->SetBottomMargin(0);
  }
}
std::vector<std::pair<double,double> > make_roc_pairs(TH1* signal, TH1* background){
  std::vector<std::pair<double,double> > result;
  if(signal->GetDimension()!=background->GetDimension()){
    std::cout << "Error: Dimension mismatch! - make_roc_pairs"<<std::endl;
    return result;
  }
  else if (signal->GetDimension() > 1 ){
    std::cout <<"Error: Only 1D hists are supported. - make_roc_pairs"<<std::endl;
  }
  else if(signal->GetNbinsX() != background->GetNbinsX()){
    std::cout<<"Error: NbinsX mismatch. - make_roc_pairs"<<std::endl;
  }
  // reserve space for the histograms
  result.reserve(signal->GetNbinsX());
  // saves having to integrate Nbins times.
  double N_bkg(0.);
  double N_sig(0.);
  for(size_t i=signal->GetNbinsX(); i > 0; --i){
    N_bkg+=background->GetBinContent(i);
    N_sig+=signal->GetBinContent(i);
    result.push_back(std::pair<double,double>(N_sig/(N_bkg+N_sig),
					      1-N_bkg/(N_bkg + N_sig)));
  }
  return result;
}
TH1* make_response_hist(TH1* base_hist, TTree* tree, 
			const char* cut_branches[],size_t cut_index, 
			const string& plot){
  TH1* response = (TH1*)base_hist->Clone((plot + "_RSP_"+
					  str_join("_",cut_branches,
						   0,cut_index+1)).c_str());
  draw_histo(tree,(plot+":truth_"+plot).c_str(),response->GetName(),
  	     str_join("*",cut_branches,0,cut_index+1).c_str());
  return response;
}
TH1* make_normal_hist(TH1* base_hist, TTree* tree, 
		      const char* cut_branches[], size_t cut_index, 
		      const string& plot){
  const std::string name_suffix="_NRM_"+ str_join("_",cut_branches,
						  0,cut_index+1);
  const std::string weight_expr=("weight*"+str_join("*",cut_branches,
						    0,cut_index+1));
  return make_normal_hist(base_hist,tree,plot,weight_expr.c_str(),name_suffix);
}
TH1* make_ratio_hist(TH1* base_hist, TTree* tree,
		     const char* cut_branches[],size_t cut_index, 
		     const string& plot){
  TH1* ratio =(TH1*)base_hist->Clone((plot +"_R_"+
				      str_join("_",cut_branches,
					       0,cut_index+1)).c_str());
  string hNDenom = plot+str_join("_",cut_branches,0,cut_index)+"_tmp";
  TH1* hDenom = (TH1*)ratio->Clone(hNDenom.c_str());
  draw_histo(tree, plot.c_str(), hNDenom.c_str(),
	     str_join("*", cut_branches, 0,cut_index).c_str());
  draw_histo(tree, plot.c_str(), ratio->GetName(),
	     str_join("*", cut_branches, 0,cut_index+1).c_str());
  ratio->Divide(hDenom);
  ratio->SetMaximum(1.2);
  ratio->SetMinimum(0.);
  return ratio;
}
TH1* make_response_hist(TH1* base_hist,TTree* tree,const std::string& plot,
			const char* weight_expr, const std::string& name_suffix){
  TH1* hist = (TH1*)base_hist->Clone((plot + "_rsp_NOM").c_str());
  draw_histo(tree,(plot + ":truth_"+plot).c_str(), hist->GetName(), "");
  return hist;
}
TH1* make_normal_hist(TH1* base_hist,TTree* tree,const std::string& plot,
		      const char* weight_expr,
		      const std::string& name_suffix){
  TH1* hist = (TH1*)base_hist->Clone((plot + name_suffix).c_str());
  draw_histo(tree,plot.c_str(), hist->GetName(), weight_expr);
  return hist;
}

void print_hist(TTree* tree, const std::string& plot, 
		TH1* base_hist, const std::string suffix, 
		TH1* (*make_hist)(TH1*,TTree*,const std::string&, const char*,
				  const std::string&)){
  TCanvas canv(("canv_"+plot).c_str(),"Plot",600,600);
  TLatex decorator;
  decorator.SetTextSize(0.04);
  TH1* hist = make_hist(base_hist,tree,plot,"_nom","weight");
  hist->Draw("H");
  decorator.DrawLatexNDC(0.,0.05,hist->GetTitle());
  canv.SaveAs((plot+suffix).c_str());
}
void print_cut_hist(TTree* tree,const char* cut_branches[],size_t nCuts, 
		const std::string& plot, TH1* base_hist, 
		map<string,string>& CutNames, std::string file_suffix,
		TH1* (*make_hist)(TH1* ,TTree* , const char**, 
				  size_t, const std::string&)){
  TCanvas canv(("canv_"+plot).c_str(),"Cut Plot",1800,800);
  // TPaveText label(0.08,0.6,0.18,0.71,"NB");
  TLatex decorator;
  // decorator.SetTextAlign(12);
  decorator.SetTextSize(0.1);
  canv.Divide(3,2);
  canv.SetRightMargin(0);
  canv.SetTopMargin(0);
  for(size_t i = 0; i < nCuts; i++){
    set_pad_margins(canv.cd(i+1),i+1);
    TH1* hist = make_hist(base_hist,tree, cut_branches, i,plot);
    hist->Draw("BOX");
    if( i < 3){ //top row
      remove_axis(hist->GetXaxis());
    }
    if(i != 0 && i != 3 && file_suffix.find("normal")==std::string::npos){
      remove_axis(hist->GetYaxis());
    }
    decorator.DrawLatexNDC(0.5,0.75,CutNames[cut_branches[i]].c_str());
  }
  canv.cd(0);
  decorator.SetTextSize(0.04);
  decorator.DrawLatex(0.0,0.05,base_hist->GetTitle());
  canv.SaveAs((plot+file_suffix).c_str());
}
