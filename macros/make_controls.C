void make_controls(std::string fname,std::string suffix){
  TTree* Mu=NULL;
  TTree* JPsi=NULL;
  TTree* Jets=NULL;
  TTree* PRIVX=NULL;
  TFile* _file0 = TFile::Open(fname.c_str());
  _file0->GetObject("Mu",Mu);
  _file0->GetObject("JPsi",JPsi);
  _file0->GetObject("TrackZJPsiJets",Jets);
  _file0->GetObject("PRIVX",PRIVX);
  std::cout<<"Mu: "<<Mu<<" JPsi: "<<JPsi<<" Jets: "<<Jets<<std::endl;
  JPsi->AddFriend("Mu");
  JPsi->AddFriend("TrackZJPsiJets");
  TFile* output = TFile::Open(("ctrl_hists_"+suffix+".root").c_str(),"RECREATE");
  TH1D* jet_pt = new TH1D("jet_pt","Jet p_T;Control Jet p_T; events",50,0,400);
  TH1D* jpsi_pt = new TH1D("jpsi_pt","Jpsi p_T;Control J/#psi p_T; events",50,0,200);
  TH1D* mu_pt = new TH1D("mu_pt","Mu p_T;Control #mu p_T; events",50,0,140);
  
  TH1D* jet_eta = new TH1D("jet_eta","Jet #eta;Control Jet #eta; events",50,-4,4);
  TH1D* jpsi_rap = new TH1D("jpsi_rap","Jpsi rapidity;Control J/#psi rapidity; events",50,-4,4);
  TH1D* mu_eta = new TH1D("mu_eta","Mu #eta;Control #mu #eta; events",50,-4,4);
  
  TH1D* vtx_ntrk = new TH1D("vtx_ntrk","Track Multiplicity in PV;n_{trk};evts/binwidth",50,0,200);
  TH1D* vtx_chi2 = new TH1D("vtx_chi2","#chi^2 of PV fit;PV #chi^2;evts/binwidth",50,0,350);
  TH1D* vtx_n = new TH1D("vtx_n","PV multiplicity;Number of PV;evts/binwidth",41,0,40);
  JPsi->Draw("VTX_pt[0]*1e-3>>jpsi_pt");
  Jets->Draw("JET_pt[0]*1e-3>>jet_pt");
  JPsi->Draw("VTX_rapidity[0]>>jpsi_rap");
  Jets->Draw("JET_eta[0]>>jet_eta");
  JPsi->Draw("Mu.Mu_MU_pt[JPsi.MUONS_index[0]]*1e-3>>mu_pt");
  JPsi->Draw("Mu.Mu_MU_eta[JPsi.MUONS_index[0]]>>mu_eta");
  PRIVX->Draw("PRIVX_chi2>>vtx_chi2");
  PRIVX->Draw("PRIVX_ntrk>>vtx_ntrk");
  PRIVX->Draw("@PRIVX_chi2.size()>>vtx_n");
  
  output->Write();
  output->Close();
} 
