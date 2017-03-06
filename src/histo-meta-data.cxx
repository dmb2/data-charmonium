#include "histo-meta-data.hh"
#include "TH1D.h"
#include "TH2D.h"
#include "TColor.h"
void init_hist2D_book(std::map<std::string,TH2D*>& Hist2DBook){
  Hist2DBook["jet_z:jet_e"] = new TH2D("jet_z:jet_E","Jet E vs Jet z;E [GeV]; z",
				       25,0,500,
				       10,0,1.2);
  Hist2DBook["jet_e:jet_z"] = new TH2D("jet_E:jet_z","Jet z vs Jet E;z;E [GeV]",
				       25,0,1.2,
				       10,0,500);
  Hist2DBook["jpsi_pt:jet_z"] = new TH2D("jpsi_pt:jet_z","Jet z vs J/#psi p_T;p_T [GeV];E [GeV]",
					 25,0,1.2,
					 10,0,250);
  Hist2DBook["jet_z:jet_e"]->GetYaxis()->SetNdivisions(508);
  Hist2DBook["jet_z:jet_e"]->GetXaxis()->SetNdivisions(508);
}
void fix_axis_labels(TH1* hist){
  char axis_title[200];
  std::string unit;
  std::string x_axis_title(hist->GetXaxis()->GetTitle());
  size_t start = x_axis_title.rfind("[");
  size_t end = x_axis_title.rfind("]");
  unit = start == std::string::npos ? "" : x_axis_title.substr(start,end);
  snprintf(axis_title,200,"Events / %.3g %s",hist->GetBinWidth(1),unit.c_str());
  hist->GetYaxis()->SetTitle(axis_title);
}

