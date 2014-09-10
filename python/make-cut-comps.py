import ROOT


def add_prefix(pre,inList):
    return [pre + s for s in inList]
def get_plot_names():
    three_vec=['pt','eta','e']
    return sum([add_prefix('jet_',three_vec),
                add_prefix('jpsi_',three_vec),
                ['jet_z','delta_r']],[])
def main():
    InFile=ROOT.TFile("cut_tree.root")
    CutTree = InFile.Get("mini");
    pretty_cNames={'num_jets_p':'N_{j} #geq 1', 
                   'jpsi_pt_p':'p_{T}(J/#psi) > 20 GeV',    
                   'jpsi_eta_p':'|#eta(J/#psi)| < 2.5',
                   'delta_r_p':'#Delta R(Jet,J/#psi) < 0.4',    
                   'jet_eta_p':'|#eta(jet)| < 2.5', 
                   'jet_pt_p':'p_{T}(jet) > 20 GeV'}
    cut_branches=['num_jets_p', 'jpsi_pt_p',    
                  'jpsi_eta_p', 'delta_r_p',    
                  'jet_eta_p', 'jet_pt_p']
    plots=get_plot_names()
    truth_plots=add_prefix('truth_',get_plot_names())
    
    hist_book={'jet_pt':ROOT.TH1D("jet_pt",'Jet p_{T};p_{T} [GeV];evts/binwidth',50,0,250),
               'jet_eta':ROOT.TH1D("jet_eta",'Jet #eta;#eta;evts/binwidth',50,-2.6,2.6),
               'jet_e':ROOT.TH1D('jet_e','Jet E;E [GeV]; evts/binwidth',50,0,500),
               'jet_z':ROOT.TH1D('jet_z','Jet Z=p_{T}(J/#psi)/p_{T}(Jet);z;evts/binwidth',50,0,1.),
               'delta_r':ROOT.TH1D('delta_r','#Delta R(J/\psi,Jet); #Delta R; evts/binwidth',50,0,1.),
               'jpsi_pt':ROOT.TH1D("jpsi_pt",'J/#psi p_{T};p_{T} [GeV];evts/binwidth',50,0,200),
               'jpsi_eta':ROOT.TH1D("jpsi_eta",'J/#psi #eta;#eta;evts/binwidth',50,-2.6,2.6),
               'jpsi_e':ROOT.TH1D('jpsi_e','J/#psi E;E [GeV]; evts/binwidth',50,0,600)}
    cut_expr="(%s)==1"
    active_cuts=""
    for plot in plots:
        canv=ROOT.TCanvas('canv_'+plot,"Efficiency Ratio",1800,800)
        canv.Divide(3,2)
        for i in range(1,len(cut_branches)):
            canv.cd(i+1)
            hNameBefore=plot+'_B_'+'_'.join(cut_branches[:i])
            hNameAfter=plot+'_A_'+'_'.join(cut_branches[:i+1])
            hBefore=hist_book[plot].Clone(hNameBefore)
            hAfter=hBefore.Clone(hNameAfter)
            CutTree.Draw("%s>>%s"%(plot,hNameBefore),'*'.join(cut_branches[:i]),"goff")
            CutTree.Draw("%s>>%s"%(plot,hNameAfter),'*'.join(cut_branches[:i+1]),"goff")
            hAfter.Sumw2()
            hBefore.Sumw2()
            hAfter.Divide(hBefore)
            hAfter.Draw()
        canv.SaveAs(plot+'.pdf')
        
            
        # print 'Before:', 
        # print 'After:', cut_expr%('*'.join(cut_branches[:(i+1)]),)
if __name__=="__main__":
    ROOT.gROOT.SetBatch(1)
    ROOT.gStyle.SetOptStat(0)
    main()
