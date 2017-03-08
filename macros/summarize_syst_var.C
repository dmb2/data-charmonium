void summarize_syst_var(std::string nom_fname, std::string up_fname, std::string down_fname){
  TFile* f_nom = TFile::Open(nom_fname.c_str());
  TFile* f_up = TFile::Open(up_fname.c_str());
  TFile* f_down = TFile::Open(down_fname.c_str());
  TTree* nom = NULL; TTree* up=NULL; TTree* down=NULL;
  f_nom->GetObject("mini",nom);
  f_up->GetObject("mini",up);
  f_down->GetObject("mini",down);
  TH1D* hnom= new TH1D("hnom","Jet z",50,0,1.1);
  TH1D* hup= new TH1D("hup","Jet z",50,0,1.1);
  TH1D* hdown= new TH1D("hdown","Jet z",50,0,1.1);
  nom->Draw("jet_z>>hnom");
  up->Draw("jet_z>>hup");
  down->Draw("jet_z>>hdown");
  std::cout<<"| z | ";
  std::cout<<(hup->GetMean()-hdown->GetMean())/hnom->GetMean()*100<<std::endl;
  delete hnom;
  delete hup;
  delete hdown;
  hnom= new TH1D("hnom","dR",50,0,0.08);
  hup= new TH1D("hup","dR",50,0,0.08);
  hdown= new TH1D("hdown","dR",50,0,0.08);
  nom->Draw("delta_r>>hnom");
  up->Draw("delta_r>>hup");
  down->Draw("delta_r>>hdown");
  std::cout<<"| $\\Delta R$ | ";
  std::cout<<(hup->GetMean()-hdown->GetMean())/hnom->GetMean()*100<<std::endl;
}
