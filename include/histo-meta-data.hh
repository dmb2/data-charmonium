void initialize_hist_book(std::map<std::string,TH1D*>& HistBook){
  HistBook["jet_pt"]=new TH1D("jet_pt","Jet p_{T};p_{T} [GeV];evts/binwidth",50,0,400);
  HistBook["jet_eta"]=new TH1D("jet_eta","Jet #eta;#eta;evts/binwidth",50,-2.6,2.6);
  HistBook["jet_e"]=new TH1D("jet_e","Jet E;E [GeV]; evts/binwidth",50,0,500);
  HistBook["jet_z"]=new TH1D("jet_z","Jet Z;z;evts/binwidth",50,0,1.);

  HistBook["tau1"]=new TH1D("tau1","N Subjettiness #tau_{1};#tau_{1};evts/binwidth",100,0,1.);
  HistBook["tau2"]=new TH1D("tau2","N Subjettiness #tau_{2};#tau_{2};evts/binwidth",100,0,1.);
  HistBook["tau3"]=new TH1D("tau3","N Subjettiness #tau_{3};#tau_{3};evts/binwidth",100,0,1.);
  HistBook["tau32"]=new TH1D("tau32","N Subjettiness #tau_{32};#tau_{32};evts/binwidth",100,0,1.2);
  HistBook["tau21"]=new TH1D("tau21","N Subjettiness #tau_{21};#tau_{21};evts/binwidth",100,0,1.2);
  HistBook["delta_r"]=new TH1D("delta_r","#Delta R(J/#psi,Jet); #Delta R; evts/binwidth",50,0,1.);
  HistBook["jpsi_pt"]=new TH1D("jpsi_pt","J/#psi p_{T};p_{T} [GeV];evts/binwidth",50,0,300);
  HistBook["jpsi_eta"]=new TH1D("jpsi_eta","J/#psi #eta;#eta;evts/binwidth",50,-2.6,2.6);
  HistBook["jpsi_e"]=new TH1D("jpsi_e","J/#psi E;E [GeV]; evts/binwidth",50,0,600);
}
void initialize_cut_names(std::map<std::string,std::string>& cut_names){
  cut_names["num_jets_p"]="N_{j} #geq 1"; 
  cut_names["jpsi_pt_p"]="p_{T}(J/#psi) > 20 GeV";    
  cut_names["jpsi_eta_p"]="|#eta(J/#psi)| < 2.5";
  cut_names["delta_r_p"]="#Delta R(Jet;J/#psi) < 0.4";    
  cut_names["jet_eta_p"]="|#eta(jet)| < 2.5"; 
  cut_names["jet_pt_p"]="p_{T}(jet) > 45 GeV";
  cut_names["mu_trigger_p"]="1 #mu Trigger, 2012";

}
