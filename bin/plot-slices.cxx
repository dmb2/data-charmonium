#include "root-sugar.hh"
#include "TCanvas.h"
#include "TROOT.h"
#include "TColor.h"
#include "TKey.h"
#include "TH1.h"
#include "TFile.h"
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
int main(const int argc, char* const argv[]){
  setup_global_style();
  char* out_fname=NULL;
  int c;
  while((c = getopt(argc,argv,"o:")) != -1){
    switch(c){
    case 'o':
      out_fname = optarg;
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
    // int hi_edge = atoi(parts.at(parts.size()-2).c_str());
    // int lo_edge = atoi(parts.at(parts.size()-1).c_str());
    TFile* file = TFile::Open(fname.c_str());
    TKey* key = dynamic_cast<TKey*>(file->GetListOfKeys()->First());
    TCanvas *c1 = dynamic_cast<TCanvas*>(key->ReadObj());
    if(c1==nullptr){
      MSG_ERR("Could not retrieve canvas from first key of input file: "<<fname);
      exit(1);
    }
    MSG_DEBUG("Retrieved canvas: "<<c1->GetName());
    TIter iter(c1->GetListOfPrimitives());
    std::vector<TObject*> to_draw;
    TObject* obj=nullptr;
    while((obj=iter())){
      const std::string class_name(obj->ClassName());
      double max;
      if(class_name=="TH1D"){
	TH1D* hist = dynamic_cast<TH1D*>(obj);
	max = hist->GetBinContent(hist->GetMaximumBin());
	max+=hist->GetBinError(hist->GetMaximumBin());
	if(max > draw_max){
	  draw_max = max;
	}
      }
      if(class_name=="THStack"){
	THStack* stack = dynamic_cast<THStack*>(obj);
	TH1* hist = dynamic_cast<TH1*>(stack->GetStack()->Last());
	max = hist->GetBinContent(hist->GetMaximumBin());
	max+=hist->GetBinError(hist->GetMaximumBin());
	if(max > draw_max){
	  draw_max = max;
	}
      }
      if(std::string(obj->ClassName())=="TH1D" ||
	 std::string(obj->ClassName())=="THStack"){
	to_draw.push_back(obj);
      }
    }
    draw_map[fname]=to_draw; 
  }
  size_t n_slices = argc-optind;
  // this gives us a canvas for 600x600 plots 2x(N/2)
  TCanvas canv("canvas","canvas",1200,600*ceil(n_slices/2.0));
  MSG_DEBUG(n_slices<< " " << ceil(n_slices/2.0));
  canv.Divide(2,ceil(n_slices/2.0));
  canv.SaveAs(out_fname);
  /*
    for(std::vector<TObject*>::iterator it=to_draw.begin();
	it!=to_draw.end(); ++it){
    }
    ((TH1*)to_draw.front())->SetMaximum(draw_max);
    to_draw.front()->Draw();
    for(std::vector<TObject*>::iterator it=to_draw.begin()+1;
	it!=to_draw.end(); ++it){
      (*it)->Draw("same");
    }
    
   */
  return 0;
}
