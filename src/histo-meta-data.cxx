#include "histo-meta-data.hh"
#include "TH1D.h"
#include "TH2D.h"
#include "TColor.h"
void init_hist2D_book(std::map<std::string,TH2D*>& Hist2DBook){
  Hist2DBook["jet_z:jet_e"] = new TH2D("jet_z:jet_E","Jet E vs Jet z;E [GeV]; z",
				       25,0,500,
				       10,0,1.);
  Hist2DBook["jet_e:jet_z"] = new TH2D("jet_E:jet_z","Jet z vs Jet E;z;E [GeV]",
				       25,0,1,
				       10,0,500);
  Hist2DBook["jpsi_pt:jet_z"] = new TH2D("jpsi_pt:jet_z","Jet z vs J/#psi p_T;p_T [GeV];E [GeV]",
					 25,0,1,
					 10,0,250);
  Hist2DBook["jet_z:jet_e"]->GetYaxis()->SetNdivisions(508);
  Hist2DBook["jet_z:jet_e"]->GetXaxis()->SetNdivisions(508);
}

void init_hist_book(std::map<std::string,TH1D*>& HistBook){
  HistBook["jet_pt"]=new TH1D("jet_pt","Jet p_{T};p_{T} [GeV];evts/binwidth",50,0,400);
  HistBook["jet_eta"]=new TH1D("jet_eta","Jet #eta;#eta;evts/binwidth",50,-2.6,2.6);
  HistBook["jet_e"]=new TH1D("jet_e","Jet E;E [GeV]; evts/binwidth",50,0,500);
  HistBook["jet_m"]=new TH1D("jet_m","Jet Mass; M [GeV]; evts/binwidth",50,0,45);
  HistBook["jet_z"]=new TH1D("jet_z","Jet Z;z;evts/binwidth",50,0,1.);
  HistBook["jet_emfrac"]=new TH1D("jet_emfrac","Jet EM Fraction;EM Fraction;evts/binwidth",50,0,1.5);

  HistBook["tau1"]=new TH1D("tau1","N Subjettiness #tau_{1};#tau_{1};evts/binwidth",50,0,1.);
  HistBook["tau2"]=new TH1D("tau2","N Subjettiness #tau_{2};#tau_{2};evts/binwidth",50,0,1.);
  HistBook["tau3"]=new TH1D("tau3","N Subjettiness #tau_{3};#tau_{3};evts/binwidth",50,0,1.);
  HistBook["tau32"]=new TH1D("tau32","N Subjettiness #tau_{32};#tau_{32};evts/binwidth",50,0,1.2);
  HistBook["tau21"]=new TH1D("tau21","N Subjettiness #tau_{21};#tau_{21};evts/binwidth",50,0,1.2);
  HistBook["delta_r"]=new TH1D("delta_r","#Delta R(J/#psi,Jet); #Delta R; evts/binwidth",50,0,.2);

  HistBook["psi_m"]=new TH1D("psi_m","J/#psi + 2 Tracks Mass; M(#mu #mu Trk Trk) [GeV]; evts/binwidth",50,2.5,6);

  HistBook["jpsi_pt"]=new TH1D("jpsi_pt","J/#psi p_{T};p_{T} [GeV];evts/binwidth",50,0,300);
  HistBook["jpsi_eta"]=new TH1D("jpsi_eta","J/#psi #eta;#eta;evts/binwidth",50,-2.6,2.6);
  HistBook["jpsi_e"]=new TH1D("jpsi_e","J/#psi E;E [GeV]; evts/binwidth",50,0,600);
  HistBook["jpsi_m"]=new TH1D("jpsi_m","J/#psi Mass; M(#mu#mu) [GeV]; evts/binwidth",50,2.,4.);
  HistBook["jpsi_lxy"]=new TH1D("jpsi_lxy","J/#psi L_{xy}; L_{xy} [mm]; evts/binwidth",50,-4,10);
  // HistBook["jpsi_s"]=new TH1D("jpsi_s","Impact Parameter Significance;S;evts/binwidth",50,0,55000);
  HistBook["jpsi_tau"]= new TH1D("jpsi_tau","J/#psi c#tau; c#tau; evts/binwidth",50,-1,5);
  // HistBook["mup_d0^2 + mun_d0^2"]=new TH1D("mup_d0^2 + mun_d0^2","Impact parameter quadrature",50,0,10);
  //HistBook["1/mup_d0^2 + 1/mun_d0^2"]=new TH1D("1/mup_d0^2 + 1/mun_d0^2","Impact parameter quadrature",50,0,10);
}
void init_cut_names(std::map<std::string,std::string>& cut_names){
  cut_names["mu_trigger_p"]="Trigger";
  cut_names["num_jets_p"]="N_{j} #geq 1"; 
  cut_names["jpsi_pt_p"]="p_{T}(J/#psi) > 50 GeV";    
  cut_names["jpsi_eta_p"]="|#eta(J/#psi)| < 2.5";
  cut_names["jpsi_rap_p"]="|y(J/#psi)| < 2.0";
  cut_names["mumu_eta_p"]="|#eta(#mu)| < 2.3";
  cut_names["jet_eta_p"]="|#eta(jet)| < 2.5"; 
  cut_names["delta_r_p"]="#Delta R(Jet;J/#psi) < 0.4";    
  cut_names["jet_pt_p"]="p_{T}(jet) > 45 GeV";
}

