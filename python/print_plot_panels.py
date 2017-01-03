import ROOT
def print_summary(var):
    canv = ROOT.TCanvas("canv", "Canvas",1200,1200)
    plot_type=['response','res_dif','rel_res']
    sub_systems=['LCTopoJets','MuonLCTopoJets','TrackZJets']
    canv.Divide(3,3)
    pad_pos=1
    files=[]
    decorator = ROOT.TLatex()
    decorator.SetTextSize(1.);
    for pt in plot_type:
        for system in sub_systems:
            print pad_pos,pt,system
            files+=[ROOT.TFile.Open("%s/%s_nominal_%s.root"%(system,var,pt))]
            pad_canv = files[-1].Get("canv_"+var)
            contents=pad_canv.GetListOfPrimitives()
            # pad_canv.SaveAs("%s/%s_%s.png"%(system,var,pt))
            canv.cd(pad_pos)
            for obj in contents:
                obj.Draw("COLZ")
            # pad_canv.Draw("COLZ")
            pad_pos+=1
    canv.SaveAs(var+"_summary.pdf")
def main():
    ROOT.gROOT.SetBatch(1)
    ROOT.gStyle.SetPalette(53)
    ROOT.gStyle.SetOptStat(0)
    variables=['jet_e','jet_eta','jet_pt','jet_z']
    for var in variables:
        print_summary(var)
main()