void init_hist_book(std::map<std::string,TH1D*>& HistBook){
  HistBook["jet_pt"]=new TH1D("jet_pt","Jet p_{T};Jet p_{T} [GeV];evts/binwidth",50,0,400);
  HistBook["jet_eta"]=new TH1D("jet_eta","Jet #eta;Jet #eta;evts/binwidth",50,-2.6,2.6);
  HistBook["jet_e"]=new TH1D("jet_e","Jet E;Jet E [GeV]; evts/binwidth",50,0,500);
  HistBook["jet_m"]=new TH1D("jet_m","Jet Mass;Jet  M [GeV]; evts/binwidth",50,0,45);
  HistBook["jet_z"]=new TH1D("jet_z","Jet Z;Jet z;evts/binwidth",50,0,1.1);

  HistBook["tau1"]=new TH1D("tau1","N Subjettiness #tau_{1};#tau_{1};evts/binwidth",50,0,1.2);
  HistBook["tau2"]=new TH1D("tau2","N Subjettiness #tau_{2};#tau_{2};evts/binwidth",50,0,0.5);
  HistBook["tau3"]=new TH1D("tau3","N Subjettiness #tau_{3};#tau_{3};evts/binwidth",50,0,0.5);
  HistBook["tau32"]=new TH1D("tau32","N Subjettiness #tau_{32};#tau_{32};evts/binwidth",50,0,1.2);
  HistBook["tau21"]=new TH1D("tau21","N Subjettiness #tau_{21};#tau_{21};evts/binwidth",50,0,1.2);
  HistBook["delta_r"]=new TH1D("delta_r","#Delta R(J/#psi,Jet);#Delta R(J/#psi,Jet); evts/binwidth",50,0,.08);

  HistBook["mu1_pt"]=new TH1D("mu1_pt","#mu_1 p_{T};#mu_1 p_{T} [GeV];evts/binwidth",50,0,140);
  HistBook["mu2_pt"]=new TH1D("mu2_pt","#mu_2 p_{T};#mu_2 p_{T} [GeV];evts/binwidth",50,0,140);
  HistBook["mu1_eta"]=new TH1D("mu1_eta","#mu_1 #eta;#mu_1 #eta;evts/binwidth",50,-2.6,2.6);
  HistBook["mu2_eta"]=new TH1D("mu2_eta","#mu_2 #eta;#mu_2 #eta;evts/binwidth",50,-2.6,2.6);
  
  HistBook["vtx_ntrk"] = new TH1D("vtx_ntrk","Track Multiplicity in PV;n_{trk};evts/binwidth",50,0,200);
  HistBook["vtx_chi2"] = new TH1D("vtx_chi2","#chi^2 of PV fit;PV #chi^2;evts/binwidth",50,0,350);
  HistBook["vtx_n"] = new TH1D("vtx_n","PV multiplicity;Number of PV;evts/binwidth",41,0,40);
  

  HistBook["jpsi_pt"]=new TH1D("jpsi_pt","J/#psi p_{T};J/#psi p_{T} [GeV];evts/binwidth",50,0,200);
  HistBook["jpsi_eta"]=new TH1D("jpsi_eta","J/#psi #eta;J/#psi #eta;evts/binwidth",50,-2.6,2.6);
  HistBook["jpsi_e"]=new TH1D("jpsi_e","J/#psi E;J/#psi E [GeV]; evts/binwidth",50,0,600);
  HistBook["jpsi_m"]=new TH1D("jpsi_m","J/#psi Mass;J/#psi E M(#mu#mu) [GeV]; evts/binwidth",50,2.,4.);
  HistBook["jpsi_lxy"]=new TH1D("jpsi_lxy","J/#psi L_{xy}; L_{xy} [mm]; evts/binwidth",50,-4,10);
  HistBook["jpsi_tau"]= new TH1D("jpsi_tau","J/#psi c#tau; c#tau [mm]; evts/binwidth",50,-1,5);

  HistBook["pileup"]= new TH1D("pileup","Average interactions per bunch crossing; Avg. Interactions <#mu>; evts/binwidth",41,-0.5,40.5);
  
  HistBook["delta_r"]->GetXaxis()->SetNdivisions(505);
  // Doctor the y axis titles for binwidth
  for(std::map<std::string,TH1D*>::iterator it=HistBook.begin();
      it!=HistBook.end(); ++it){
    fix_axis_labels(it->second);
  }
}
void init_cut_names(std::map<std::string,std::string>& cut_names){
  cut_names["mu_trigger_p"]="Trigger";
  cut_names["num_jets_p"]="N_{j} #geq 1"; 
  cut_names["jpsi_pt_p"]="p_{T}(J/#psi) > 45 GeV";    
  cut_names["jpsi_eta_p"]="|#eta(J/#psi)| < 2.5";
  cut_names["jpsi_rap_p"]="|y(J/#psi)| < 2.0";
  cut_names["mumu_eta_p"]="|#eta(#mu)| < 2.3";
  cut_names["jet_eta_p"]="|#eta(jet)| < 2.5"; 
  cut_names["delta_r_p"]="#Delta R(Jet;J/#psi) < 0.4";    
  cut_names["jet_pt_p"]="p_{T}(jet) > 45 GeV";
}

