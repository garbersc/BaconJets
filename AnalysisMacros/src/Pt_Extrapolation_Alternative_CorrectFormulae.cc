// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// with this script pT extrapolations in bins of eta will be calculated
//
// kFSR is aproximated by function and kFSR correction is done by the 
// function values at each eta bin
//
// number of root files with historgrams and txt files for JEC db
// are produced
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#include "../include/parameters.h"
#include "../include/useful_functions.h"
#include "../include/CorrectionObject.h"

#include <TStyle.h>
#include <TF1.h>
#include <TH1.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TLine.h>
#include <TCanvas.h>
#include <TMatrixDSym.h>
#include <TPaveStats.h>
#include <TFitResult.h>
#include <TLegend.h>
#include <TLatex.h>
#include <TVirtualFitter.h>
#include <TMath.h>
#include <TFile.h>
#include <TProfile.h>


using namespace std;

void CorrectionObject::Pt_Extrapolation_Alternative_CorrectFormulae(bool mpfMethod){
  cout << "--------------- Starting Pt_Extrapolation() ---------------" << endl << endl;
  TStyle* m_gStyle = new TStyle();
  m_gStyle->SetOptFit(000);

  // fill the histos for pt average in bins of eta
  TH1D* ptave_data[n_eta-1];
  int countPt = 0;
  TString namehist = "ptave_";
  for(int i=0; i<n_eta-1; i++){
    TString selection = "alpha<";
    selection+=alpha_cut;
    selection+=" && fabs(probejet_eta)<";
    selection+=eta_range[i+1];
    selection+=" && fabs(probejet_eta)>=";
    selection+=eta_range[i];
    TString var1 = "pt_ave";
    ptave_data[i] = (TH1D*)GetHist(CorrectionObject::_DATAFile, selection, var1, 300,0,3000)->Clone();
    TString namecur = namehist;
    namecur += countPt;
    ptave_data[i]->SetName(namecur);
    countPt++;
  }


  //Set up histos for ratios of responses
  double ratio_al_rel_r[n_pt-1][n_eta-1]; //ratio at pt,eta,alpha bins
  double err_ratio_al_rel_r[n_pt-1][n_eta-1]; //error of ratio at pt,eta,alpha bins
  double ratio_al_mpf_r[n_pt-1][n_eta-1]; //ratio at pt,eta,alpha bins
  double err_ratio_al_mpf_r[n_pt-1][n_eta-1]; //error of ratio at pt,eta,alpha bins
  TProfile *pr_data_asymmetry[n_eta-1];// pT-balance response for data  
  TProfile *pr_data_B[n_eta-1];//MPF response for data
  TProfile *pr_mc_asymmetry[n_eta-1];// pT-balanse responce for MC  
  TProfile *pr_mc_B[n_eta-1];//MPF response for MC
  TH2D *hdata_asymmetry[n_eta-1];
  TH2D *hdata_B[n_eta-1];
  TH2D *hmc_asymmetry[n_eta-1];
  TH2D *hmc_B[n_eta-1];
  int n_entries_mc[n_eta-1][n_pt-1];
  int n_entries_data[n_eta-1][n_pt-1];
  TH1D *hdata_ptave[n_pt-1][n_eta-1];//pt-ave in each bin of pT_ave in bins of eta
  
  int count = 0;
  TString name1 = "hist_data_asymmetry_";
  TString name2 = "hist_data_B_";
  TString name3 = "hist_mc_asymmetry_";
  TString name4 = "hist_mc_B_";
  TString name5 = "hist_data_pt_ave";

  for(int j=0; j<n_eta-1; j++){
    TString eta_name = "eta_"+eta_range2[j]+"_"+eta_range2[j+1];
    
    TString name = name1 + eta_name;
    hdata_asymmetry[j] = new TH2D(name,"",n_pt-1,pt_bins,nResponseBins, -1.2, 1.2);
    name = name2 + eta_name;
    hdata_B[j] = new TH2D(name,"",n_pt-1,pt_bins,nResponseBins, -1.2, 1.2);
    name = name3 + eta_name;
    hmc_asymmetry[j] = new TH2D(name,"",n_pt-1,pt_bins,nResponseBins, -1.2, 1.2);
    name = name4 + eta_name;
    hmc_B[j] = new TH2D(name,"",n_pt-1,pt_bins,nResponseBins, -1.2, 1.2);
 
    /*
    TString name = name1 + eta_name;
    hdata_asymmetry[j] = new TH2D(name,"",n_pt-1,pt_bins,nResponseBins, -2.5, 0);
    name = name2 + eta_name;
    hdata_B[j] = new TH2D(name,"",n_pt-1,pt_bins,nResponseBins, -2.5, 0);
    name = name3 + eta_name;
    hmc_asymmetry[j] = new TH2D(name,"",n_pt-1,pt_bins,nResponseBins, -2.5, 0);
    name = name4 + eta_name;
    hmc_B[j] = new TH2D(name,"",n_pt-1,pt_bins,nResponseBins, -2.5, 0);
    
    TString name = name1 + eta_name;
    hdata_asymmetry[j] = new TH2D(name,"",n_pt-1,pt_bins,nResponseBins, 0, 2.5);
    name = name2 + eta_name;
    hdata_B[j] = new TH2D(name,"",n_pt-1,pt_bins,nResponseBins, 0, 2.5);
    name = name3 + eta_name;
    hmc_asymmetry[j] = new TH2D(name,"",n_pt-1,pt_bins,nResponseBins, 0, 2.5);
    name = name4 + eta_name;
    hmc_B[j] = new TH2D(name,"",n_pt-1,pt_bins,nResponseBins, 0, 2.5);
    */
    
    for(int k=0; k<n_pt-1; k++){
      TString pt_name = "pt_"+pt_range[k]+"_"+pt_range[k+1];
      name = name5 + eta_name + "_" + pt_name; 
      hdata_ptave[k][j] = new TH1D(name,"",3000,0,3000); //only used for GetMean and GetStdDev in the pT extrapolations
      count++;
      ratio_al_rel_r[k][j] = 0;
      err_ratio_al_rel_r[k][j] = 0;
      ratio_al_mpf_r[k][j] = 0;
      err_ratio_al_mpf_r[k][j] = 0;
      n_entries_mc[j][k] = 0;
      n_entries_data[j][k] = 0;
    }
  }  




  //Get relevant information from DATA, loop over DATA events
  TTreeReader myReader_DATA("AnalysisTree", CorrectionObject::_DATAFile);
  TTreeReaderValue<Float_t> pt_ave_data(myReader_DATA, "pt_ave");
  TTreeReaderValue<Float_t> probejet_eta_data(myReader_DATA, "probejet_eta");
  TTreeReaderValue<Float_t> alpha_data(myReader_DATA, "alpha");
  TTreeReaderValue<Float_t> rel_r_data(myReader_DATA, "rel_r");
  TTreeReaderValue<Float_t> mpf_r_data(myReader_DATA, "mpf_r");
  TTreeReaderValue<Float_t> asymmetry_data(myReader_DATA, "asymmetry");
  TTreeReaderValue<Float_t> B_data(myReader_DATA, "B");   
  TTreeReaderValue<Float_t> weight_data(myReader_DATA, "weight");
   
  while (myReader_DATA.Next()) {
    if(*alpha_data>alpha_cut) continue;
    for(int j=0; j<n_eta-1; j++){
      if(fabs(*probejet_eta_data)>eta_bins[j+1] || fabs(*probejet_eta_data)<eta_bins[j]) continue;
      else{
	hdata_asymmetry[j]->Fill(*pt_ave_data,*asymmetry_data,*weight_data);
	hdata_B[j]->Fill(*pt_ave_data,*B_data,*weight_data);

	for(int k=0; k<n_pt-1; k++){
	  if(*pt_ave_data<pt_bins[k] || *pt_ave_data>pt_bins[k+1]) continue;
	  hdata_ptave[k][j]->Fill(*pt_ave_data,*weight_data);
	  n_entries_data[j][k]++;
	}
      }
    }
  }
  


  //Get relevant information from MC, loop over MC events 
  TTreeReader myReader_MC("AnalysisTree", CorrectionObject::_MCFile);
  TTreeReaderValue<Float_t> pt_ave_mc(myReader_MC, "pt_ave");
  TTreeReaderValue<Float_t> probejet_eta_mc(myReader_MC, "probejet_eta");
  TTreeReaderValue<Float_t> alpha_mc(myReader_MC, "alpha");
  TTreeReaderValue<Float_t> rel_r_mc(myReader_MC, "rel_r");
  TTreeReaderValue<Float_t> mpf_r_mc(myReader_MC, "mpf_r");
  TTreeReaderValue<Float_t> asymmetry_mc(myReader_MC, "asymmetry");
  TTreeReaderValue<Float_t> B_mc(myReader_MC, "B");
  TTreeReaderValue<Float_t> weight_mc(myReader_MC, "weight");
  while (myReader_MC.Next()) {
    if(*alpha_mc>alpha_cut) continue;
    for(int j=0; j<n_eta-1; j++){
      if(fabs(*probejet_eta_mc)>eta_bins[j+1] || fabs(*probejet_eta_mc)<eta_bins[j]) continue;
      else{
	hmc_asymmetry[j]->Fill(*pt_ave_mc,*asymmetry_mc,*weight_mc);
	hmc_B[j]->Fill(*pt_ave_mc,*B_mc,*weight_mc);
	for(int k=0; k<n_pt-1; k++){
	  if(*pt_ave_mc<pt_bins[k] || *pt_ave_mc>pt_bins[k+1]) continue;
	  n_entries_mc[j][k]++;
	}
      }
    }
  }

  bool enough_entries[n_eta-1][n_pt-1];
  for(int j=0; j<n_eta-1; j++){
    for(int k=0; k<n_pt-1; k++){
      enough_entries[j][k] = false;
      if(n_entries_mc[j][k] > 100 && n_entries_data[j][k] > 100) enough_entries[j][k] = true;
    }
  }


  //build profiles out of asymmetry and B 2d-histos to get <A> and <B> as a function of pT in bins of eta,alpha
  for(int j=0; j<n_eta-1; j++){
    //build profiles
    pr_data_asymmetry[j] = (TProfile*)hdata_asymmetry[j]->ProfileX();
    pr_data_B[j]         = (TProfile*)hdata_B[j]->ProfileX();
    pr_mc_asymmetry[j]   = (TProfile*)hmc_asymmetry[j]->ProfileX();
    pr_mc_B[j]           = (TProfile*)hmc_B[j]->ProfileX();
  }


  //calculate response from <A> and <B> in bins of pt,eta,alpha
  //gaussian error propagation from errors on <A> and <B>
  for(int k=0; k<n_pt-1; k++){
    for(int j=0; j<n_eta-1; j++){
      //responses for data, MC separately. Only for bins with >= 100 entries
      double mpf_mc = (1+pr_mc_B[j]->GetBinContent(k+1))/(1-pr_mc_B[j]->GetBinContent(k+1));
      //if(pr_mc_B[j]->GetBinEntries(k+1) < 100) mpf_mc = 0;
      if(!enough_entries[j][k] || pr_mc_B[j]->GetBinContent(k+1)==0) mpf_mc = 0;
      double mpf_data = (1+pr_data_B[j]->GetBinContent(k+1))/(1-pr_data_B[j]->GetBinContent(k+1));
      //      if (j==10 || j==14) std::cout<<"@"<<eta_range[j]<<" mpf_mc = "<<mpf_mc<<" mpf_data = "<<mpf_data<<" B_MC = "<<pr_mc_B[j]->GetBinContent(k+1)<<" B_DATA = "<<pr_data_B[j]->GetBinContent(k+1)<<std::endl;
      //if(pr_data_B[j]->GetBinEntries(k+1) < 100) mpf_data = 0;
      if(!enough_entries[j][k] || pr_data_B[j]->GetBinContent(k+1)==0) mpf_data = 0;
      double rel_mc = (1+pr_mc_asymmetry[j]->GetBinContent(k+1))/(1-pr_mc_asymmetry[j]->GetBinContent(k+1));
      //if(pr_mc_asymmetry[j]->GetBinEntries(k+1) < 100) rel_mc = 0;
      if(!enough_entries[j][k] || pr_mc_asymmetry[j]->GetBinContent(k+1)==0) rel_mc = 0;
      double rel_data = (1+pr_data_asymmetry[j]->GetBinContent(k+1))/(1-pr_data_asymmetry[j]->GetBinContent(k+1));
      //if(pr_data_asymmetry[j]->GetBinEntries(k+1) < 100) rel_data = 0;
      if(!enough_entries[j][k] || pr_data_asymmetry[j]->GetBinContent(k+1)==0) rel_data = 0;
      double err_mpf_mc = 2/(pow((1-pr_mc_B[j]->GetBinContent(k+1)),2)) * pr_mc_B[j]->GetBinError(k+1);
      //if(pr_mc_B[j]->GetBinEntries(k+1) < 100) err_mpf_mc = 0;
      if(!enough_entries[j][k] || pr_mc_B[j]->GetBinContent(k+1)==0) err_mpf_mc = 0;
      double err_mpf_data = 2/(pow((1-pr_data_B[j]->GetBinContent(k+1)),2)) * pr_data_B[j]->GetBinError(k+1);
      //if(pr_data_B[j]->GetBinEntries(k+1) < 100) err_mpf_data = 0;
      if(!enough_entries[j][k] || pr_data_B[j]->GetBinContent(k+1)==0) err_mpf_data = 0;
      double err_rel_mc = 2/(pow((1-pr_mc_asymmetry[j]->GetBinContent(k+1)),2)) * pr_mc_asymmetry[j]->GetBinError(k+1);
      //if(pr_mc_asymmetry[j]->GetBinEntries(k+1) < 100) err_rel_mc = 0;
      if(!enough_entries[j][k] || pr_mc_asymmetry[j]->GetBinContent(k+1)==0) err_rel_mc = 0;
      double err_rel_data = 2/(pow((1-pr_data_asymmetry[j]->GetBinContent(k+1)),2)) * pr_data_asymmetry[j]->GetBinError(k+1);
      //if(pr_data_asymmetry[j]->GetBinEntries(k+1) < 100) err_rel_data = 0;
      if(!enough_entries[j][k] || pr_data_asymmetry[j]->GetBinContent(k+1)==0) err_rel_data = 0;

      //ratio of responses, again gaussian error propagation
      if(rel_data > 0) ratio_al_rel_r[k][j] = rel_mc/rel_data;
      else ratio_al_rel_r[k][j] = 0;
      err_ratio_al_rel_r[k][j] = sqrt(pow(1/rel_data*err_rel_mc,2) + pow(rel_mc/(rel_data*rel_data)*err_rel_data,2));
      if(mpf_data > 0) ratio_al_mpf_r[k][j] = mpf_mc/mpf_data;
      else ratio_al_mpf_r[k][j] = 0;
      err_ratio_al_mpf_r[k][j] = sqrt(pow(1/mpf_data*err_mpf_mc,2) + pow(mpf_mc/(mpf_data*mpf_data)*err_mpf_data,2));
    }
  }

  /*
  //normalize each bin of pt_ave to the average in that bin in |eta|<1.3
  for(int i=0; i<n_pt-1; i++){
    double norm_rel = 0;
    double norm_mpf = 0;
    for(int j=0; j<n_etabarr; j++){
      norm_rel += ratio_al_rel_r[i][j];
      norm_mpf += ratio_al_mpf_r[i][j];
      //cout << "Alpha bin: " << k << ", pt bin: " << i << ", eta-bin: " << j << endl;
      //cout << "before normalization: MPF = " << ratio_al_mpf_r[i][j][k] << ", pt-balance: " << ratio_al_rel_r[i][j][k] << endl;
    }
    norm_mpf = norm_mpf/n_etabarr;
    norm_rel = norm_rel/n_etabarr;
    cout << "normalization factor MPF: " << norm_mpf << ", pt-balance: " << norm_rel << endl;

    for(int j=0; j<n_eta-1; j++){
      //normalize each eta-bin to integral of first 5 eta-bins	 
      if(norm_rel != 0 && norm_mpf != 0){
	ratio_al_rel_r[i][j] = ratio_al_rel_r[i][j]/norm_rel;
	ratio_al_mpf_r[i][j] =  ratio_al_mpf_r[i][j]/norm_mpf;
	//cout << "after normalization: MPF = " << ratio_al_mpf_r[i][j][k] << ", pt-balance: " << ratio_al_rel_r[i][j][k] << endl;
      }
    }
  }
  */
   
  // get ratio for MC to DATA responses
  //BAD NOMENCLATURE
  double ratio_mpf[n_eta-1][n_pt-1];     //ratio at pt,eta bins for alpha = 0.3
  double err_ratio_mpf[n_eta-1][n_pt-1]; //error of ratio at pt,eta bins for alpha = 0.3
  for(int j=0; j<n_eta-1; j++){
    for(int k=0; k<n_pt-1; k++){
      if(mpfMethod){
	ratio_mpf[j][k] = ratio_al_mpf_r[k][j];
	err_ratio_mpf[j][k] = err_ratio_al_mpf_r[k][j];
	//	std::cout<<"ratio_mpf[j][k] = @"<<eta_range[j]<<" "<<ratio_mpf[j][k]<<std::endl;
      }
      else{
	ratio_mpf[j][k] = ratio_al_rel_r[k][j];
	err_ratio_mpf[j][k] = err_ratio_al_rel_r[k][j];
      }
    }
  }


  //Create and fill TGraphErrors
  // double xbin_tgraph[n_pt-1];
  // double zero[n_pt-1];
  double xbin_tgraph[n_eta-1][n_pt-1];
  double zero[n_eta-1][n_pt-1];
  for(int i=0;i<n_pt-1;i++){
    for(int j=0; j<n_eta-1; j++){
      // xbin_tgraph[i]=(pt_bins[j]+pt_bins[j+1])/2;
      // zero[i]=(pt_bins[j+1]-pt_bins[j])/2 ;
      xbin_tgraph[j][i] = hdata_ptave[i][j]->GetMean();
      zero[j][i] = hdata_ptave[i][j]->GetStdDev();
      // cout << "x before: " << (pt_bins[i]+pt_bins[i+1])/2 << " +- " << (pt_bins[i+1]-pt_bins[i])/2 << endl;
      // cout << "x now:    " << xbin_tgraph[j][i] << " +- " << zero[j][i] << endl << endl;
    }
  }
  TGraphErrors *graph1_mpf[n_eta-1];
 
  bool graph_filled[n_eta-1];
  for(int j=0; j<n_eta-1; j++) graph_filled[j] = false;
  for(int j=0; j<n_eta-1; j++){
    graph1_mpf[j] = new TGraphErrors(n_pt-1, xbin_tgraph[j], ratio_mpf[j], zero[j], err_ratio_mpf[j]);
    graph1_mpf[j] = (TGraphErrors*)CleanEmptyPoints(graph1_mpf[j]); 
    if(graph1_mpf[j]->GetN() > 0) graph_filled[j] = true;
    // cout << "Eta bin no: " << j << ", graph filled?: " << graph_filled[j] << endl;
  }

  for(int k=0; k<n_pt-1; k++){
    double x = 0;
    double val = 0;
    graph1_mpf[13]->GetPoint(k,x,val);
    // cout << "BEFORE: In eta bin no 13: ratio of responses in pT bin no " << k << " at " << x << " : " << val << endl; 
  }

  //Mikko's request: delete super-high data point in 2.8-2.9 bin (j==13) above ~400 GeV pT (point no 6 & 7 [c++])
// <<<<<<< HEAD
//   if(graph1_mpf[13]->GetN() == 10){
//     graph1_mpf[13]->RemovePoint(9);
//     graph1_mpf[13]->RemovePoint(8);
//     graph1_mpf[13]->RemovePoint(7);
//     if(CorrectionObject::_runnr == "H") graph1_mpf[13]->RemovePoint(6);
//   }
//   else if(graph1_mpf[13]->GetN() == 9){
//     graph1_mpf[13]->RemovePoint(8);
//     graph1_mpf[13]->RemovePoint(7);
//     graph1_mpf[13]->RemovePoint(6);
//     if(CorrectionObject::_runnr == "H") graph1_mpf[13]->RemovePoint(5);
//   }
//   else if(graph1_mpf[13]->GetN() == 8){
//     graph1_mpf[13]->RemovePoint(7);
//     graph1_mpf[13]->RemovePoint(6);
//     graph1_mpf[13]->RemovePoint(5);
//     if(CorrectionObject::_runnr == "H") graph1_mpf[13]->RemovePoint(4);
//   }
/*
 if(graph1_mpf[13]->GetN() == 7){
    graph1_mpf[13]->RemovePoint(6);
    graph1_mpf[13]->RemovePoint(5);
     graph1_mpf[13]->RemovePoint(4);
     //  if(CorrectionObject::_runnr == "H") graph1_mpf[13]->RemovePoint(3);
  }
 else if(graph1_mpf[13]->GetN() == 6){
    graph1_mpf[13]->RemovePoint(5);
     graph1_mpf[13]->RemovePoint(4);
     graph1_mpf[13]->RemovePoint(4);
     // if(CorrectionObject::_runnr == "H") graph1_mpf[13]->RemovePoint(3);
  }
*/

// =======

  // graph1_mpf[13]->RemovePoint(8);
  // graph1_mpf[13]->RemovePoint(7);
  // if(CorrectionObject::_runnr == "H")  graph1_mpf[13]->RemovePoint(7);


  for(int k=0; k<n_pt-1; k++){
    double x = 0;
    double val = 0;
    graph1_mpf[13]->GetPoint(k,x,val);
    // cout << "AFTER: In eta bin no 13: ratio of responses in pT bin no " << k << " at " << x << " : " << val << endl; 
  }


  // create horizontal line for plotting ("ideal value")
  TLine *line = new TLine(0.,1,pt_bins[n_pt-1]+10,1);

  // create a function for the loglinear fit
  TF1 * f1[n_eta-1];
  // create a function for the constant fit
  TF1 * f2[n_eta-1];



  // create output .dat file, including the kFSR extrapolation (alpha->0)
  FILE *fp, *fp2, *l2resfile;
  if(mpfMethod){
    fp = fopen(CorrectionObject::_outpath+"output/pT_MPF_"+CorrectionObject::_generator_tag+"_extrapolations.dat","w");
    fp2 = fopen(CorrectionObject::_outpath+"output/pT_MPF_"+CorrectionObject::_generator_tag+"_constantExtrapolation.dat","w");
    l2resfile = fopen(CorrectionObject::_outpath+"output/L2Res_MPF_"+CorrectionObject::_generator_tag+".dat","w");
  }
  else{
    fp = fopen(CorrectionObject::_outpath+"output/pT_DiJet_"+CorrectionObject::_generator_tag+"_extrapolations.dat","w");
    fp2 = fopen(CorrectionObject::_outpath+"output/pT_DiJet_"+CorrectionObject::_generator_tag+"_constantExtrapolation.dat","w");
    l2resfile = fopen(CorrectionObject::_outpath+"output/L2Res_DiJet_"+CorrectionObject::_generator_tag+".dat","w");
  }



  /* +++++++++++++++++++++++ Plots +++++++++++++++++++ */
  TCanvas* asd[n_eta-1];
  TString plotname[n_eta-1];
  double Vcov[3][n_eta-1];//covariance matrix for log lin fit results
  TH1D* h_chi2_loglin = new TH1D("h_chi2_loglin", "Chi2/ndf for each eta bin;|#eta|;#chi^{2}/ndf", n_eta-1, eta_bins);
  TH1D* h_chi2_const =  new TH1D("h_chi2_const", "Chi2/ndf for each eta bin;|#eta|;#chi^{2}/ndf", n_eta-1, eta_bins);
 
  for (int j=0; j<n_eta-1; j++){
    if(mpfMethod){
      plotname[j]="mpf_ptextra_"+CorrectionObject::_generator_tag+"_eta_"+eta_range[j]+"_"+eta_range[j+1];
    }
    else{
      plotname[j]="dijet_ptextra_"+CorrectionObject::_generator_tag+"_eta_"+eta_range[j]+"_"+eta_range[j+1];
    }
    asd[j] = new TCanvas(plotname[j], plotname[j], 800,700);
    m_gStyle->SetOptTitle(0);
    gPad->SetLogx();
    graph1_mpf[j]->SetMarkerColor(kBlue);
    graph1_mpf[j]->SetMarkerStyle(20);
    graph1_mpf[j]->SetLineColor(kBlue);


    f1[j] = new TF1(plotname[j]+"f1","[0]+[1]*TMath::Log(x)", 50 , pt_bins[n_pt-1]+10);
    f1[j]->SetParameters(1,0);
    f2[j] = new TF1(plotname[j]+"f2","pol0", 50 , pt_bins[n_pt-1]+10);
    f2[j]->SetLineColor(kBlue);
    f2[j]->SetLineStyle(3);

    //FOR LEGACY DATA TRY Quadratic Function
    /*
    f3[j] = new TF1(plotname[j]+"f1","[0]+[1]*TMath::Log(x*x)", 50 , pt_bins[n_pt-1]+10);
    f3[j]->SetParameters(1,0);
    */

    if(j==12){ //to help the fit converge
      f1[j]->SetParameter(0,1.32);
      f1[j]->SetParameter(1,-0.04);

      if(CorrectionObject::_collection == "AK8Puppi"){
	f1[j]->SetParameter(0,0.85);
	f1[j]->SetParameter(1,0.11);
	
	if(CorrectionObject::_runnr == "G"){
	  f1[j]->SetParameter(0,1.1765);
	  f1[j]->SetParameter(1,-0.00031);
	}
      }
      if(CorrectionObject::_closuretest){
	f1[j]->SetParameters(0.000385,0.915);
      }
      if(CorrectionObject::_runnr == "BCDEFearly"){
	if(CorrectionObject::_closuretest){
	  f1[j]->SetParameter(0,0.933);      //CLOSURETEST
	  f1[j]->SetParameter(1,0.00037);
	}
      }
    }
    if(j==13){ //to help the fit converge
      f1[j]->SetParameter(0,1.2);
      f1[j]->SetParameter(1,-0.05);
      if(CorrectionObject::_runnr == "FlateGH"){
	f1[j]->SetParameter(0,1.17);
	f1[j]->SetParameter(1,-0.00067);
	if(CorrectionObject::_closuretest){
	  f1[j]->SetParameter(0,0.89);      //CLOSURETEST
	  f1[j]->SetParameter(1,0.00033);
	}
      }
      else if(CorrectionObject::_runnr == "BCD"){
	if(CorrectionObject::_closuretest){
	  f1[j]->SetParameter(0,0.945);      //CLOSURETEST
	  f1[j]->SetParameter(1,0.00035);
	}
      }
      else if(CorrectionObject::_runnr == "EFearly"){
	if(CorrectionObject::_closuretest){
	  f1[j]->SetParameter(0,0.945);      //CLOSURETEST
	  f1[j]->SetParameter(1,0.00035);
	}
      }
      else if(CorrectionObject::_runnr == "FlateG"){
	if(CorrectionObject::_closuretest){
	  f1[j]->SetParameter(0,0.945);      //CLOSURETEST
	  f1[j]->SetParameter(1,0.00035);
	}
      }
      else if(CorrectionObject::_runnr == "H"){
	if(CorrectionObject::_closuretest){
	  f1[j]->SetParameter(0,0.945);      //CLOSURETEST
	  f1[j]->SetParameter(1,0.00035);
	}
      }
      if(CorrectionObject::_runnr == "BCDEFearly"){
	if(CorrectionObject::_closuretest){
	  f1[j]->SetParameter(0,0.933);      //CLOSURETEST
	  f1[j]->SetParameter(1,0.00037);
	}
      }
    }

    if(j==16){
      if(CorrectionObject::_runnr == "BCDEFGH"){
	if(!mpfMethod){
	  f1[j]->SetParameter(0,1.1);
	  f1[j]->SetParameter(1,-0.0005);
	}
      }
    }

    if(j==17){
      if(CorrectionObject::_runnr == "BCD"){
	if(!mpfMethod){ //only for pt bal
	  f1[j]->SetParameter(0,1.19);
	  f1[j]->SetParameter(1,-0.0012);
	}
      }
      else if(CorrectionObject::_runnr == "EFearly"){
	if(!mpfMethod){ //only for pt bal
	  f1[j]->SetParameter(0,1.19);
	  f1[j]->SetParameter(1,-0.0012);
	}
      }
      else if(CorrectionObject::_runnr == "FlateG"){
	if(!mpfMethod){ //only for pt bal
	  f1[j]->SetParameter(0,1.19);
	  f1[j]->SetParameter(1,-0.0012);
	}
      }
      else if(CorrectionObject::_runnr == "H"){
	if(mpfMethod){ //this time only for MPF
	  f1[j]->SetParameter(0,1.19);
	  f1[j]->SetParameter(1,-0.0012);
	}
	else{
	  f1[j]->SetParameter(0,1.32);
	  f1[j]->SetParameter(1,-0.0015);
	}
      }
      else if(CorrectionObject::_runnr == "BCDEFGH"){
	if(mpfMethod){
	  f1[j]->SetParameter(0,1.16);
	  f1[j]->SetParameter(1,-0.0008);
	}
	else{
	  f1[j]->SetParameter(0,1.2);
	  f1[j]->SetParameter(1,-0.001);
	}
      }
    }
    

    //Do the fit!
    if(graph_filled[j]){
      TFitResultPtr fitloglin =  graph1_mpf[j]->Fit(plotname[j]+"f1","SM","",95,1000);
      TMatrixDSym cov = fitloglin->GetCovarianceMatrix();
      Vcov[0][j] = cov(0,0);
      Vcov[1][j] = cov(1,1);     
      Vcov[2][j] = cov(0,1);
    }
    else{
      f1[j]->SetParameters(0.0001,0.0001);
      f1[j]->SetParError(0,0.0001);
      f1[j]->SetParError(1,0.0001);
      Vcov[0][j] = 0.0001;
      Vcov[1][j] = 0.0001;     
      Vcov[2][j] = 0.0001;
    }
    
    if(graph_filled[j]){
      graph1_mpf[j]->Fit(plotname[j]+"f2","SM + SAME","",95,1000);
    }
    else{
      f2[j]->SetParameter(0,0.0001);
      f2[j]->SetParError(0,0.0001);
    }
    /*
    if(graph_filled[j]){
      TFitResultPtr fitlogquad =  graph1_mpf[j]->Fit(plotname[j]+"f3","SM","",95,1000);
      TMatrixDSym cov1 = fitlogquad->GetCovarianceMatrix();
      Vcov[0][j] = cov1(0,0);
      Vcov[1][j] = cov1(1,1);     
      Vcov[2][j] = cov1(0,1);
    }
    else{
      f3[j]->SetParameters(0.0001,0.0001);
      f3[j]->SetParError(0,0.0001);
      f3[j]->SetParError(1,0.0001);
      Vcov[0][j] = 0.0001;
      Vcov[1][j] = 0.0001;     
      Vcov[2][j] = 0.0001;
    }
    */


    graph1_mpf[j]->Draw("AP");
    if(graph_filled[j]){
      graph1_mpf[j]->GetXaxis()->SetTitle("#bar{p}_{T} [GeV]");
      graph1_mpf[j]->GetXaxis()->SetTitleSize(0.05);
      graph1_mpf[j]->GetXaxis()->SetTitleOffset(0.80);
      graph1_mpf[j]->GetXaxis()->SetLimits(30,pt_bins[n_pt-1]+100);
      graph1_mpf[j]->GetYaxis()->SetRangeUser(0.85,1.25);
    }
 
    line->SetLineStyle(2);
    line->Draw("SAME");

    //Store the p0 parameter from const fit
    if (fp2!=NULL) {
      // getting the p0 parameter from the constant fit
      Float_t value = f2[j]->GetParameter(0);
      fprintf(fp2, "%f\n",value);
    }
    if (l2resfile!=NULL) {
      fprintf(l2resfile, "%f\n", eta_bins[j]);
    }


    //Cosmetics
    asd[j]->Modified(); 
    asd[j]->Update();
    
    TPaveStats *st = ((TPaveStats*)(graph1_mpf[j]->GetListOfFunctions()->FindObject("stats")));
    if (st) {
      st->SetTextColor(kRed);
      st->SetX1NDC(0.69); st->SetX2NDC(0.99);
      st->SetY1NDC(0.65); st->SetY2NDC(0.8);
    }
    st = ((TPaveStats*)(graph1_mpf[j]->GetListOfFunctions()->FindObject("stats")));
    if (st) {
      st->SetTextColor(graph1_mpf[j]->GetLineColor());
      st->SetX1NDC(0.69); st->SetX2NDC(0.99);
      st->SetY1NDC(0.85); st->SetY2NDC(0.95);
    }
    asd[j]->Modified(); 
    asd[j]->Update();


    //Set up legend, prepare canvas layout
    TLegend *leg1;
    leg1 = new TLegend(0.12,0.68,0.35,0.88,"","brNDC");//x+0.1
    leg1->SetBorderSize(0);
    leg1->SetTextSize(0.038);
    leg1->SetFillColor(10);
    leg1->SetLineColor(1);
    leg1->SetTextFont(42);
    if(mpfMethod){
      leg1->SetHeader("MPF #bar{p}_{T} extrapolation, "+eta_range[j]+"#leq|#eta|<"+eta_range[j+1]);
    }
    else{
      leg1->SetHeader("p_{T} balance #bar{p}_{T} extrapolation, "+eta_range[j]+"#leq|#eta|<"+eta_range[j+1]);
    }


    TString alVal;
    alVal.Form("%0.2f\n",alpha_cut);
    TString altitle = "{#alpha<"+alVal+"}";
    TString axistitle = "(R^{MC}/R^{data})_";
    axistitle +=altitle;
    leg1->AddEntry(graph1_mpf[j], axistitle,"P");
    leg1->AddEntry(f1[j], "loglinear fit","L");
    leg1->AddEntry(f2[j], "constant fit","L");
    leg1->Draw();
    
    TLatex *tex = new TLatex();
    tex->SetNDC();
    tex->SetTextSize(0.045); 
    tex->DrawLatex(0.58,0.91,CorrectionObject::_lumitag+"(13TeV)"); 

    TLatex *tex2 = new TLatex();
    if(graph_filled[j]){    
      TString chi2_loglin = "loglinear fit #chi^{2}/n.d.f = ";
      chi2_loglin += trunc(f1[j]->GetChisquare());
      chi2_loglin +="/";
      chi2_loglin +=trunc(f1[j]->GetNDF());
      TString chi2_const = "constant fit #chi^{2}/n.d.f = ";
      chi2_const+=trunc(f2[j]->GetChisquare());
      chi2_const+="/";
      chi2_const+=trunc(f2[j]->GetNDF());

      tex2->SetNDC();
      tex2->SetTextSize(0.035); 
      tex2->DrawLatex(0.51,0.73,chi2_loglin);
      tex2->DrawLatex(0.51,0.69,chi2_const);

      double chi2ndf_loglin = f1[j]->GetChisquare() / f1[j]->GetNDF();
      double chi2ndf_const =  f2[j]->GetChisquare() / f2[j]->GetNDF();
      h_chi2_loglin->SetBinContent(j+1,chi2ndf_loglin); //to make sure to fill the right eta-bin...
      h_chi2_const->SetBinContent(j+1,chi2ndf_const);   //to make sure to fill the right eta-bin...
    }

    h_chi2_loglin->GetYaxis()->SetRangeUser(0,20);
    h_chi2_const->GetYaxis()->SetRangeUser(0,20);
    //Store plots
    if(mpfMethod){
      asd[j]->Print(CorrectionObject::_outpath+"plots/pTextrapolation_MPF_"+CorrectionObject::_generator_tag+"_pT_"+eta_range2[j]+"_"+eta_range2[j+1]+".pdf");
    }
    else{
      asd[j]->Print(CorrectionObject::_outpath+"plots/pTextrapolation_Pt_"+CorrectionObject::_generator_tag+"_pT_"+eta_range2[j]+"_"+eta_range2[j+1]+".pdf");
    }
    delete tex2;
    delete tex;
    delete leg1;
  }

  TCanvas* c_chi2_loglin = new TCanvas();
  h_chi2_loglin->Draw("HIST");
  if(mpfMethod)c_chi2_loglin->SaveAs(CorrectionObject::_outpath+"plots/pTextrapolation_MPF_chi2ndf_loglin_"+CorrectionObject::_generator_tag+".pdf");
  else c_chi2_loglin->SaveAs(CorrectionObject::_outpath+"plots/pTextrapolation_Pt_chi2ndf_loglin_"+CorrectionObject::_generator_tag+".pdf");
  delete c_chi2_loglin;
  delete h_chi2_loglin;

  TCanvas* c_chi2_const = new TCanvas();
  h_chi2_const->Draw("HIST");
  if(mpfMethod) c_chi2_const->SaveAs(CorrectionObject::_outpath+"plots/pTextrapolation_MPF_chi2ndf_const_"+CorrectionObject::_generator_tag+".pdf");
  else c_chi2_const->SaveAs(CorrectionObject::_outpath+"plots/pTextrapolation_Pt_chi2ndf_const_"+CorrectionObject::_generator_tag+".pdf");
  delete c_chi2_const;
  delete h_chi2_const;

  fclose(fp);
  fclose(fp2);


  /* ++++++++++++++++++++++++++ Calculate L2Residuals MPF ++++++++++++++++++++++++++++++ */



  // get the kFSR file
  TCanvas* c_kfsr_fit = new TCanvas("c_kfsr_fit", "c_kfsr_fit",1);
  c_kfsr_fit->Update();
  if(mpfMethod){
    TFile* kfsr_mpf;
    kfsr_mpf = new TFile(CorrectionObject::_outpath+"Histo_KFSR_MPF_"+CorrectionObject::_generator_tag+"_L1.root","READ");
    TH1D* hist_kfsr_mpf = (TH1D*)kfsr_mpf->Get("kfsr_mpf");
    //fit the kFSR values
    TF1 *kfsr_fit_mpf = new TF1("kfsr_fit_mpf","[0]+([1]*TMath::CosH(x))/(1+[2]*TMath::CosH(x))",0,5.);   //Range: 0,5. by default

    bool fit_fullrange = false;
    //    bool fit_fullrange = true;
    //VERY fragile fit, carefully set initial values
    if(CorrectionObject::_generator == "pythia"){

      if(CorrectionObject::_collection == "AK4Puppi"){ 
	kfsr_fit_mpf->SetParameters(1,4.,100.);
	if(CorrectionObject::_runnr == "E"){
	  kfsr_fit_mpf->SetParameters(1,-50,200);
	  fit_fullrange = true;
	}
      }
      
      else if(CorrectionObject::_collection == "AK4CHS"){
	if(CorrectionObject::_runnr == "BCD"){
	  //	  kfsr_fit_mpf->SetParameters(1,100,500);
	  kfsr_fit_mpf->SetParameters(1,100,300);      
	  fit_fullrange = true;
	}
	else if(CorrectionObject::_runnr == "E"){
	  if(CorrectionObject::_closuretest) kfsr_fit_mpf->SetParameters(1.,-100,300); //CLOSURETEST
	  else kfsr_fit_mpf->SetParameters(1.,-100,300); //RES
	}
	else if(CorrectionObject::_runnr == "F") {
	  kfsr_fit_mpf->SetParameters(1.002,-0.0005,0.001); 
	} //CLOSURETEST
	else if(CorrectionObject::_runnr == "FlateG"){
	  if(!CorrectionObject::_closuretest) kfsr_fit_mpf->SetParameters(1,100,300); //RES
	}
	else if(CorrectionObject::_runnr == "H"){
	  cout<<"HELLO ANYBODY?"<<endl;
	  if(!CorrectionObject::_closuretest) kfsr_fit_mpf->SetParameters(1,100,300); //RES
	  else kfsr_fit_mpf->SetParameters(1.,150,250); //CLOSURETEST
	  fit_fullrange = true;
	}
      }

      else if(CorrectionObject::_collection == "AK8CHS"){
	if(CorrectionObject::_runnr == "BCD"){
	kfsr_fit_mpf->SetParameters(1.1,-30,300);
	fit_fullrange = true;
      }
     else kfsr_fit_mpf->SetParameters(1.001,0.,0.);
    }
      
      else if(CorrectionObject::_collection == "AK8Puppi"){
	if(CorrectionObject::_runnr == "G"){
	  kfsr_fit_mpf->SetParameters(1,-30,300);
	  fit_fullrange = true;
	}
	else kfsr_fit_mpf->SetParameters(1.001,0.,0.);
      }

      else kfsr_fit_mpf->SetParameters(1.001,0.,0.);
    }
    
    else if(CorrectionObject::_generator == "herwig"){
      if(CorrectionObject::_collection == "AK4CHS") {
	kfsr_fit_mpf->SetParameters(1.,-0.001,0.04);
      }
      else if(CorrectionObject::_collection == "AK4Puppi"){
	if(CorrectionObject::_runnr == "G"){
	  kfsr_fit_mpf->SetParameters(1.,-0.001,0.05);
	}
	if(CorrectionObject::_runnr == "BCD"){
	  kfsr_fit_mpf->SetParameters(1.,-50,200); 
	}
	if(CorrectionObject::_runnr == "F"){
	  kfsr_fit_mpf->SetParameters(1.,-50,200); 
	}
      }
      else if(CorrectionObject::_collection == "AK8CHS"){
	if(CorrectionObject::_runnr == "BCD"){
	  kfsr_fit_mpf->SetParameters(1.,-150,300);
	}
      }
    }
    else if(CorrectionObject::_generator == "madgraph"){
      
      
    }
    else throw runtime_error("PTextrapolation, MPF kFSR-fit: Invalid generator specified.");

    //Finally perform the fit!
    kfsr_fit_mpf->SetLineColor(kRed+1);
    std::cout<<"!!! kFSR MPF fit !!!"<<std::endl;
    if(!fit_fullrange) hist_kfsr_mpf->Fit("kfsr_fit_mpf","SR","",0,3.14);
    //   if(!fit_fullrange) hist_kfsr_mpf->Fit("kfsr_fit_mpf","SR","",0,2.4);
    else hist_kfsr_mpf->Fit("kfsr_fit_mpf","SR","",0,5.);
    

    //Create a histogram to hold the confidence intervals                                                                                        
    TH1D *hist_kfsr_fit_mpf = (TH1D*)hist_kfsr_mpf->Clone();
    (TVirtualFitter::GetFitter())->GetConfidenceIntervals(hist_kfsr_fit_mpf);
    //Now the "hist_kfsr_fit" histogram has the fitted function values as the
    //bin contents and the confidence intervals as bin errors

    hist_kfsr_fit_mpf->SetStats(kFALSE);
    hist_kfsr_fit_mpf->SetFillColor(kRed-10);     
    hist_kfsr_fit_mpf->SetName("hist_kfsr_fit_mpf");
    hist_kfsr_fit_mpf->SetTitle("kfsr fit for mpf");
    double flat[n_eta-1];
    double loglin[n_eta-1];
    for (int j=0; j<n_eta-1; j++){
      flat[j] = hist_kfsr_fit_mpf->GetBinContent(j+1)*f2[j]->GetParameter(0);
      loglin[j] = hist_kfsr_fit_mpf->GetBinContent(j+1)*(f1[j]->GetParameter(0)+f1[j]->GetParameter(1)*TMath::Log(ptave_data[j]->GetMean()) );
    }
    double flat_norm=0, loglin_norm=0;
    for (int j=0; j<n_etabarr; j++){
      flat_norm += flat[j];
      loglin_norm += loglin[j];
    }
    flat_norm = flat_norm/n_etabarr;
    loglin_norm = loglin_norm/n_etabarr;
    for (int j=0; j<n_eta-1; j++){
      cout << "eta bin " << j << " before normalization: " << loglin[j] << endl;
      flat[j] = flat[j]/flat_norm;
      loglin[j] = loglin[j]/loglin_norm;
      cout << "eta bin " << j << " after normalization: " << loglin[j] << endl;
    }

    cout << "normalization factor (flat): " << flat_norm << endl;
    cout << "normalization factor (logl): " << loglin_norm << endl;

    //Histograms to hold the output
    TH1D* Residual_logpt_MPF = new TH1D("res_logpt_mpf","res_logpt_mpf", n_eta-1,eta_bins);
    TH1D* Residual_const_MPF = new TH1D("res_const_mpf","res_const_mpf", n_eta-1,eta_bins);
    TH1D* ptave_const_MPF = new TH1D("ptave_const_mpf","ptave_const_mpf", n_eta-1,eta_bins);
    TH1D* ptave_logpt_MPF = new TH1D("ptave_logpt_mpf","ptave_logpt_mpf", n_eta-1,eta_bins);
    

    ofstream output, output_loglin, uncerts, uncerts_loglin;
    output.open(CorrectionObject::_outpath+"output/Summer16_23Sep2016_MPF_FLAT_L2Residual_"+CorrectionObject::_generator_tag+"_"+CorrectionObject::_jettag+".txt");
    output_loglin.open(CorrectionObject::_outpath+"output/Summer16_23Sep2016_MPF_LOGLIN_L2Residual_"+CorrectionObject::_generator_tag+"_"+CorrectionObject::_jettag+".txt");
    uncerts.open(CorrectionObject::_outpath+"output/Summer16_23Sep2016_MPF_FLAT_L2Residual_"+CorrectionObject::_generator_tag+"_"+CorrectionObject::_jettag+".txt.STAT");
    uncerts_loglin.open(CorrectionObject::_outpath+"output/Summer16_23Sep2016_MPF_LOGLIN_L2Residual_"+CorrectionObject::_generator_tag+"_"+CorrectionObject::_jettag+".txt.STAT");

    output  << "{ 1 JetEta 1 JetPt [2]*([3]*([4]+[5]*TMath::Log(max([0],min([1],x))))*1./([6]+[7]*100./3.*(TMath::Max(0.,1.03091-0.051154*pow(x,-0.154227))-TMath::Max(0.,1.03091-0.051154*TMath::Power(208.,-0.154227)))+[8]*((-2.36997+0.413917*TMath::Log(x))/x-(-2.36997+0.413917*TMath::Log(208))/208))) Correction L2Relative}" << endl;
    output_loglin  << "{ 1 JetEta 1 JetPt [2]*([3]*([4]+[5]*TMath::Log(max([0],min([1],x))))*1./([6]+[7]*100./3.*(TMath::Max(0.,1.03091-0.051154*pow(x,-0.154227))-TMath::Max(0.,1.03091-0.051154*TMath::Power(208.,-0.154227)))+[8]*((-2.36997+0.413917*TMath::Log(x))/x-(-2.36997+0.413917*TMath::Log(208))/208))) Correction L2Relative}" << endl;
    uncerts << "{ 1 JetEta 1 JetPt [0] kFSR_err f0_err}" << endl;
    //uncerts_loglin << "{ 1 JetEta 1 JetPt [0] kFSR_err cov(0,0) cov(1,1) cov(0,1) }" << endl;
    uncerts_loglin << "{ 1 JetEta 1 JetPt sqrt(fabs([0]*[0]+[1]+[2]*log(x)*log(x)+2*[3]*log(x))) Correction L2Relative}" << endl;

    for (int j=n_eta-1; j>0; --j){
      output << fixed << std::setprecision(6)  << "  -" << eta_range[j]<< " -" << eta_range[j-1] << "   11   10 6500   55   " << ptave_data[j-1]->FindLastBinAbove(0.)*10 << "   " << 1/flat_norm << " " << hist_kfsr_fit_mpf->GetBinContent(j) << "   " << f2[j-1]->GetParameter(0) << " 0   1 0.0000 0.0" << endl;
      
      output_loglin << fixed << std::setprecision(6)  << "  -" << eta_range[j]<< " -" << eta_range[j-1] << "   11   10 6500   55   " << ptave_data[j-1]->FindLastBinAbove(0.)*10 << "   " << 1/loglin_norm << " " << hist_kfsr_fit_mpf->GetBinContent(j) << "   " << f1[j-1]->GetParameter(0) << " " << f1[j-1]->GetParameter(1) << "   1 0.0000 0.0" << endl;

      uncerts << fixed << std::setprecision(6) << "-" << eta_range[j] << " -" << eta_range[j-1] << "  3    55    " 
	      << ptave_data[j-1]->FindLastBinAbove(0.)*10 << "  " << hist_kfsr_fit_mpf->GetBinError(j-1) / flat_norm 
	      <<" "<< f2[j-1]->GetParError(0) << endl;

      uncerts_loglin << fixed << std::setprecision(6) << "-" << eta_range[j] << " -" 
		     << eta_range[j-1] << "  6    55    " << ptave_data[j-1]->FindLastBinAbove(0.)*10 
		     << " " << hist_kfsr_fit_mpf->GetBinError(j-1) / loglin_norm  
		     << " " <<Vcov[0][j-1] <<" "<<Vcov[1][j-1]<<" "<<Vcov[2][j-1]<<endl; 
    }


    for (int j=0; j<n_eta-1; j++){
      output << fixed << std::setprecision(6)  << "   " << eta_range[j]<< "  " << eta_range[j+1] << "   11   10 6500   55   " 
	     << ptave_data[j]->FindLastBinAbove(0.)*10 << "   " << 1/flat_norm << " " << hist_kfsr_fit_mpf->GetBinContent(j+1) 
	     << "   " << f2[j]->GetParameter(0) << " 0   1 0.0000 0.0"  << endl;

      output_loglin << fixed << std::setprecision(6)  << "   " << eta_range[j]<< "  " << eta_range[j+1] << "   11   10 6500   55   " 
		    << ptave_data[j]->FindLastBinAbove(0.)*10 << "   " << 1/loglin_norm << " " 
		    << hist_kfsr_fit_mpf->GetBinContent(j+1) << "   " << f1[j]->GetParameter(0) 
		    << " " << f1[j]->GetParameter(1) << "   1 0.0000 0.0" << endl;

      uncerts << fixed << std::setprecision(6) << " " << eta_range[j] << "  " << eta_range[j+1] << "  3    55    " 
	      << ptave_data[j]->FindLastBinAbove(0.)*10 
	      <<" "<< hist_kfsr_fit_mpf->GetBinError(j)/flat_norm<< " " << f2[j]->GetParameter(0) <<endl;

      uncerts_loglin << fixed << std::setprecision(6) << " " << eta_range[j] << "  " << eta_range[j+1] << "  6    55    " 
		     << ptave_data[j]->FindLastBinAbove(0.)*10 << " " 
		     << hist_kfsr_fit_mpf->GetBinError(j)/loglin_norm<< " "<< Vcov[0][j]<<" "<< Vcov[1][j]<<" "<< Vcov[2][j] << endl; 
    }

    //pT-dependence plots, setting bin contents and errors
    for(int k=0; k<5; k++){
      for (int j=0; j<n_eta-1; j++){
	double pave_value_corr;
	if(graph_filled[j]) {if(k==0) pave_value_corr = ptave_data[j]->GetMean();}
	else{
	  if(k==0) pave_value_corr = 0.0001;
	}
	if(k==1) pave_value_corr = 120;
	if(k==2) pave_value_corr = 60;
	if(k==3) pave_value_corr = 240;
	if(k==4) pave_value_corr = 480;

                                              
	//Residual_logpt_MPF->SetBinContent(j+1,hist_kfsr_fit_mpf->GetBinContent(j+1)*(f1[j]->GetParameter(0) +f1[j]->GetParameter(1)*TMath::Log(pave_value_corr)))
	//in above formula, no normalization has been applied!
	Residual_logpt_MPF->SetBinContent(j+1,hist_kfsr_fit_mpf->GetBinContent(j+1)*(f1[j]->GetParameter(0)
										     +f1[j]->GetParameter(1)*TMath::Log(pave_value_corr))/loglin_norm);
	Residual_logpt_MPF->SetBinError(j+1, sqrt(pow(f1[j]->GetParameter(0)+f1[j]->GetParameter(1)*TMath::Log(pave_value_corr),2)*pow(hist_kfsr_fit_mpf->GetBinError(j+1),2)   + pow(hist_kfsr_fit_mpf->GetBinContent(j+1),2)*(Vcov[0][j]+Vcov[1][j]*pow(TMath::Log(pave_value_corr),2)+2*Vcov[2][j]*TMath::Log(pave_value_corr)))/loglin_norm);
	ptave_logpt_MPF->SetBinError(j+1,sqrt(Vcov[0][j]
					      +Vcov[1][j]*pow(TMath::Log(pave_value_corr),2)+2*Vcov[2][j]*TMath::Log(pave_value_corr)));
	ptave_logpt_MPF->SetBinContent(j+1,f1[j]->GetParameter(0)+f1[j]->GetParameter(1)*TMath::Log(pave_value_corr));

                                                
	//Residual_const_MPF->SetBinContent(j+1,hist_kfsr_fit_mpf->GetBinContent(j+1)*f2[j]->GetParameter(0));
	//in above formula, no normalization has been applied!
	Residual_const_MPF->SetBinContent(j+1,hist_kfsr_fit_mpf->GetBinContent(j+1)*f2[j]->GetParameter(0)/flat_norm);
	Residual_const_MPF->SetBinError(j+1, sqrt( pow(f2[j]->GetParameter(0)*hist_kfsr_fit_mpf->GetBinError(j+1),2)+pow( hist_kfsr_fit_mpf->GetBinContent(j+1)*f2[j]->GetParError(0),2)  ) /flat_norm );
	ptave_const_MPF->SetBinContent(j+1,f2[j]->GetParameter(0));
	ptave_const_MPF->SetBinError(j+1,f2[j]->GetParError(0));

	/*
	cout << "res log MPF:" << Residual_logpt_MPF->GetBinContent(j+1) << " +- " << Residual_logpt_MPF->GetBinError(j+1) << endl;
	cout << "const MPF:" << Residual_const_MPF->GetBinContent(j+1) << " +- " << Residual_const_MPF->GetBinError(j+1) << endl;
	cout << "RATIO log MPF:" << ptave_logpt_MPF->GetBinContent(j+1) << " +- " << ptave_logpt_MPF->GetBinError(j+1) << endl;
	cout << "RATIO const MPF:" << ptave_const_MPF->GetBinContent(j+1) << " +- " << ptave_const_MPF->GetBinError(j+1) << endl;
	*/
      }

      //File containing the results with kFSR correction applied
      TFile* outputfile;
      TString variation2;
      if(k==0){
	outputfile = new TFile(CorrectionObject::_outpath+"Histo_Res_MPF_L1_"+CorrectionObject::_generator_tag+"_"+CorrectionObject::_jettag+".root","RECREATE");
      }
      if(k>0){
	if(k==1) variation2 = "central";
	if(k==2) variation2 = "down";
	if(k==3) variation2 = "up";
	if(k==4) variation2 = "doubleup";
	outputfile = new TFile(CorrectionObject::_outpath+"Histo_Res_MPF_L1_"+CorrectionObject::_generator_tag+"_"+CorrectionObject::_jettag+"_"+variation2+".root","RECREATE");
      }

      hist_kfsr_mpf->Write();
      hist_kfsr_fit_mpf->Write();                                                                                                                
      kfsr_fit_mpf->Write();
      ptave_const_MPF->Write();                                                                                                                    
      ptave_logpt_MPF->Write();
      
      Residual_logpt_MPF->Write();
      Residual_const_MPF->Write();
      outputfile->Write();
      outputfile->Close();

      //File containing the pure ratios, no kFSR correction applied
      TFile* outputfile2;
      if(k==0){
	outputfile2 = new TFile(CorrectionObject::_outpath+"Histo_ptave_MPF_L1_"+CorrectionObject::_generator_tag+"_"+CorrectionObject::_jettag+".root","RECREATE");
      }
      if(k>0){
	if(k==1) variation2 = "central";
	if(k==2) variation2 = "down";
	if(k==3) variation2 = "up";
	if(k==4) variation2 = "doubleup";
	outputfile2 = new TFile(CorrectionObject::_outpath+"Histo_ptave_MPF_L1_"+CorrectionObject::_generator_tag+"_"+CorrectionObject::_jettag+"_"+variation2+".root","RECREATE");
      }
    
      ptave_const_MPF->Write();
      ptave_logpt_MPF->Write();
      for (int j=0; j<n_eta-1; j++){
	f1[j]->Write();
	f2[j]->Write();
      }
      outputfile2->Write();
      outputfile2->Close();

      //delete stuff
      delete outputfile2;
      delete outputfile;
    }

    //delete stuff
    delete Residual_logpt_MPF;
    delete Residual_const_MPF;
    delete ptave_const_MPF;
    delete ptave_logpt_MPF;
    delete hist_kfsr_fit_mpf;
    delete hist_kfsr_mpf;
    delete kfsr_fit_mpf;
    delete kfsr_mpf;
  } //if(mpfMethod) ends here


  /* ++++++++++++++++++++++++++ Calculate L2Residuals PT balance ++++++++++++++++++++++++++++++ */

  else{
    cout<<"HELLO EVERYBODY!"<<endl;

    TFile* kfsr_dijet;
    kfsr_dijet = new TFile(CorrectionObject::_outpath+"Histo_KFSR_DiJet_"+CorrectionObject::_generator_tag+"_L1.root","READ");
    TH1D* hist_kfsr_dijet = (TH1D*)kfsr_dijet->Get("kfsr_dijet");
    
    //kFSR fit function
    TF1 *kfsr_fit_dijet = new TF1("kfsr_fit_dijet","[0]+([1]*TMath::CosH(x))/(1+[2]*TMath::CosH(x))",0,5.); //Range: 0,5. by default
    
    bool fit_fullrange = false;
    bool fit_285 = false;
    //    bool fit_fullrange = true;
    //tune the initial values for the fit
    if(CorrectionObject::_generator == "herwig"){
      //Initial values for herwig
      if(CorrectionObject::_collection == "AK4Puppi"){
	kfsr_fit_dijet->SetParameters(1.,1.,100.);
	if(CorrectionObject::_runnr == "G"){
	  kfsr_fit_dijet->SetParameters(1,0,0.5);
	}
	if(CorrectionObject::_runnr == "E"){
	  kfsr_fit_dijet->SetParameters(1,-100,300);
	}
	if(CorrectionObject::_runnr == "F"){
	  kfsr_fit_dijet->SetParameters(5,-600,100);
	}
      }
      if(CorrectionObject::_collection == "AK8CHS" || CorrectionObject::_collection == "AK4CHS") kfsr_fit_dijet->SetParameters(1.,-0.03,0.4); 
      else if (CorrectionObject::_collection == "AK8Puppi") kfsr_fit_dijet->SetParameters(10.,-1000.,200.); 
    }

    ///Set Parameter for kFSR Fit (pT-Balance)
    else if(CorrectionObject::_generator == "pythia"){
      //Initial values for pythia
      if(CorrectionObject::_collection == "AK4CHS"){ 
	//RunBCD
	if(CorrectionObject::_runnr == "BCD"){ 
	  if(CorrectionObject::_closuretest) kfsr_fit_dijet->SetParameters(-2,500,150.); //Closure Test
	  if(!CorrectionObject::_closuretest) kfsr_fit_dijet->SetParameters(6, 700, -100.); //reweighted MC, RES
	
	fit_fullrange = false;
        fit_285 = true;
	}
	
	else if(CorrectionObject::_runnr == "E") kfsr_fit_dijet->SetParameters(2,300,300.); //CLOSURETEST
	else if(CorrectionObject::_runnr == "F") kfsr_fit_dijet->SetParameters(3,-800,300.); //CLOSURETEST
	else if(CorrectionObject::_runnr == "EFearly"){
	  if(CorrectionObject::_closuretest) kfsr_fit_dijet->SetParameters(-6,2000,300); //CLOSURETEST
	  if(!CorrectionObject::_closuretest)kfsr_fit_dijet->SetParameters(1,500,-100); //reweighted MC, RES
	  
	fit_fullrange = false;
        fit_285 = true;
	}

	else if(CorrectionObject::_runnr == "FlateG"){
	  if(CorrectionObject::_closuretest) kfsr_fit_dijet->SetParameters(-5,2000,300); //CLOSURETEST
	  if(!CorrectionObject::_closuretest)  kfsr_fit_dijet->SetParameters(1,3000,300.); //RES
	  
	fit_fullrange = false;
	fit_285 = true; 	
	}
	else if(CorrectionObject::_runnr == "G") kfsr_fit_dijet->SetParameters(2,-400,300.); //CLOSURETEST
	else if(CorrectionObject::_runnr == "H"){
	  if(CorrectionObject::_closuretest)   kfsr_fit_dijet->SetParameters(-9,5000,500); //CLOSURETEST
	  if(!CorrectionObject::_closuretest)  kfsr_fit_dijet->SetParameters(-4,1000,200); //reweighted MC, RES
	  
	fit_fullrange = false;
	fit_285 = false;
	  
	}
	else kfsr_fit_dijet->SetParameters(0,0,200.); 
      }


      else if(CorrectionObject::_collection == "AK8CHS") {
	kfsr_fit_dijet->SetParameters(1.,0,0.05); 
	if(CorrectionObject::_runnr == "BCD") {
	  kfsr_fit_dijet->SetParameters(1.,-0.005,0.5); 
	  fit_fullrange = true;
	  hist_kfsr_dijet->Fit("kfsr_fit_dijet","SR","",0,5.);
	}
	else if(CorrectionObject::_runnr == "E"){
	  kfsr_fit_dijet->SetParameters(1.,-0.001,0.1); 
	  fit_fullrange = true;
	  hist_kfsr_dijet->Fit("kfsr_fit_dijet","SR","",0,5.);
	}
	else if(CorrectionObject::_runnr == "F") kfsr_fit_dijet->SetParameters(1.,-30,100); 
	else if(CorrectionObject::_runnr == "G"){
	  kfsr_fit_dijet->SetParameters(2,-200,200); 
	  fit_fullrange = true;
	  hist_kfsr_dijet->Fit("kfsr_fit_dijet","SR","",0,5.);
	}
      }
      else if(CorrectionObject::_collection == "AK8Puppi") kfsr_fit_dijet->SetParameters(1.,-200,100.); 
      else if(CorrectionObject::_collection == "AK4Puppi"){
	kfsr_fit_dijet->SetParameters(1.5,-50,100);
	if(CorrectionObject::_runnr == "BCD") kfsr_fit_dijet->SetParameters(1,-0.01,0.5); 
	if(CorrectionObject::_runnr == "E"){
	  kfsr_fit_dijet->SetParameters(1,-0.01,0.5); 
	  kfsr_fit_dijet->SetParameters(1,-0.01,0.5); 
	}
	if(CorrectionObject::_runnr == "G") {
	  hist_kfsr_dijet->Fit("kfsr_fit_dijet","SR","",0,5.);
	  fit_fullrange = true;
	}
      }
    }
    else if(CorrectionObject::_generator == "madgraph"){
      kfsr_fit_dijet->SetParameters(1,-50,200);



    }
    else throw runtime_error("PTextrapolation: Invalid generator specified.");

    //Finally perform the fit 
    kfsr_fit_dijet->SetLineColor(kBlue+1);
    std::cout<<"------ !!! kFSR pT-balance fit !!! ------"<<std::endl;
    if(fit_fullrange) hist_kfsr_dijet->Fit("kfsr_fit_dijet","SR","",0,5.);
    else if(!fit_285) hist_kfsr_dijet->Fit("kfsr_fit_dijet","SR","",0,2.65);
    else              hist_kfsr_dijet->Fit("kfsr_fit_dijet","SR","",0,2.85);
    //    else hist_kfsr_dijet->Fit("kfsr_fit_dijet","SR","",0,2.4);


    //Create a histogram to hold the confidence intervals                                                                                        
    TH1D *hist_kfsr_fit_dijet = (TH1D*)hist_kfsr_dijet->Clone();
    (TVirtualFitter::GetFitter())->GetConfidenceIntervals(hist_kfsr_fit_dijet);
    //Now the "hist_kfsr_fit" histogram has the fitted function values as the
    //bin contents and the confidence intervals as bin errors

    hist_kfsr_fit_dijet->SetStats(kFALSE);
    hist_kfsr_fit_dijet->SetFillColor(kBlue-10);     
    hist_kfsr_fit_dijet->SetName("hist_kfsr_fit_dijet");
    hist_kfsr_fit_dijet->SetTitle("kfsr fit for dijet");

    double flat[n_eta-1];
    double loglin[n_eta-1];
    for (int j=0; j<n_eta-1; j++){
      flat[j] = hist_kfsr_fit_dijet->GetBinContent(j+1)*f2[j]->GetParameter(0);
      loglin[j] = hist_kfsr_fit_dijet->GetBinContent(j+1)*(f1[j]->GetParameter(0)+f1[j]->GetParameter(1)*TMath::Log(ptave_data[j]->GetMean()) );
    }

    double flat_norm = 0, loglin_norm = 0;
    for (int j=0; j<n_etabarr; j++){
      flat_norm += flat[j];
      loglin_norm += loglin[j];
    }
    flat_norm = flat_norm/n_etabarr;
    loglin_norm = loglin_norm/n_etabarr;
    for (int j=0; j<n_eta-1; j++){
      flat[j] = flat[j]/flat_norm;
      loglin[j] = loglin[j]/loglin_norm;
    }

    cout << "normalization factor (flat): " << flat_norm << endl;
    cout << "normalization factor (logl): " << loglin_norm << endl;

    //Histograms holding the output
    TH1D* Residual_logpt_DiJet = new TH1D("res_logpt_dijet","res_logpt_dijet", n_eta-1,eta_bins);
    TH1D* Residual_const_DiJet = new TH1D("res_const_dijet","res_const_dijet", n_eta-1,eta_bins);
    TH1D* ptave_const_DiJet = new TH1D("ptave_const_dijet","ptave_const_dijet", n_eta-1,eta_bins);
    TH1D* ptave_logpt_DiJet = new TH1D("ptave_logpt_dijet","ptave_logpt_dijet", n_eta-1,eta_bins);


    ofstream output, output_loglin, uncerts, uncerts_loglin;
    output.open(CorrectionObject::_outpath+"output/Summer16_23Sep2016_pT_FLAT_L2Residual_"+CorrectionObject::_generator_tag+"_"+CorrectionObject::_jettag+".txt");
    output_loglin.open(CorrectionObject::_outpath+"output/Summer16_23Sep2016_pT_LOGLIN_L2Residual_"+CorrectionObject::_generator_tag+"_"+CorrectionObject::_jettag+".txt");
    uncerts.open(CorrectionObject::_outpath+"output/Summer16_23Sep2016_pT_FLAT_L2Residual_"+CorrectionObject::_generator_tag+"_"+CorrectionObject::_jettag+".txt.STAT");
    uncerts_loglin.open(CorrectionObject::_outpath+"output/Summer16_23Sep2016_pT_LOGLIN_L2Residual_"+CorrectionObject::_generator_tag+"_"+CorrectionObject::_jettag+".txt.STAT");


    output  << "{ 1 JetEta 1 JetPt [2]*([3]*([4]+[5]*TMath::Log(max([0],min([1],x))))*1./([6]+[7]*100./3.*(TMath::Max(0.,1.03091-0.051154*pow(x,-0.154227))-TMath::Max(0.,1.03091-0.051154*TMath::Power(208.,-0.154227)))+[8]*((-2.36997+0.413917*TMath::Log(x))/x-(-2.36997+0.413917*TMath::Log(208))/208))) Correction L2Relative}" << endl;
    output_loglin  << "{ 1 JetEta 1 JetPt [2]*([3]*([4]+[5]*TMath::Log(max([0],min([1],x))))*1./([6]+[7]*100./3.*(TMath::Max(0.,1.03091-0.051154*pow(x,-0.154227))-TMath::Max(0.,1.03091-0.051154*TMath::Power(208.,-0.154227)))+[8]*((-2.36997+0.413917*TMath::Log(x))/x-(-2.36997+0.413917*TMath::Log(208))/208))) Correction L2Relative}" << endl;
    uncerts << "{ 1 JetEta 1 JetPt [0] kFSR_err f0_err}" << endl;
    //uncerts_loglin << "{ 1 JetEta 1 JetPt [0] kFSR_err cov(0,0) cov(1,1) cov(0,1) }" << endl;
    uncerts_loglin << "{ 1 JetEta 1 JetPt sqrt(fabs([0]*[0]+[1]+[2]*log(x)*log(x)+2*[3]*log(x))) Correction L2Relative}" << endl;

    for (int j=n_eta-1; j>0; --j){
      output << fixed << std::setprecision(6)  << "  -" << eta_range[j]<< " -" << eta_range[j-1] << "   11   10 6500   55   " << ptave_data[j-1]->FindLastBinAbove(0.)*10 << "   " << 1/flat_norm << " " << hist_kfsr_fit_dijet->GetBinContent(j) << "   " << f2[j-1]->GetParameter(0) << " 0   1 0.0000 0.0" << endl;
      output_loglin << fixed << std::setprecision(6)  << "  -" << eta_range[j]<< " -" << eta_range[j-1] << "   11   10 6500   55   " << ptave_data[j-1]->FindLastBinAbove(0.)*10 << "   " << 1/loglin_norm << " " << hist_kfsr_fit_dijet->GetBinContent(j) << "   " << f1[j-1]->GetParameter(0) << " " << f1[j-1]->GetParameter(1) <<  "   1 0.0000 0.0" << endl;
 
      uncerts << fixed << std::setprecision(6) << "-" << eta_range[j] << " -" << eta_range[j-1] << "  3    55    "
	      << ptave_data[j-1]->FindLastBinAbove(0.)*10 << "  " << hist_kfsr_fit_dijet->GetBinError(j-1) / flat_norm
	      <<" "<< f2[j-1]->GetParError(0) << endl; 
      uncerts_loglin << fixed << std::setprecision(6) << "-" << eta_range[j] << " -"
		     << eta_range[j-1] << "  6    55    " << ptave_data[j-1]->FindLastBinAbove(0.)*10
		     << " " << hist_kfsr_fit_dijet->GetBinError(j-1) / loglin_norm<< " " <<Vcov[0][j-1] <<" "<<Vcov[1][j-1]<<" "<<Vcov[2][j-1]<<endl;   
    }


    for (int j=0; j<n_eta-1; j++){ 
      output << fixed << std::setprecision(6)  << "   " << eta_range[j]<< "  " << eta_range[j+1] << "   11   10 6500   55   " << ptave_data[j]->FindLastBinAbove(0.)*10 << "   " << 1/flat_norm << " " << hist_kfsr_fit_dijet->GetBinContent(j+1) << "   " << f2[j]->GetParameter(0) << " 0   1 0.0000 0.0" << endl;
      output_loglin << fixed << std::setprecision(6)  << "   " << eta_range[j]<< "  " << eta_range[j+1] << "   11   10 6500   55   " << ptave_data[j]->FindLastBinAbove(0.)*10 << "   " << 1/loglin_norm << " " << hist_kfsr_fit_dijet->GetBinContent(j+1) << "   " << f1[j]->GetParameter(0) << " " << f1[j]->GetParameter(1) <<  "   1 0.0000 0.0" << endl;

      uncerts << fixed << std::setprecision(6) << " " << eta_range[j] << "  " << eta_range[j+1] << "  3    55    "
	      << ptave_data[j]->FindLastBinAbove(0.)*10<<" "<< hist_kfsr_fit_dijet->GetBinError(j)/flat_norm<< " " 
	      << f2[j]->GetParameter(0) <<endl;                                         
      uncerts_loglin << fixed << std::setprecision(6) << " " << eta_range[j] << "  " << eta_range[j+1] 
		     << "  6    55    "<< ptave_data[j]->FindLastBinAbove(0.)*10 << " "
		     << hist_kfsr_fit_dijet->GetBinError(j)/loglin_norm<< " "<< Vcov[0][j]<<" "<< Vcov[1][j]<<" "<< Vcov[2][j] << endl;     
    }



    for(int k=0; k<5; k++){  
      for (int j=0; j<n_eta-1; j++){
	double pave_corr;
	if(graph_filled[j]) {if(k==0) pave_corr = ptave_data[j]->GetMean();}
	else{
	  if(k==0) pave_corr = 0.0001;
	}
	if(k==1) pave_corr = 120;
	if(k==2) pave_corr = 60;
	if(k==3) pave_corr = 240;
	if(k==4) pave_corr = 480;

	Residual_logpt_DiJet->SetBinContent(j+1,hist_kfsr_fit_dijet->GetBinContent(j+1)*(f1[j]->GetParameter(0)
											 +f1[j]->GetParameter(1)*TMath::Log(pave_corr))/loglin_norm);
	Residual_logpt_DiJet->SetBinError(j+1, sqrt(pow(f1[j]->GetParameter(0)+f1[j]->GetParameter(1)*TMath::Log(pave_corr),2)*pow(hist_kfsr_fit_dijet->GetBinError(j+1),2)+ pow(hist_kfsr_fit_dijet->GetBinContent(j+1),2)*(Vcov[0][j]+Vcov[1][j]*pow(TMath::Log(pave_corr),2)+2*Vcov[2][j]*TMath::Log(pave_corr)))/loglin_norm);

	ptave_logpt_DiJet->SetBinContent(j+1,f1[j]->GetParameter(0)+f1[j]->GetParameter(1)*TMath::Log(pave_corr));
	ptave_logpt_DiJet->SetBinError(j+1,sqrt((Vcov[0][j]+Vcov[1][j]*pow(TMath::Log(ptave_data[j]->GetMean()),2)+2*Vcov[2][j]*TMath::Log(pave_corr))));

	Residual_const_DiJet->SetBinContent(j+1,hist_kfsr_fit_dijet->GetBinContent(j+1)*f2[j]->GetParameter(0)/flat_norm);
	Residual_const_DiJet->SetBinError(j+1, sqrt( pow(f2[j]->GetParameter(0)*hist_kfsr_fit_dijet->GetBinError(j+1),2)+pow(hist_kfsr_fit_dijet->GetBinContent(j+1)*f2[j]->GetParError(0),2)  )/flat_norm  );
	ptave_const_DiJet->SetBinContent(j+1,f2[j]->GetParameter(0));
	ptave_const_DiJet->SetBinError(j+1,f2[j]->GetParError(0));

	/*
	cout << "res log pt balance:" << Residual_logpt_DiJet->GetBinContent(j+1) << " +- " << Residual_logpt_DiJet->GetBinError(j+1) << endl;
	cout << "res const pt balance:" << Residual_const_DiJet->GetBinContent(j+1) << " +- " << Residual_const_DiJet->GetBinError(j+1) << endl;
	cout << "RATIO log pt balance:" << ptave_logpt_DiJet->GetBinContent(j+1) << " +- " << ptave_logpt_DiJet->GetBinError(j+1) << endl;
	cout << "RATIO const pt balance:" << ptave_const_DiJet->GetBinContent(j+1) << " +- " << ptave_const_DiJet->GetBinError(j+1) << endl;
	*/
      }


      //File containing the corrections, with kFSR correction applied
      TFile* outputfile;
      TString variation2;
      if(k==0){
	outputfile = new TFile(CorrectionObject::_outpath+"Histo_Res_DiJet_L1_"+CorrectionObject::_generator_tag+"_"+CorrectionObject::_jettag+".root","RECREATE");
      }
      if(k>0){
	if(k==1) variation2 = "central";
	if(k==2) variation2 = "down";
	if(k==3) variation2 = "up";
	if(k==4) variation2 = "doubleup";
	outputfile = new TFile(CorrectionObject::_outpath+"Histo_Res_DiJet_L1_"+CorrectionObject::_generator_tag+"_"+CorrectionObject::_jettag+"_"+variation2+".root","RECREATE");
      }
    
      hist_kfsr_dijet->Write();
      hist_kfsr_fit_dijet->Write();
      kfsr_fit_dijet->Write();
      ptave_const_DiJet->Write();
      ptave_logpt_DiJet->Write();
                  
      Residual_logpt_DiJet->Write();
      Residual_const_DiJet->Write();
      outputfile->Write();
      outputfile->Close();


      //Contains the pT-Extrapolations
      TFile* outputfile2;
      if(k==0){
	outputfile2 = new TFile(CorrectionObject::_outpath+"Histo_ptave_DiJet_L1_"+CorrectionObject::_generator_tag+"_"+CorrectionObject::_jettag+".root","RECREATE");
      }
      if(k>0){
	if(k==1) variation2 = "central";
	if(k==2) variation2 = "down";
	if(k==3) variation2 = "up";
	if(k==4) variation2 = "doubleup";
	outputfile2 = new TFile(CorrectionObject::_outpath+"Histo_ptave_DiJet_L1_"+CorrectionObject::_generator_tag+"_"+CorrectionObject::_jettag+"_"+variation2+".root","RECREATE");
      }
    

      ptave_const_DiJet->Write();
      ptave_logpt_DiJet->Write();
      for (int j=0; j<n_eta-1; j++){
	f1[j]->Write();
	f2[j]->Write();
      }             
      outputfile2->Write();
      outputfile2->Close();

      //delete stuff
      delete outputfile2;
      delete outputfile;
    }



    //delete stuff
    delete ptave_logpt_DiJet;
    delete ptave_const_DiJet;
    delete Residual_const_DiJet;
    delete Residual_logpt_DiJet;
    delete hist_kfsr_fit_dijet;
    delete hist_kfsr_dijet;
    delete kfsr_fit_dijet;
    delete kfsr_dijet;
  } //end of (if(mpfMethod...else))




//delete everything
  delete c_kfsr_fit;

  for (int j=0; j<n_eta-1; j++) {
    delete asd[j];
    delete f1[j];
    delete f2[j];
  }

  delete line;

  for(int j=0; j<n_eta-1; j++){
    delete graph1_mpf[j];
  }
 
  for(int j=0; j<n_eta-1; j++){
    delete pr_data_asymmetry[j];
    delete pr_data_B[j];
    delete pr_mc_asymmetry[j];
    delete pr_mc_B[j];
    delete hdata_asymmetry[j];
    delete hdata_B[j];
    delete hmc_asymmetry[j];
    delete hmc_B[j];
  }
     
  for(int i=0; i<n_eta-1; i++) delete ptave_data[i];
  delete m_gStyle;

}
