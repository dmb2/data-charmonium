void print_control_data(std::string fname){
  gROOT->ProcessLine(".L ./macros/AtlasStyle.C++ ");
  gROOT->ProcessLine("AtlasStyle style;");
  gROOT->ProcessLine("style.SetAtlasStyle();");
  TFile* file = TFile::Open(fname.c_str());
  TH1* hist=NULL;
  TCanvas canv("canv","canv",600,600);
  const char* vars[] = {"jet_pt","jpsi_pt","mu_pt","jet_eta","jpsi_rap","mu_eta","vtx_ntrk","vtx_chi2","vtx_n"};
  for(size_t i=0; i < sizeof(vars)/sizeof(*vars); i++){
    file->GetObject(vars[i],hist);
    hist->Draw();
    canv.SaveAs((std::string(vars[i])+"_data_ctrl.pdf").c_str());
  }
}