void init_hist_styles(std::map<std::string,aesthetic>& styles){
  int octet_blues[3];
  octet_blues[0]=TColor::GetColor(0,0x5a,0xc8);
  octet_blues[1]=TColor::GetColor(0,0x0a0,0xfa);
  octet_blues[2]=TColor::GetColor(0x14,0xd2,0xdc);
  int singlet_reds[2];
  singlet_reds[0]=TColor::GetColor(0xfa,0x78,0xfa);
  singlet_reds[1]=TColor::GetColor(0xaa,0x0a,0x3c);
  styles["global_syst_err"]=hist_aes("Systematic Error",TColor::GetColorTransparent(kBlack,0.4),1001,0);
  styles["1S0_8"]=hist_aes("^{1}S^{(8)}_{0} Octet",octet_blues[0],1001,kSolid);
  styles["3S1_8"]=hist_aes("^{3}S^{(8)}_{1} Octet",octet_blues[1],1001,kSolid);
  styles["3PJ_8"]=hist_aes("^{3}P^{(8)}_{J} Octet",octet_blues[2],1001,kSolid);
  // Singlet styles	                    
  styles["3S1_1"]=hist_aes("^{3}S^{(1)}_{1} Singlet",singlet_reds[0],1001,kSolid);
  styles["3PJ_1"]=hist_aes("^{3}P^{(1)}_{J} Singlet",singlet_reds[1],1001,kSolid);  

  // Octet styles
  styles["208024.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_1S0_8"]=hist_aes("^{1}S^{(8)}_{0} Octet",octet_blues[0],1001,kSolid);
  styles["208028.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3S1_8"]=hist_aes("^{3}S^{(8)}_{1} Octet",octet_blues[1],1001,kSolid);
  styles["208026.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3PJ_8"]=hist_aes("^{3}P^{(8)}_{J} Octet",octet_blues[2],1001,kSolid); 
  // Singlet styles						                        
  styles["208027.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3S1_1"]=hist_aes("^{3}S^{(1)}_{1} Singlet",singlet_reds[0],1001,kSolid);
  styles["208025.Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu20_3PJ_1"]=hist_aes("^{3}P^{(1)}_{J} Singlet",singlet_reds[1],1001,kSolid);  

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

  styles["non_prompt"]=hist_aes("Non-Prompt",TColor::GetColor(152,0,67),1001,kSolid);
  styles["feed_down"]=hist_aes("Feed Down",TColor::GetColor(0,104,55),1001,kSolid);

  styles["background"]=hist_aes("Background",TColor::GetColor(0,104,55),1001,kSolid);
  styles["signal"]=hist_aes("Signal",TColor::GetColor(240,59,32),1001,kSolid);
  styles["psi_bkg"]=hist_aes("#psi(2S)",TColor::GetColor(0,104,55),1001,kSolid);
  styles["comb_bkg"]=hist_aes("Combinatoric",TColor::GetColor(240,59,32),1001,kSolid);

  styles["208202.Pythia8B_AU2_CTEQ6L1_bb_Jpsimu6mu6"]=hist_aes("bb Non Prompt Background",TColor::GetColor(120,198,121),1001,kSolid);
  styles["208207.Pythia8B_AU2_CTEQ6L1_anti_bb_Jpsimu6mu6"]=hist_aes(" #bar{bb} Non Prompt Background",TColor::GetColor(120,198,121),1001,kSolid);
  
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

  styles["non_prompt.mu24i_tight"]=hist_aes("MC Isolated",TColor::GetColor(240,59,32),1001,kSolid); 
  styles["non_prompt.mu36_tight"]=hist_aes("MC Non-isolated",TColor::GetColor(152,0,67),1001,kSolid); 
  styles["non_prompt.tau_sb"]=hist_aes("Non-Prompt SB",TColor::GetColor(253,141,60),1001,kSolid);

  styles["non_prompt.signal_region"]=hist_aes("Non-prompt Signal",TColor::GetColor(120,198,121),1001,kSolid);


  styles["full2012.mass_sb_1"]=hist_aes("Data Low Mass SB",TColor::GetColor(215,181,216),1001,kSolid);
  styles["full2012.mass_sb_2"]=hist_aes("Data High Mass SB ",TColor::GetColor(223,101,176),1001,kSolid); 

  styles["full2012.tau_sb"]=hist_aes("Data Tau SB",TColor::GetColor(221,28,119),1001,kSolid); 
  styles["full2012.mu24i_tight"]=hist_aes("Isolated",TColor::GetColor(240,59,32),1001,kSolid); 
  styles["full2012.mu36_tight"]=hist_aes("Non-Isolated",TColor::GetColor(49,163,84),1001,kSolid); 

  styles["LCTopoJets"]=hist_aes("Calo Jets",TColor::GetColor(55,126,184),1001,kSolid);
  styles["MuonLCTopoJets"]=hist_aes("Muon+Calo Jets ",TColor::GetColor(228,26,28),1001,kSolid); 
  styles["TrackZJets"]=hist_aes("Track Jets",TColor::GetColor(77,175,74),1001,kSolid); 

  styles["singlet"]=hist_aes("Singlet",TColor::GetColor(228,26,28),1001,kSolid); 
  styles["octet"]=hist_aes("Octet",TColor::GetColor(55,126,184),1001,kSolid);

  styles["periodA"]=data_aes("Period A Data",TColor::GetColor(0,0,0),kFullCircle,kSolid);
  styles["data"]=data_aes("2012 Data",TColor::GetColor(0,0,0),kFullCircle,kSolid);
  styles["master"]=data_aes("MC Comparison ",TColor::GetColor(0,0,0),kDot,kSolid);
}
