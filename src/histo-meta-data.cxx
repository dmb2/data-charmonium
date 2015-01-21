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
  HistBook["jet_z"]=new TH1D("jet_z","Jet Z;z;evts/binwidth",50,0,1.);

  HistBook["tau1"]=new TH1D("tau1","N Subjettiness #tau_{1};#tau_{1};evts/binwidth",50,0,1.);
  HistBook["tau2"]=new TH1D("tau2","N Subjettiness #tau_{2};#tau_{2};evts/binwidth",50,0,1.);
  HistBook["tau3"]=new TH1D("tau3","N Subjettiness #tau_{3};#tau_{3};evts/binwidth",50,0,1.);
  HistBook["tau32"]=new TH1D("tau32","N Subjettiness #tau_{32};#tau_{32};evts/binwidth",50,0,1.2);
  HistBook["tau21"]=new TH1D("tau21","N Subjettiness #tau_{21};#tau_{21};evts/binwidth",50,0,1.2);
  HistBook["delta_r"]=new TH1D("delta_r","#Delta R(J/#psi,Jet); #Delta R; evts/binwidth",50,0,1.);
  HistBook["jpsi_pt"]=new TH1D("jpsi_pt","J/#psi p_{T};p_{T} [GeV];evts/binwidth",50,0,300);
  HistBook["jpsi_eta"]=new TH1D("jpsi_eta","J/#psi #eta;#eta;evts/binwidth",50,-2.6,2.6);
  HistBook["jpsi_e"]=new TH1D("jpsi_e","J/#psi E;E [GeV]; evts/binwidth",50,0,600);
  HistBook["jpsi_m"]=new TH1D("jpsi_m","J/#psi Mass; M [GeV]; evts/binwidth",50,2.,4.);
  HistBook["jpsi_lxy"]=new TH1D("jpsi_lxy","J/#psi L_{xy}; L_{xy} [mm]; evts/binwidth",50,-4,4.);
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
  leg_map["3S1_1"]="^{3}S^{(1)}_{1}";
  leg_map["3S1_8"]="^{3}S^{(1)}_{8}";
  leg_map["3PJ_1"]="^{3}P^{(1)}_{J}";
  leg_map["3PJ_8"]="^{3}P^{(8)}_{J}";

  leg_map["108601.PythiaBc_Bc_JPsi_mu2p5mu2p5_Pi"]="B_{c}#rightarrow J/#psi #pi";
  leg_map["108606.PythiaBc_Bc_JPsi_mu2p5mu2p5_3Pi"]="B_{c}#rightarrow J/#psi 3#pi";
  leg_map["208022.Pythia8B_AU2_CTEQ6L1_pp_Psi2S_JpsiPiPi"]="#psi(2S) #rightarrow J/#psi 2#pi";
  leg_map["208023.Pythia8B_AU2_CTEQ6L1_pp_X3872_JpsiPiPi"]="X(3872) #rightarrow J/#psi 2#pi";
  leg_map["208401.Pythia8B_AU2_CTEQ6L1_Bs_Jpsimu4mu4_phi"]="B_{s}#rightarrow J/#psi #phi";
  leg_map["208413.Pythia8B_AU2_CTEQ6L1_chib_Jpsimu3p5mu3p5Jpsimu3p5mu3p5"]="#chi_{b}#rightarrow 2 J/#psi";

  leg_map["208004.trkComb.LCTOPO"]="#mu Comb, Jet LCTopo";
  leg_map["208004.trkInnerExtr.LCTOPO"]="#mu ID Extr., Jet LCTopo";
  leg_map["208004.trkMS.LCTOPO"]="#mu MS, Jet LCTopo";
  leg_map["208004.trkMuonExtr.LCTOPO"]="#mu MS Extr., Jet LCTopo";

  leg_map["208004.trkComb.TOPOEM"]="#mu Comb, Jet TopoEM";
  leg_map["208004.trkInnerExtr.TOPOEM"]="#mu ID Extr., Jet TopoEM";
  leg_map["208004.trkMS.TOPOEM"]="#mu MS, Jet Topoem";
  leg_map["208004.trkMuonExtr.TOPOEM"]="#mu MS Extr., Jet TopoEM";

  leg_map["208004.trkComb.TRACKZ"]="#mu Comb, Jet TrackZ";
  leg_map["208004.trkInnerExtr.TRACKZ"]="#mu ID Extr., Jet TrackZ";
  leg_map["208004.trkMS.TRACKZ"]="#mu MS, Jet TrackZ";
  leg_map["208004.trkMuonExtr.TRACKZ"]="#mu MS Extr., Jet TrackZ";

  leg_map["master"]="Signal";
}
void init_colors(std::map<std::string,int>& colors){
  // Octet colors
  colors["1S0_8"]=TColor::GetColor(222,235,247);
  colors["3S1_8"]=TColor::GetColor(158,202,225);
  colors["3PJ_8"]=TColor::GetColor(49,130,189);
  // Singlet colors
  colors["3S1_1"]=TColor::GetColor(252,146,114);
  colors["3PJ_1"]=TColor::GetColor(222,45,38);
  // non-prompt BKG Colors
  colors["108601.PythiaBc_Bc_JPsi_mu2p5mu2p5_Pi"]=TColor::GetColor(186,228,179);
  colors["108606.PythiaBc_Bc_JPsi_mu2p5mu2p5_3Pi"]=TColor::GetColor(116,196,118);
  colors["208401.Pythia8B_AU2_CTEQ6L1_Bs_Jpsimu4mu4_phi"]=TColor::GetColor(49,163,84);
  colors["208413.Pythia8B_AU2_CTEQ6L1_chib_Jpsimu3p5mu3p5Jpsimu3p5mu3p5"]=TColor::GetColor(0,109,44);
  // feed down
  colors["208022.Pythia8B_AU2_CTEQ6L1_pp_Psi2S_JpsiPiPi"]=TColor::GetColor(158,154,200);
  colors["208023.Pythia8B_AU2_CTEQ6L1_pp_X3872_JpsiPiPi"]=TColor::GetColor(106,81,163);
  //Oranges
  colors["208004.trkComb.LCTOPO"]=TColor::GetColor(254,204,92);
  colors["208004.trkInnerExtr.LCTOPO"]=TColor::GetColor(253,141,60);
  colors["208004.trkMS.LCTOPO"]=TColor::GetColor(240,59,32);
  colors["208004.trkMuonExtr.LCTOPO"]=TColor::GetColor(189,0,38);
  //Greens 
  colors["208004.trkComb.TOPOEM"]=TColor::GetColor(194,230,153);
  colors["208004.trkInnerExtr.TOPOEM"]=TColor::GetColor(120,198,121);
  colors["208004.trkMS.TOPOEM"]=TColor::GetColor(49,163,84);
  colors["208004.trkMuonExtr.TOPOEM"]=TColor::GetColor(0,104,55);
  // Pinks
  colors["208004.trkComb.TRACKZ"]=TColor::GetColor(215,181,216);
  colors["208004.trkInnerExtr.TRACKZ"]=TColor::GetColor(223,101,176);
  colors["208004.trkMS.TRACKZ"]=TColor::GetColor(221,28,119);
  colors["208004.trkMuonExtr.TRACKZ"]=TColor::GetColor(152,0,67);

  colors["master"]=TColor::GetColor(0,0,0);
}
