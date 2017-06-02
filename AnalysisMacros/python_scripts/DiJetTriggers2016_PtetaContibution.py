#! /usr/bin/env python 
# plot events/lumi for dijet triggers in 2016 used for JERC
# cd /nfs/dust/cms/user/karavdia/DIJet_Triggers2016/ClosureTest_Summer16_03Feb2017_V3
# python /afs/desy.de/user/k/karavdia/xxl/af-cms/CMSSW_8_0_24_patch1/src/UHH2/ZprimeSemiLeptonic/macros/DiJetTriggers2016_PtetaContibution.py
from ROOT import *
#TH1.DrawClone._creates = False
import sys
import numpy


samplelist = {'DATA':'uhh2.AnalysisModuleRunner.DATA.DATA_RunALL_AK4CHS.root'}
#path = {'DiPFJetAve40':'HLT_DiPFJetAve40/', 'DiPFJetAve60':'HLT_DiPFJetAve60/', 'DiPFJetAve80':'HLT_DiPFJetAve80/', 'DiPFJetAve140':'HLT_DiPFJetAve140/','DiPFJetAve200':'HLT_DiPFJetAve200/','DiPFJetAve260':'HLT_DiPFJetAve260/','DiPFJetAve320':'HLT_DiPFJetAve320/','DiPFJetAve400':'HLT_DiPFJetAve400/','DiPFJetAve500':'HLT_DiPFJetAve500/'}
#thresholds = {'DiPFJetAve40':'51', 'DiPFJetAve60':'73', 'DiPFJetAve80':'95', 'DiPFJetAve140':'163','DiPFJetAve200':'230','DiPFJetAve260':'299','DiPFJetAve320':'365','DiPFJetAve400':'453','DiPFJetAve500':'566'}
#path = {'DiPFJetAve40':'HLT_DiPFJetAve40/'} 
path = {'DiPFJetAve60_HFJEC':'HLT_DiPFJetAve60_HFJEC/','DiPFJetAve80_HFJEC':'HLT_DiPFJetAve80_HFJEC/','DiPFJetAve100_HFJEC':'HLT_DiPFJetAve100_HFJEC/','DiPFJetAve160_HFJEC':'HLT_DiPFJetAve160_HFJEC/','DiPFJetAve220_HFJEC':'HLT_DiPFJetAve220_HFJEC/','DiPFJetAve300_HFJEC':'HLT_DiPFJetAve300_HFJEC/'}
thresholds = {'DiPFJetAve60_HFJEC':'100','DiPFJetAve80_HFJEC':'126','DiPFJetAve100_HFJEC':'152','DiPFJetAve160_HFJEC':'250','DiPFJetAve220_HFJEC':'319','DiPFJetAve300_HFJEC':'433'}
#path = {'DiPFJetAve60_HFJEC':'HLT_DiPFJetAve60_HFJEC/'}
#thresholds = {'DiPFJetAve60_HFJEC':'100'}
read_hist = {'Pt_ave':'pt_ave_rebin'}
name_hist = {'Pt_ave':'p^{ave}_{T}, GeV'}

#TCanvas
cSigEff_Sig = {} 
#TCanvas("c1","EventsLumi_Central",800,600)
#output = TFile('EleID_Eff_Plots.root', 'RECREATE')
#j = 0
trg_hist = {}
line = {}
i = 0
for key_sample in samplelist:
    myfile = TFile(samplelist[key_sample])
    cSigEff_Sig[key_sample] = TCanvas("cDIjetTriggers_ptAve"+key_sample,"DIjetTriggers_ptAve_"+key_sample,800,600)
    legend = TLegend(.70,.55,.99,.95) 
    gStyle.SetOptStat(0)
    gStyle.SetOptTitle(0)
    for key_path in path:
        i = i+1
        for key_hist in read_hist:
            full_key_hist = path[key_path]+read_hist[key_hist]
            print "full key list",  full_key_hist
            trg_hist[full_key_hist] = myfile.Get(full_key_hist).Clone()
            trg_hist[full_key_hist].SetMarkerStyle(20)
            trg_hist[full_key_hist].SetMarkerSize(1.2)
            #trg_hist[full_key_hist].SetMarkerColor(kRed+i)
            trg_hist[full_key_hist].SetMarkerColor(i)
            trg_hist[full_key_hist].GetYaxis().SetTitle(name_hist[key_hist])
            trg_hist[full_key_hist].GetYaxis().SetTitleSize(0.04)
            trg_hist[full_key_hist].GetXaxis().SetTitleSize(0.04)
            trg_hist[full_key_hist].GetYaxis().SetLabelSize(0.04)
            trg_hist[full_key_hist].GetXaxis().SetLabelSize(0.04)
#            print "Y label size = ", trg_hist[full_key_hist].GetYaxis().GetLabelSize()
            trg_hist[full_key_hist].GetXaxis().SetRangeUser(0,1000)
            trg_hist[full_key_hist].GetYaxis().SetRangeUser(0,1000000)
     #       trg_hist[full_key_hist].GetYaxis().SetRangeUser(0,40000)
            trg_hist[full_key_hist].GetXaxis().SetTitle(name_hist[key_hist])
            trg_hist[full_key_hist].Draw('same')
            legend.AddEntry(trg_hist[full_key_hist],key_path,"lp")
            line[full_key_hist] = TLine(float(thresholds[key_path]),0,float(thresholds[key_path]),1000000);
            line[full_key_hist].SetLineColor(kBlack)
            line[full_key_hist].SetLineWidth(2)
            line[full_key_hist].SetLineStyle(2)
            line[full_key_hist].Draw('same')
    legend.Draw()
    cSigEff_Sig[key_sample].SaveAs('DIjetTriggersHF_ptAve_'+key_sample+'.pdf')
