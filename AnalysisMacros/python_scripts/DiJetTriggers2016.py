#! /usr/bin/env python 
# plot events/lumi for dijet triggers in 2016 used for JERC
# cd /nfs/dust/cms/user/karavdia/DIJet_Triggers2016/ClosureTest_Summer16_03Feb2017_V3
# python /afs/desy.de/user/k/karavdia/xxl/af-cms/CMSSW_8_0_24_patch1/src/UHH2/ZprimeSemiLeptonic/macros/DiJetTriggers2016.py
from ROOT import *
#TH1.DrawClone._creates = False
import sys
import numpy


samplelist = {'DATA':'uhh2.AnalysisModuleRunner.DATA.DATA_RunALL_AK4CHS.root'}
#path = {'DiPFJetAve40':'Lumi_Trig40/', 'DiPFJetAve60':'Lumi_Trig60/', 'DiPFJetAve80':'Lumi_Trig80/', 'DiPFJetAve140':'Lumi_Trig140/','DiPFJetAve200':'Lumi_Trig200/','DiPFJetAve260':'Lumi_Trig260/','DiPFJetAve320':'Lumi_Trig320/','DiPFJetAve400':'Lumi_Trig400/','DiPFJetAve500':'Lumi_Trig500/'} 
#path = {'DiPFJetAve40':'Lumi_Trig40/'} 
path = {'DiPFJetAve60_HFJEC':'Lumi_TrigHF60/','DiPFJetAve80_HFJEC':'Lumi_TrigHF80/','DiPFJetAve100_HFJEC':'Lumi_TrigHF100/','DiPFJetAve160_HFJEC':'Lumi_TrigHF160/','DiPFJetAve220_HFJEC':'Lumi_TrigHF220/','DiPFJetAve300_HFJEC':'Lumi_TrigHF300/'}
read_hist = {'luminosity':'luminosity'}
name_hist = {'luminosity':'events/500pb^{-1}'}

#TCanvas
cSigEff_Sig = {} 
#TCanvas("c1","EventsLumi_Central",800,600)
#output = TFile('EleID_Eff_Plots.root', 'RECREATE')
#j = 0
trg_hist = {}
i = 0
for key_sample in samplelist:
    myfile = TFile(samplelist[key_sample])
    for key_path in path:
        cSigEff_Sig[key_sample] = TCanvas("cEventsPerLumi_"+key_sample+key_path,"EventsPerLumi_"+key_sample+key_path,800,600)
        legend = TLegend(.70,.85,.99,.95) 
        gStyle.SetOptStat(0)
        gStyle.SetOptTitle(0)
        i = i+1
        for key_hist in read_hist:
            full_key_hist = path[key_path]+read_hist[key_hist]
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
#            trg_hist[full_key_hist].GetYaxis().SetRangeUser(0,100000)
            trg_hist[full_key_hist].GetYaxis().SetRangeUser(0,40000)
            trg_hist[full_key_hist].GetXaxis().SetTitle('luminosity, pb^{-1}')
            trg_hist[full_key_hist].Draw('ep')
            legend.AddEntry(trg_hist[full_key_hist],key_path,"lp")
            legend.Draw()
            cSigEff_Sig[key_sample].SaveAs('EventsPerLumi_'+key_sample+'_'+key_path+'.pdf')

        # # legend = TLegend(.64,.49,.99,.95) 
        # # gStyle.SetOptStat(0)
        # # gStyle.SetOptTitle(0)
        # # i = -7
        # # sig_eff_mgr[key_hist+key_sample] = TMultiGraph()
        # for key_path in path:
        #     print "path = ",key_path
        #     i = i+1
        #     #myfile_denom = TFile(path['No_ElecID']+samplelist[key_sample])
        #     myfile_denom = TFile('v07/'+samplelist[key_sample])
        #     myfile_eff = TFile(path[key_path]+samplelist[key_sample])
        #     key_hists_full = key_sample+'_'+key_hist+'_'+key_path
        #     print "key_hists_full",key_hists_full
        #     sig_denom[key_hists_full] = myfile_denom.Get(read_hist[key_hist]).Clone()
        #     sig_eleID[key_hists_full] = myfile_eff.Get(read_hist[key_hist]).Clone()
        #     sig_eff[key_hists_full] = TGraphAsymmErrors()
        #     sig_eff[key_hists_full].Divide(sig_eleID[key_hists_full],sig_denom[key_hists_full],"cl=0.95 b(1,1) mode")
        #     sig_eff[key_hists_full].SetName(key_hists_full)
        #     sig_eff[key_hists_full].SetMarkerStyle(27+i)
        #     sig_eff[key_hists_full].SetMarkerSize(1.4)
        #     sig_eff[key_hists_full].SetMarkerColor(kRed+i)
        #     sig_eff[key_hists_full].GetXaxis().SetTitle(name_hist[key_hist])
        #     sig_eff[key_hists_full].GetYaxis().SetTitle("#varepsilon_{SIG}")
        #     sig_denom[key_hists_full].Print()
        #     eff[key_hists_full] = sig_eleID[key_hists_full].GetEntries()/ sig_denom[key_hists_full].GetEntries()
        #     #fitres = sig_eff[key_hists_full].Fit('pol1','S0')
        #     #eff[key_hists_full] = fitres.Value(0)
        #     #eff_err[key_hists_full] = fitres.Error(0)
        #     sig_eff_mgr[key_hist+key_sample].Add(sig_eff[key_hists_full])
        #     eff_str = "%.2f " % eff[key_hists_full]
        #     #eff_err_str = "%.2f " % eff_err[key_hists_full]
        #     #legend.AddEntry(sig_eff[key_hists_full],key_path+", #bar{#varepsilon} = "+eff_str+" #pm "+eff_err_str,"lp")
        #     legend.AddEntry(sig_eff[key_hists_full],key_path+", #bar{#varepsilon} = "+eff_str,"lp")
        # sig_eff_mgr[key_hist+key_sample].Draw('ap')
        # #sig_eff_mgr[key_hist+key_sample].GetYaxis().SetRangeUser(0.0,1.2)
        # sig_eff_mgr[key_hist+key_sample].GetYaxis().SetRangeUser(0.0,0.5)
        # sig_eff_mgr[key_hist+key_sample].GetYaxis().SetTitle("#varepsilon")
        # sig_eff_mgr[key_hist+key_sample].GetXaxis().SetTitle(name_hist[key_hist])
        # legend.Draw()
        # #cSigEff_Sig[key_hist+key_sample].SaveAs('TwoDcut_Eff_'+key_hist+'_'+key_sample+'.root')
        # #cSigEff_Sig[key_hist+key_sample].SaveAs('/afs/desy.de/user/k/karavdia/www/Zprime_plots/ElecIDeff/TwoDcut_Eff_'+key_hist+'_'+key_sample+'.pdf')
        # cSigEff_Sig[key_hist+key_sample].SaveAs('EleID_Eff_'+key_hist+'_'+key_sample+'.root')
        # cSigEff_Sig[key_hist+key_sample].SaveAs('/afs/desy.de/user/k/karavdia/www/Zprime_plots/ElecIDeff/EleID_Eff_'+key_hist+'_'+key_sample+'.pdf')
