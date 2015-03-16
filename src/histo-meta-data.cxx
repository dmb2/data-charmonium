#include "histo-meta-data.hh"
#include "TH1D.h"
#include "TH2D.h"
#include "TColor.h"
void init_hist2D_book(std::map<std::string,TH2D*>& Hist2DBook){
  Hist2DBook["jet_z:jet_e"] = new TH2D("jet_z:jet_E","Jet E vs Jet z;E [GeV]; z",
				       25,0,500,
				       10,0,1.);
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
  HistBook["delta_r"]=new TH1D("delta_r","#Delta R(J/#psi,Jet); #Delta R; evts/binwidth",50,0,1.);

  HistBook["psi_m"]=new TH1D("psi_m","J/#psi + 2 Tracks Mass; M(#mu #mu Trk Trk) [GeV]; evts/binwidth",50,2.5,6.);

  HistBook["jpsi_pt"]=new TH1D("jpsi_pt","J/#psi p_{T};p_{T} [GeV];evts/binwidth",50,0,300);
  HistBook["jpsi_eta"]=new TH1D("jpsi_eta","J/#psi #eta;#eta;evts/binwidth",50,-2.6,2.6);
  HistBook["jpsi_e"]=new TH1D("jpsi_e","J/#psi E;E [GeV]; evts/binwidth",50,0,600);
  HistBook["jpsi_m"]=new TH1D("jpsi_m","J/#psi Mass; M(#mu#mu) [GeV]; evts/binwidth",50,2.,4.);
  HistBook["jpsi_lxy"]=new TH1D("jpsi_lxy","J/#psi L_{xy}; L_{xy} [mm]; evts/binwidth",50,-4,4.);
  HistBook["jpsi_s"]=new TH1D("jpsi_s","Impact Parameter Significance;S;evts/binwidth",50,0,55000);
}
void init_cut_names(std::map<std::string,std::string>& cut_names){
  cut_names["num_jets_p"]="N_{j} #geq 1"; 
  cut_names["jpsi_pt_p"]="p_{T}(J/#psi) > 20 GeV";    
  cut_names["jpsi_eta_p"]="|#eta(J/#psi)| < 2.5";
  cut_names["delta_r_p"]="#Delta R(Jet;J/#psi) < 0.4";    
  cut_names["jet_eta_p"]="|#eta(jet)| < 2.5"; 
  cut_names["jet_pt_p"]="p_{T}(jet) > 45 GeV";
  cut_names["mu_trigger_p"]="1 #mu Trigger, 2012";

}
void init_leg_names(std::map<std::string,std::string>& leg_map){
  leg_map["1S0_8"]="^{1}S^{(8)}_{0}";
  leg_map["3S1_8"]="^{3}S^{(1)}_{8}";
  leg_map["3PJ_8"]="^{3}P^{(8)}_{J}";

  leg_map["3S1_1"]="^{3}S^{(1)}_{1}";
  leg_map["3PJ_1"]="^{3}P^{(1)}_{J}";

  leg_map["208024.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_1S0_8"]="^{1}S^{(8)}_{0}";
  leg_map["208028.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3S1_8"]="^{3}S^{(1)}_{8}";
  leg_map["208026.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3PJ_8"]="^{3}P^{(8)}_{J}";
							                        
  leg_map["208027.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3S1_1"]="^{3}S^{(1)}_{1}";
  leg_map["208025.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3PJ_1"]="^{3}P^{(1)}_{J}";


  leg_map["108601.PythiaBc_Bc_JPsi_mu2p5mu2p5_Pi"]="B_{c}#rightarrow J/#psi #pi";
  leg_map["108606.PythiaBc_Bc_JPsi_mu2p5mu2p5_3Pi"]="B_{c}#rightarrow J/#psi 3#pi";
  leg_map["208022.Pythia8B_AU2_CTEQ6L1_pp_Psi2S_JpsiPiPi"]="#psi(2S) #rightarrow J/#psi 2#pi";
  leg_map["208023.Pythia8B_AU2_CTEQ6L1_pp_X3872_JpsiPiPi"]="X(3872) #rightarrow J/#psi 2#pi";
  leg_map["208401.Pythia8B_AU2_CTEQ6L1_Bs_Jpsimu4mu4_phi"]="B_{s}#rightarrow J/#psi #phi";
  leg_map["208413.Pythia8B_AU2_CTEQ6L1_chib_Jpsimu3p5mu3p5Jpsimu3p5mu3p5"]="#chi_{b}#rightarrow 2 J/#psi";

  leg_map["208004.trkComb.LCTopoJets"]="#mu Comb, Jet LCTopo";
  leg_map["208004.trkInnerExtr.LCTopoJets"]="#mu ID Extr., Jet LCTopo";
  leg_map["208004.trkMS.LCTopoJets"]="#mu MS, Jet LCTopo";
  leg_map["208004.trkMuonExtr.LCTopoJets"]="#mu MS Extr., Jet LCTopo";

  leg_map["208004.trkComb.TopoEMJets"]="#mu Comb, Jet TopoEM";
  leg_map["208004.trkInnerExtr.TopoEMJets"]="#mu ID Extr., Jet TopoEM";
  leg_map["208004.trkMS.TopoEMJets"]="#mu MS, Jet Topoem";
  leg_map["208004.trkMuonExtr.TopoEMJets"]="#mu MS Extr., Jet TopoEM";

  leg_map["208004.trkComb.TrackZJets"]="#mu Comb, Jet TrackZ";
  leg_map["208004.trkInnerExtr.TrackZJets"]="#mu ID Extr., Jet TrackZ";
  leg_map["208004.trkMS.TrackZJets"]="#mu MS, Jet TrackZ";
  leg_map["208004.trkMuonExtr.TrackZJets"]="#mu MS Extr., Jet TrackZ";

  leg_map["208004..LCTopoJets"]="#mu, Jet LCTopo";
  leg_map["208004..TopoEMJets"]="#mu, Jet TopoEM";
  leg_map["208004..TrackZJets"]="#mu, Jet TrackZ";

  leg_map["LCTopoJets"]="Jet LCTopo";
  leg_map["MULCTopoJets"]="Jet #mu+LCTopo";
  leg_map["TopoEMJets"]="Jet TopoEM";
  leg_map["TrackZJets"]="Jet TrackZ";

  leg_map["singlet.MULCTopoJets"]="Singlet";
  leg_map["octet.MULCTopoJets"]="Octet";

  leg_map["master"]="Signal";
  leg_map["periodA"]="Period A Data";
}
void init_colors(std::map<std::string,int>& colors){
  // Octet colors
  colors["1S0_8"]=TColor::GetColor(222,235,247);
  colors["3S1_8"]=TColor::GetColor(158,202,225);
  colors["3PJ_8"]=TColor::GetColor(49,130,189);
  // Singlet colors
  colors["3S1_1"]=TColor::GetColor(252,146,114);
  colors["3PJ_1"]=TColor::GetColor(222,45,38);  

  // Octet colors
  colors["208024.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_1S0_8"]=TColor::GetColor(222,235,247);
  colors["208028.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3S1_8"]=TColor::GetColor(158,202,225);
  colors["208026.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3PJ_8"]=TColor::GetColor(49,130,189); 
  // Singlet colors
  colors["208027.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3S1_1"]=TColor::GetColor(252,146,114);
  colors["208025.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3PJ_1"]=TColor::GetColor(222,45,38);  

  // non-prompt BKG Colors
  colors["108601.PythiaBc_Bc_JPsi_mu2p5mu2p5_Pi"]=TColor::GetColor(186,228,179);
  colors["108606.PythiaBc_Bc_JPsi_mu2p5mu2p5_3Pi"]=TColor::GetColor(116,196,118);
  colors["208401.Pythia8B_AU2_CTEQ6L1_Bs_Jpsimu4mu4_phi"]=TColor::GetColor(49,163,84);
  colors["208413.Pythia8B_AU2_CTEQ6L1_chib_Jpsimu3p5mu3p5Jpsimu3p5mu3p5"]=TColor::GetColor(0,109,44);
  // feed down
  colors["208022.Pythia8B_AU2_CTEQ6L1_pp_Psi2S_JpsiPiPi"]=TColor::GetColor(158,154,200);
  colors["208023.Pythia8B_AU2_CTEQ6L1_pp_X3872_JpsiPiPi"]=TColor::GetColor(106,81,163);
  //Oranges
  colors["208004.trkComb.LCTopoJets"]=TColor::GetColor(254,204,92);
  colors["208004.trkInnerExtr.LCTopoJets"]=TColor::GetColor(253,141,60);
  colors["208004.trkMS.LCTopoJets"]=TColor::GetColor(240,59,32);
  colors["208004.trkMuonExtr.LCTopoJets"]=TColor::GetColor(189,0,38);
  //Greens 
  colors["208004.trkComb.TopoEMJets"]=TColor::GetColor(194,230,153);
  colors["208004.trkInnerExtr.TopoEMJets"]=TColor::GetColor(120,198,121);
  colors["208004.trkMS.TopoEMJets"]=TColor::GetColor(49,163,84);
  colors["208004.trkMuonExtr.TopoEMJets"]=TColor::GetColor(0,104,55);
  // Pinks
  colors["208004.trkComb.TrackZJets"]=TColor::GetColor(215,181,216);
  colors["208004.trkInnerExtr.TrackZJets"]=TColor::GetColor(223,101,176);
  colors["208004.trkMS.TrackZJets"]=TColor::GetColor(221,28,119);
  colors["208004.trkMuonExtr.TrackZJets"]=TColor::GetColor(152,0,67);

  colors["208004..LCTopoJets"]=TColor::GetColor(253,141,60);
  colors["208004..TopoEMJets"]=TColor::GetColor(120,198,121);
  colors["208004..TrackZJets"]=TColor::GetColor(223,101,176);


  colors["LCTopoJets"]=TColor::GetColor(55,126,184);
  colors["MuonLCTopoJets"]=TColor::GetColor(228,26,28); 
  colors["TrackZJets"]=TColor::GetColor(77,175,74); 

  colors["singlet.MuonLCTopoJets"]=TColor::GetColor(228,26,28); 
  colors["octet.MuonLCTopoJets"]=TColor::GetColor(55,126,184);
  

  colors["periodA"]=TColor::GetColor(0,0,0);
  colors["master"]=TColor::GetColor(0,0,0);
}
