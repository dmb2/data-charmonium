{
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
  RooPlot* frame = mass->frame(200);
  RooHist* hist;
  data.plotOn(frame);
  pdf->plotOn(frame);
  hist = frame->residHist();
  hist->Draw();
  c1->SaveAs("jpsi_m_residual.pdf");
  c1->Clear();
  frame = tau->frame(200);
  data.plotOn(frame);
  pdf->plotOn(frame);
  hist = frame->residHist();
  hist->SetMaximum(1000);
  hist->SetMinimum(-1500);
  hist->Draw();
  
  c1->SaveAs("lifetime_residual.pdf");
}
