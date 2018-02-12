#include <memory>
#include <stdlib.h>

#include "UHH2/core/include/Utils.h"
#include "UHH2/core/include/Event.h"
#include "UHH2/core/include/EventHelper.h"
#include "UHH2/core/include/Jet.h"
#include "UHH2/core/include/AnalysisModule.h"

#include <UHH2/common/include/MCWeight.h>
#include <UHH2/common/include/JetCorrections.h>
#include <UHH2/common/include/LumiSelection.h> //includes also LuminosityHists.h
#include <UHH2/common/include/TriggerSelection.h>
#include "UHH2/common/include/CleaningModules.h"
#include <UHH2/common/include/NSelections.h> 
#include <UHH2/common/include/MuonIds.h> 
#include <UHH2/common/include/ElectronIds.h>
#include "UHH2/common/include/PrintingModules.h"

#include "UHH2/BaconJets/include/LumiHists.h"

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"


using namespace uhh2;
using namespace std;

    
bool operator<(const run_lumi & rl1, const run_lumi & rl2){
    if(rl1.run == rl2.run){
        return rl1.lumiblock < rl2.lumiblock;
    }
    else{
        return rl1.run < rl2.run;
    }
}


LumiHists::LumiHists(uhh2::Context & ctx,
                                const std::string & dirname,
				const std::string & triggername,
				bool do_inst_lumi_hist) :
    Hists(ctx, dirname),
    triggername_(triggername),
    do_inst_lumi_hist_(do_inst_lumi_hist)
{
    lumi_per_bin = string2double(ctx.get("lumihists_lumi_per_bin", "50.0"));
    if(lumi_per_bin <= 0.0) {
        throw runtime_error("lumihists_lumi_per_bin is <= 0.0; this is not allowed");
    }
    
    



    string lumifile = ctx.get("lumi_file");
    std::unique_ptr<TFile> file(TFile::Open(lumifile.c_str(), "read"));
    TTree * tree = dynamic_cast<TTree*>(file->Get("AnalysisTree"));
    if(!tree){
        throw runtime_error("LuminosityHists: Did not find TTree 'AnalysisTree' in file ;" + lumifile + "'");
    }
    // only fetch branches we really need:
    TBranch * brun = tree->GetBranch("run");
    TBranch * blumiblock = tree->GetBranch("luminosityBlock");
    TBranch * bilumi = tree->GetBranch("intgRecLumi");
    run_lumi rl;
    double ilumi;
    brun->SetAddress(&rl.run);
    blumiblock->SetAddress(&rl.lumiblock);
    bilumi->SetAddress(&ilumi);
    
    // use the file to build a map from run/lumi --> integrated lumi in pb.
    // Inserting into a map sorts by run and lumi.

    double total_lumi = 0.0; // in   1/pb
    double maxinstlumi = 0.0;
    auto ientries = tree->GetEntries();
    
    for(auto ientry = 0l; ientry < ientries; ientry++){
        for(auto b : {brun, blumiblock, bilumi}){
            b->GetEntry(ientry);
        }
        double ilumi_pb = ilumi * 1e-6; // convert units in file (microbarn) to pb.
	if(ilumi_pb > maxinstlumi) maxinstlumi = ilumi_pb;
        total_lumi += ilumi_pb;
        auto it_inserted = rl2lumi.insert(make_pair(rl, ilumi_pb));
        if(!it_inserted.second){
            throw runtime_error("Duplicate run/lumi entry in lumi file '" + lumifile + "'");
        }

    }
    //cout << "LuminosityHists: read total lumi " << total_lumi << " from lumi file " << lumifile << endl;
    
    // Save the bin borders to find out the number of bins to use and for later assigning each event to a bin.
    int nbins_estimated = int(total_lumi / lumi_per_bin + 1);
    if(nbins_estimated >= 20000){
        throw runtime_error("LuminosityHists misconfiguration: would take more than 20000 bins. Please increase lumi_per_bin");
    }
    upper_binborders.reserve(nbins_estimated);

    double ilumi_current_bin = 0.0;
    for(const auto & rl : rl2lumi){
       ilumi_current_bin += rl.second;
       if(ilumi_current_bin >= lumi_per_bin){
          upper_binborders.push_back(rl.first);
            ilumi_current_bin = ilumi_current_bin - lumi_per_bin;
        }
    }
    int nbins = upper_binborders.size() + 1; // add one for the partial bin

  TString name1 = "hist_data_A_";
  TString name2 = "hist_data_B_";
  for(int i=0;i<n_eta_full-1;i++){
    for(int j=0;j<n_pt-1;j++){
     TString name = name1; name+="eta_"+eta_range_full[i]+"_"+eta_range_full[i+1]+"_pT_"+pt_range[j]+"_"+pt_range[j+1];
     hAsymLumi[i][j] = book<TH2D>(name, "Asymmetry per Lumi", nbins,0,(int(total_lumi / lumi_per_bin) + 1)*lumi_per_bin,100,-1.2,1.2);
     name = name2; name+="eta_"+eta_range_full[i]+"_"+eta_range_full[i+1]+"_pT_"+pt_range[j]+"_"+pt_range[j+1];
     hBsymLumi[i][j] = book<TH2D>(name, "Bsymmetry per Lumi", nbins,0,(int(total_lumi / lumi_per_bin) + 1)*lumi_per_bin,100,-1.2,1.2);
  }
  }
    
}
    
