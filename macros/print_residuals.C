{
  gROOT->ProcessLine(".L ./macros/AtlasStyle.C++ ");
  gROOT->ProcessLine("AtlasStyle style;");
  gROOT->ProcessLine("style.SetAtlasStyle();");
  TFile *_file0 = TFile::Open("fitresult.root");
  RooWorkspace* wkspc;
  _file0->GetObject("workspace",wkspc);
  RooAbsPdf* pdf = wkspc->pdf("model");
  RooRealVar* mass = wkspc->var("jpsi_m");
  TFile* file = TFile::Open("full2012.mini.root");
  TTree* tree;
  file->GetObject("mini",tree);
  RooRealVar* tau = wkspc->var("jpsi_tau");
  RooDataSet data("data","data",RooArgSet(*mass,*tau),RooFit::Import(*tree));
  TCanvas canv("canv","canv",600,600);
  RooPlot* frame = mass->frame(200);
  RooHist* hist;
  data.plotOn(frame);
  pdf->plotOn(frame);
  hist = frame->residHist();
  hist->GetYaxis()->SetTitle("Fit Residual");
  hist->GetXaxis()->SetTitle("J/#psi Mass [GeV]");
  hist->Draw();
  canv.SaveAs("jpsi_m_residual.pdf");
  canv.Clear();
  frame = tau->frame(200);
  data.plotOn(frame);
  pdf->plotOn(frame);
  hist = frame->residHist();
  hist->GetYaxis()->SetTitle("Fit Residual");
  hist->GetXaxis()->SetTitle("J/#psi Proper Decay Time [ps]");
  hist->SetMaximum(1000);
  hist->SetMinimum(-1500);
  hist->Draw();
  
  canv.SaveAs("lifetime_residual.pdf");
}
