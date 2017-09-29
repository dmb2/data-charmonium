{
  gROOT->ProcessLine(".L ./macros/AtlasStyle.C++ ");
  gROOT->ProcessLine("AtlasStyle style;");
  gROOT->ProcessLine("style.SetAtlasStyle();");
  TFile* f_singlet = TFile::Open("208025.ctrl.root");
  TFile* f_octet = TFile::Open("208028.ctrl.root");
  TH1* singlet=NULL;
  TH1* octet=NULL;
  TCanvas canv("canv","canv",600,600);
  const char* vars[] = {"jet_pt","jpsi_pt","mu_pt","jet_eta","jpsi_rap","mu_eta","vtx_ntrk","vtx_chi2","vtx_n"};
  for(size_t i=0; i < sizeof(vars)/sizeof(*vars); i++){
    TLegend* leg = new TLegend(0.65,0.5,0.92,0.92);
    leg->SetFillColor(0);
    leg->SetFillStyle(0);
    leg->SetBorderSize(0);
    leg->AddEntry(singlet,"^{3}P_{J}^{(1)} Singlet","l");
    leg->AddEntry(octet,"^{3}S_{1}^{(8)} Octet","l");
    f_singlet->GetObject(vars[i],singlet);
    f_octet->GetObject(vars[i],octet);
    singlet->SetLineColor(kRed+2);
    octet->SetLineColor(kBlue+2);

    singlet->Draw();
    octet->Draw("same");
    leg->Draw();
    canv.SaveAs((std::string(vars[i])+"_mc_ctrl.pdf").c_str());
  }
}
