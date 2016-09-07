#include "root-sugar.hh"
#include "TCanvas.h"
#include "TROOT.h"
#include "TColor.h"
#include "TKey.h"
#include "TH1.h"
#include "TFile.h"
#include "TLegend.h"
#include "TLegendEntry.h"
#include "TLatex.h"
#include "THStack.h"
#include "color.hh"
#include "histo-utils.hh"
#include "TStyle.h"

#include <string>
#include <math.hh>

void usage(const char* prog_name){
  MSG("Usage: "<<prog_name<< " -o outfile [slice files]");
}
void update_draw_max(TH1* hist,double& draw_max){
  double max = hist->GetBinContent(hist->GetMaximumBin());
  max+=hist->GetBinError(hist->GetMaximumBin());
  if(max > draw_max){
    draw_max = max;
  }
}
void fix_and_draw_leg(TLegend* leg,TH1* err_hist){
  TIter iter(leg->GetListOfPrimitives());
  TLegendEntry* entry;
  while((entry=dynamic_cast<TLegendEntry*>(iter()))){
    if(std::string(entry->GetLabel())=="Systematic Error"){
      entry->SetObject(err_hist);
    }
  }
  leg->SetTextSize(0.08);
  leg->SetX1NDC(0);
  leg->SetY1NDC(0);
  leg->SetX2NDC(1);
  leg->SetY2NDC(0.78);
  leg->Draw();
}
int main(const int argc, char* const argv[]){
  setup_global_style();
  char* out_fname=NULL;
  size_t n_cols = 3;
  int c;
  double lumi=0;
  while((c = getopt(argc,argv,"o:n:l:")) != -1){
    switch(c){
    case 'o':
      out_fname = optarg;
      break;
    case 'n':
      n_cols=atoi(optarg);
      break;
    case 'l':
      lumi=atof(optarg);
      break;
    default:
      abort();
    }
  }
  if(out_fname==NULL || optind==argc){
    usage(argv[0]);
    exit(1);
  }
  MSG_DEBUG("Writing to: "<< out_fname);
  std::map<std::string,std::vector<TObject*> > draw_map; 
  double draw_max;
  
  char pretty_name[256];
  TLegend* leg=nullptr;
  for(int i=optind; i < argc; i++){
    const std::string fname(argv[i]);
    MSG_DEBUG("Processing "<<fname);
    std::string slice_dir;
    std::vector<std::string> parts = split_string(fname,'/');
    for(std::vector<std::string>::const_iterator str_itr = parts.begin();
	str_itr!=parts.end(); ++str_itr){
      if(str_itr->find("jpsi_pt")!=std::string::npos){
	slice_dir = *str_itr;
	break;
      }
    }
    parts = split_string(slice_dir,'_');
    int lo_edge = atoi(parts.at(parts.size()-2).c_str());
    int hi_edge = atoi(parts.at(parts.size()-1).c_str());
    snprintf(pretty_name,256,"%.3g GeV < p_{T}(J/#psi) < %.3g GeV",lo_edge/100.0,hi_edge/100.0);
    TFile* file = TFile::Open(fname.c_str());
    TKey* key = dynamic_cast<TKey*>(file->GetListOfKeys()->First());
    TCanvas *c1 = dynamic_cast<TCanvas*>(key->ReadObj());
    if(c1==nullptr){
      MSG_ERR("Could not retrieve canvas from first key of input file: "<<fname);
      exit(1);
    }
    TIter iter(c1->GetListOfPrimitives());
    std::vector<TObject*> to_draw;
    TObject* obj=nullptr;
    while((obj=iter())){
      const std::string class_name(obj->ClassName());
      if(class_name=="TH1D"){
	TH1D* hist = dynamic_cast<TH1D*>(obj);
	update_draw_max(hist,draw_max);
      }
      if(class_name=="THStack"){
	THStack* stack = dynamic_cast<THStack*>(obj);
	TH1* hist = dynamic_cast<TH1*>(stack->GetStack()->Last());
	update_draw_max(hist,draw_max);
      }
      if(std::string(obj->ClassName())=="TH1D" ||
	 std::string(obj->ClassName())=="THStack"){
	to_draw.push_back(obj);
      }
      else{
	if(leg==nullptr || std::string(obj->ClassName())=="TLegend"){
	  leg = dynamic_cast<TLegend*>(obj);
	}
      }
    }
    draw_map[pretty_name]=to_draw; 
  }//loop over file names
  size_t n_slices = argc-optind;
  const size_t n_rows = ceil(n_slices/float(n_cols));
  TCanvas canv("canvas","canvas",600*n_cols,600*n_rows);
  canv.Divide(n_cols,n_rows);
  size_t idx=0;
  TH1D* err_hist;
  TVirtualPad* pad;
  TLatex decorator;
  decorator.SetTextSize(0.06);
  for(std::map<std::string,std::vector<TObject*> >::iterator itr=draw_map.begin();
      itr!=draw_map.end(); ++itr){
    const std::string& name = itr->first;
    std::vector<TObject*>& draw_obj = itr->second;
    MSG_DEBUG(name);
    idx++;
    pad = canv.cd(idx);
    pad->SetTickx(0);
    pad->SetTicky(0);
    pad->SetRightMargin(0.06);
    pad->SetTopMargin(0.05);
    
    if(idx%n_cols!=1){
      pad->SetLeftMargin(0.01);
    }
    pad->cd();
    
    double N_sig;
    double N_MC;
    TH1D* hist = nullptr;
    THStack* hstack = nullptr;
    if(std::string(draw_obj.front()->ClassName())=="THStack"){
      hstack = dynamic_cast<THStack*>(draw_obj.front());
      if(idx%n_cols!=1){
	hstack->GetYaxis()->SetNdivisions(0);
	hstack->GetYaxis()->SetAxisColor(kWhite);
	remove_axis(hstack->GetYaxis());
      }
      else{
	hstack->GetYaxis()->SetNdivisions(10);
      }
      TH1* hs = dynamic_cast<TH1*>(hstack->GetStack()->Last());
      N_MC=hs->Integral();
      MSG_DEBUG("Total stack integral: "<<N_MC);
      hstack->SetMaximum(draw_max);
      hstack->Draw("HIST");
    }
    if(std::string(draw_obj.front()->ClassName())=="TH1D"){
      hist = dynamic_cast<TH1D*>(draw_obj.front());
      std::string hname(hist->GetName());
      if(hname.find("tau1")!=std::string::npos  ||
	 (hname.find("tau2")!=std::string::npos && hname.find("tau21")==std::string::npos) ||
	 (hname.find("tau3")!=std::string::npos && hname.find("tau32")==std::string::npos)){
	pad->SetLogy(true);
      }
      if(idx%n_cols!=1){
	hist->GetYaxis()->SetNdivisions(0);
	hist->GetYaxis()->SetAxisColor(kWhite);
	remove_axis(hist->GetYaxis());
      }
      if(N_sig == 0 && std::string(hist->GetName()).find("sig")!=std::string::npos){
	N_sig = hist->Integral();
      }
      hist->SetMaximum(draw_max);
      hist->SetFillStyle(0);
      hist->SetLineWidth(0);
      hist->SetMarkerColor(kBlack);
      hist->Draw("E");
    }
    std::string draw_opt("same ");
    bool draw_copy=true;
    for(std::vector<TObject*>::iterator d_itr = draw_obj.begin()+1;
    	d_itr!=draw_obj.end(); ++d_itr){
      const std::string class_name((*d_itr)->ClassName());
      if(class_name=="TH1D"){
	hist = dynamic_cast<TH1D*>(*d_itr);
	hist->SetMaximum(draw_max);
	if(std::string(hist->GetName()).find("sig")!=std::string::npos){
	  N_sig = hist->Integral();
	}
	if(std::string(hist->GetName())=="tot_syst_err"){
	  err_hist=hist;
	  hist->SetMarkerStyle(kDot);
	  hist->SetFillColor(TColor::GetColorTransparent(kBlack,0.4));
	  hist->SetLineColor(TColor::GetColorTransparent(kBlack,0.4));
	  hist->SetMarkerColor(TColor::GetColorTransparent(kBlack,0.4));
	  draw_opt=("e2 same");
	}
	if(std::string(hist->GetName()).find("copy")!=std::string::npos){
	  if(!draw_copy ){
	    draw_copy=true;
	    continue;
	  }
	  if(leg){
	    TIter iter(leg->GetListOfPrimitives());
	    TLegendEntry* entry;
	    while((entry=dynamic_cast<TLegendEntry*>(iter()))){
	      MSG_DEBUG(entry->GetFillColor()<<" "<<entry->GetLabel()<<" "<<entry->GetObject());
	      if(std::string(entry->GetLabel())=="Background" &&
		 std::string(hist->GetName()).find("bkg")!=std::string::npos){
		hist->SetFillColor(TColor::GetColorTransparent(entry->GetFillColor(),0.6));
		hist->SetLineColor(TColor::GetColorTransparent(entry->GetFillColor(),0.6));
		hist->SetMarkerColor(TColor::GetColorTransparent(entry->GetFillColor(),0.6));
	      }
	      if(std::string(entry->GetLabel())=="Signal" &&
		 std::string(hist->GetName()).find("sig")!=std::string::npos){
		hist->SetFillColor(TColor::GetColorTransparent(entry->GetFillColor(),0.6));
		hist->SetLineColor(TColor::GetColorTransparent(entry->GetFillColor(),0.6));
		hist->SetMarkerColor(TColor::GetColorTransparent(entry->GetFillColor(),0.6));
	      }
	    }
	  }
	  hist->SetMarkerStyle(kDot);
	  draw_opt=("e2 same");
	  TH1D* hl = dynamic_cast<TH1D*>(hist->Clone("H"));
	  hl->SetFillStyle(0);
	  hl->Draw("HIST same");
	}
	MSG_DEBUG("Drawing "<<hist<<" name: "<<hist->GetName()<<" "<<draw_opt);
	decorator.DrawLatexNDC(0.2,0.95,name.c_str());
	hist->Draw(draw_opt.c_str());
	draw_copy=false;
      }
    }
    if(hstack){
      for(int i = 0; i < hstack->GetStack()->GetEntries(); i++){
	hist = dynamic_cast<TH1D*>(hstack->GetStack()->At(i));
	hist->Scale(N_sig/N_MC);
      }
    }
  }
  pad=canv.cd(n_slices+1);
  if(leg){
    fix_and_draw_leg(leg,err_hist);
  }
  add_atlas_badge(canv,float(n_cols-1)/n_cols,1-float(n_rows-1)/n_rows+0.01,lumi);
  canv.SaveAs(out_fname);
  return 0;
}