void LumiHists::fill(const uhh2::Event & ev){
    if(!ev.isRealData) return;
    

    bool trigger_accepted = true;
    if (trigger_accepted) {
        run_lumi rl{ev.run, ev.luminosityBlock};
        auto it = upper_bound(upper_binborders.begin(), upper_binborders.end(), rl);
        int ibin = distance(upper_binborders.begin(), it); // can be upper_bounds.size() at most, which is nbins and thus Ok.

 //Calculate pt_ave
   sort_by_pt<Jet>(*ev.jets);
    Jet* jet1 = &ev.jets->at(0);// leading jet
    Jet* jet2 = &ev.jets->at(1);// sub-leading jet
    float jet1_pt = jet1->pt(); float jet2_pt = jet2->pt();

//###############################  Declare Probe and Barrel Jet  ###########################################

    Jet* jet_probe = jet1; Jet* jet_barrel = jet2;
    if ((fabs(jet1->eta())<1.3)&&(fabs(jet2->eta())<1.3)) {
      int ran = rand();
      int numb = ran % 2 + 1;
      if(numb==1){
	jet_probe = jet2;
	jet_barrel = jet1;
      }
      if(numb==2){
	jet_probe = jet1;
	jet_barrel = jet2;
      }
    } 
    else if ((fabs(jet1->eta())<1.3)||(fabs(jet2->eta())<1.3)){
      if(fabs(jet1->eta())<1.3){
	jet_probe = jet2;
	jet_barrel = jet1;
      }
      else{
	jet_probe = jet1;
	jet_barrel = jet2;
      }
    }
//##########################################################################################################
 
    float  probejet_pt = jet_probe->pt(); 
    float  barreljet_pt = jet_barrel->pt(); 
    float probejet_eta = jet_probe->eta(); 
    float  probejet_phi = jet_probe->phi(); 
     float barreljet_eta = jet_barrel->eta(); 
    float  barreljet_phi = jet_barrel->phi(); 
    float pt_ave = (jet1_pt + jet2_pt)/2.;
    float jet3_pt = 0; float jet3_ptRaw = 0; 
    if(ev.jets->size()>2){
      Jet* jet3 = &ev.jets->at(2);
      jet3_pt = jet3->pt();
     }
   float alpha = jet3_pt/pt_ave;
    
    float asymmetry = (probejet_pt - barreljet_pt)/(probejet_pt + barreljet_pt);
    TVector2 pt, met;
    met.Set(ev.met->pt() * cos(ev.met->phi()),ev.met->pt() * sin(ev.met->phi()));
    pt.Set(barreljet_pt * cos(barreljet_phi),barreljet_pt* sin(barreljet_phi));
    float mpf_r = 1 + (met.Px()*pt.Px() + met.Py()*pt.Py())/(pt.Px()*pt.Px() + pt.Py()*pt.Py());
    float B = (met.Px()*pt.Px() + met.Py()*pt.Py())/((probejet_pt + barreljet_pt) * sqrt(pt.Px()*pt.Px() + pt.Py()*pt.Py())); //vec_MET*vec_ptbarr/(2ptave*ptbarr)

  
    for(int j=0; j<n_eta_full-1; j++){
      if(alpha>0.3) continue;
       if(probejet_eta > eta_bins_full[j+1] || probejet_eta < eta_bins_full[j]) continue;
       for(int i=0; i<n_pt-1; i++){
	 if(pt_ave>pt_bins[i+1] || pt_ave<pt_bins[i]) continue;
    hAsymLumi[j][i]->Fill(ibin*lumi_per_bin,asymmetry,ev.weight);
    hBsymLumi[j][i]->Fill(ibin*lumi_per_bin, B, ev.weight);
    }
    }
     }

}