void init_hist_styles(std::map<std::string,aesthetic>& styles){
  styles["1S0_8"]=hist_aes("^{1}S^{(8)}_{0}",TColor::GetColor(222,235,247),1001,kSolid);
  styles["3S1_8"]=hist_aes("^{3}S^{(8)}_{1}",TColor::GetColor(158,202,225),1001,kSolid);
  styles["3PJ_8"]=hist_aes("^{3}P^{(8)}_{J}",TColor::GetColor(49,130,189),1001,kSolid);
  // Singlet styles	                    
  styles["3S1_1"]=hist_aes("^{3}S^{(1)}_{1}",TColor::GetColor(252,146,114),1001,kSolid);
  styles["3PJ_1"]=hist_aes("^{3}P^{(1)}_{J}",TColor::GetColor(222,45,38),1001,kSolid);  

  // Octet styles
  styles["208024.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_1S0_8"]=hist_aes("^{1}S^{(8)}_{0}",TColor::GetColor(222,235,247),1001,kSolid);
  styles["208028.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3S1_8"]=hist_aes("^{3}S^{(8)}_{1}",TColor::GetColor(158,202,225),1001,kSolid);
  styles["208026.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3PJ_8"]=hist_aes("^{3}P^{(8)}_{J}",TColor::GetColor(49,130,189),1001,kSolid); 
  // Singlet styles						                        
  styles["208027.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3S1_1"]=hist_aes("^{3}S^{(1)}_{1}",TColor::GetColor(252,146,114),1001,kSolid);
  styles["208025.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3PJ_1"]=hist_aes("^{3}P^{(1)}_{J}",TColor::GetColor(222,45,38),1001,kSolid);  

  // non-prompt BKG Styles
  styles["108601.PythiaBc_Bc_JPsi_mu2p5mu2p5_Pi"]=hist_aes("B_{c}#rightarrow J/#psi #pi",TColor::GetColor(186,228,179),1001,kSolid);
  styles["108606.PythiaBc_Bc_JPsi_mu2p5mu2p5_3Pi"]=hist_aes("B_{c}#rightarrow J/#psi 3#pi",TColor::GetColor(116,196,118),1001,kSolid);
  styles["208401.Pythia8B_AU2_CTEQ6L1_Bs_Jpsimu4mu4_phi"]=hist_aes("B_{s}#rightarrow J/#psi #phi",TColor::GetColor(49,163,84),1001,kSolid);
  styles["208413.Pythia8B_AU2_CTEQ6L1_chib_Jpsimu3p5mu3p5Jpsimu3p5mu3p5"]=hist_aes("#chi_{b}#rightarrow 2 J/#psi",TColor::GetColor(0,109,44),1001,kSolid);
  styles["208400.Pythia8B_AU2_CTEQ6L1_Bs_Jpsimu3p5mu3p5_phi"]=hist_aes("B_{s}#rightarrow J/#psi #phi",TColor::GetColor(24,138,59),1001,kSolid);
  styles["208432.Pythia8B_AU2_CTEQ6L1_Bplus_Jpsi_mu3p5mu3p5_piplus"]=hist_aes("B^{+} #rightarrow J/#psi #pi^{+}",TColor::GetColor(86,128,79),1001,kSolid);
  // feed down
  styles["208022.Pythia8B_AU2_CTEQ6L1_pp_Psi2S_JpsiPiPi"]=hist_aes("#psi(2S) #rightarrow J/#psi 2#pi",TColor::GetColor(158,154,200),1001,kSolid);
  styles["208023.Pythia8B_AU2_CTEQ6L1_pp_X3872_JpsiPiPi"]=hist_aes("X(3872) #rightarrow J/#psi 2#pi",TColor::GetColor(106,81,163),1001,kSolid);

  styles["non_prompt"]=hist_aes("Non Prompt Background",TColor::GetColor(189,0,38),1001,kSolid);
  styles["feed_down"]=hist_aes("Feed Down Sources",TColor::GetColor(0,104,55),1001,kSolid);

  styles["psi_bkg"]=hist_aes("#psi(2S) Background",TColor::GetColor(0,104,55),1001,kSolid);
  styles["comb_bkg"]=hist_aes("Comb. Background",TColor::GetColor(240,59,32),1001,kSolid);
  
  //Oranges
  styles["208004.trkComb.LCTopoJets"]=hist_aes("#mu Comb, Jet LCTopo",TColor::GetColor(254,204,92),1001,kSolid);
  styles["208004.trkInnerExtr.LCTopoJets"]=hist_aes("#mu ID Extr., Jet LCTopo",TColor::GetColor(253,141,60),1001,kSolid);
  styles["208004.trkMS.LCTopoJets"]=hist_aes("#mu MS, Jet LCTopo",TColor::GetColor(240,59,32),1001,kSolid);
  styles["208004.trkMuonExtr.LCTopoJets"]=hist_aes("#mu MS Extr., Jet LCTopo",TColor::GetColor(189,0,38),1001,kSolid);
  //Greens 
  styles["208004.trkComb.TopoEMJets"]=hist_aes("#mu Comb, Jet TopoEM",TColor::GetColor(194,230,153),1001,kSolid);
  styles["208004.trkInnerExtr.TopoEMJets"]=hist_aes("#mu ID Extr., Jet TopoEM",TColor::GetColor(120,198,121),1001,kSolid);
  styles["208004.trkMS.TopoEMJets"]=hist_aes("#mu MS, Jet Topoem",TColor::GetColor(49,163,84),1001,kSolid);
  styles["208004.trkMuonExtr.TopoEMJets"]=hist_aes("#mu MS Extr., Jet TopoEM",TColor::GetColor(0,104,55),1001,kSolid);
  // Pinks
  styles["208004.trkComb.TrackZJets"]=hist_aes("#mu Comb, Jet TrackZ",TColor::GetColor(215,181,216),1001,kSolid);
  styles["208004.trkInnerExtr.TrackZJets"]=hist_aes("#mu ID Extr., Jet TrackZ",TColor::GetColor(223,101,176),1001,kSolid);
  styles["208004.trkMS.TrackZJets"]=hist_aes("#mu MS, Jet TrackZ",TColor::GetColor(221,28,119),1001,kSolid);
  styles["208004.trkMuonExtr.TrackZJets"]=hist_aes("#mu MS Extr., Jet TrackZ",TColor::GetColor(152,0,67),1001,kSolid);

  styles["208004..LCTopoJets"]=hist_aes("#mu, Jet LCTopo",TColor::GetColor(253,141,60),1001,kSolid);
  styles["208004..TopoEMJets"]=hist_aes("#mu, Jet TopoEM",TColor::GetColor(120,198,121),1001,kSolid);
  styles["208004..TrackZJets"]=hist_aes("#mu, Jet TrackZ",TColor::GetColor(223,101,176),1001,kSolid);


  styles["LCTopoJets"]=hist_aes("Calo Jets",TColor::GetColor(55,126,184),1001,kSolid);
  styles["MuonLCTopoJets"]=hist_aes("Muon+Calo Jets ",TColor::GetColor(228,26,28),1001,kSolid); 
  styles["TrackZJets"]=hist_aes("Track Jets",TColor::GetColor(77,175,74),1001,kSolid); 

  styles["singlet"]=hist_aes("Singlet",TColor::GetColor(228,26,28),1001,kSolid); 
  styles["octet"]=hist_aes("Octet",TColor::GetColor(55,126,184),1001,kSolid);

  styles["periodA"]=data_aes("Period A Data",TColor::GetColor(0,0,0),kFullCircle,kSolid);
  styles["master"]=data_aes("MC Comparison ",TColor::GetColor(0,0,0),kDot,kSolid);
}
