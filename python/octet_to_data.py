#!/usr/bin/env python
import ROOT
from optparse import OptionParser, OptionGroup
from sys import stdout, stderr, exit

parser = OptionParser(usage='Usage: %prog [options] [input file list]')
parser.add_option('-i','--input',dest='input_fname',
                  help='Data file containing SPlot files',metavar='FNAME')
parser.add_option('-n',action="store_true",dest='do_nsj',
                  help='Include n-subjettiness plots as well')
(options,args)=parser.parse_args()
def main():
    ROOT.gROOT.SetBatch(1)
    ROOT.gStyle.SetPalette(53)
    ROOT.gStyle.SetOptStat(0)
    print ROOT.gROOT.LoadMacro("./python/AtlasStyle.C")
    astyle = ROOT.AtlasStyle()
    astyle.SetAtlasStyle()
    if len(args)==0:
        parser.print_help()
        exit(1)
    print options.input_fname
    data_file = ROOT.TFile.Open(options.input_fname)
    pub_vars = ['delta_r','jet_z']
    nsj_vars = ['tau1','tau2','tau3','tau21','tau32']
    if(options.do_nsj):
        pub_vars+=nsj_vars
    files=[] 
    trees={} 
    for fname in args:
        tfile = ROOT.TFile.Open(fname)
        files+=[tfile]
        trees[fname.split('.')[1]]= tfile.Get("mini")
    octet_blues={}
    octet_blues["Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu0_1S0_8"]=ROOT.TColor.GetColor(0,0x5a,0xc8);
    octet_blues["Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu0_3S1_8"]=ROOT.TColor.GetColor(0,0x0a0,0xfa);
    octet_blues["Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu0_3PJ_8"]=ROOT.TColor.GetColor(0x14,0xd2,0xdc);
    leg_names ={} 
    leg_names["Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu0_1S0_8"]="^{1}S^{(8)}_{0} Octet"
    leg_names["Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu0_3S1_8"]="^{3}S^{(8)}_{1} Octet"
    leg_names["Pythia8B_AU2_CTEQ6L1_pp_Jpsimu20mu0_3PJ_8"]="^{3}P^{(8)}_{J} Octet"
    for plot in pub_vars:
        print "Processing plot: "+plot
        hist = data_file.Get(plot)
        hists=[]
        leg = ROOT.TLegend(0.65,0.5,0.92,0.92)
        if plot == "jet_z":
            leg = ROOT.TLegend(0.25,0.68,0.4,0.92)
        leg.SetFillColor(0);
        leg.SetFillStyle(0);
        leg.SetBorderSize(0);
        leg.SetTextSize(0.04)
        for key in trees:
            base_hist = ROOT.TH1D(plot+"_"+key,hist.GetTitle(),hist.GetNbinsX(),
                                  hist.GetXaxis().GetXmin(),
                                  hist.GetXaxis().GetXmax())
            # base_hist.Reset()
            cut_expr = plot+">>"+base_hist.GetName()
            trees[key].Draw(cut_expr)
            base_hist.SetMarkerStyle(0);
            base_hist.SetLineColor(octet_blues[key])
            base_hist.SetLineWidth(2)
            base_hist.Scale(1/base_hist.Integral())
            leg.AddEntry(base_hist,leg_names[key],"l")
            hists+=[base_hist]
        canv = ROOT.TCanvas("canv_"+plot,"Canvas",600,600)
        hist.SetName(plot+"_splot")
        hist.Scale(1/hist.Integral())
        hist.GetYaxis().SetTitle("Arbitrary Units")
        hist.Draw()
        draw_max = hist.GetMaximum()
        leg.AddEntry(hist,"2012 Data","lp")
        for dhist in hists:
            dhist.Draw("H same")
            if dhist.GetMaximum() > draw_max:
                draw_max = dhist.GetMaximum()
        hist.SetMaximum(1.2*draw_max)
        leg.Draw()
        hist.Draw("same")
        canv.SaveAs(plot+"_octet_cmp.pdf")
            
if __name__ == "__main__":
    exit(main())
