#include "UHH2/BaconJets/include/jet_corrections.h"

#include "UHH2/BaconTrans/baconheaders/TJet.hh"
#include "UHH2/BaconJets/include/constants.h"
using namespace std;
namespace uhh2bacon {

JetCorrections::JetCorrections(uhh2::Context & ctx) :
    context(ctx),
    event(0)
{
  auto jetCollection = ctx.get("jetCollection");
  h_jets = context.declare_event_input<TClonesArray>(jetCollection);
  //h_jets = context.declare_event_input<TClonesArray>("AK4PFPUPPI");

  h_eventInfo = context.declare_event_input<baconhep::TEventInfo>("Info");
 // h_jetsout = context.declare_event_output<TClonesArray>("Jet05");

}

void JetCorrections::SetEvent(uhh2::Event& evt)
{
   event = &evt;
   assert(event);
}

bool JetCorrections::JetMatching()
{
  assert(event);

  //const TClonesArray & jsout = event->set(h_jetsout);
  const TClonesArray & js = event->get(h_jets);
  const baconhep::TEventInfo & info = event->get(h_eventInfo);


  baconhep::TEventInfo* eventInfo= new baconhep::TEventInfo(info);
  assert(eventInfo);

  int njets = js.GetEntries();

  // njets >= 2
  if (njets<2) return false;

  baconhep::TJet* jet1 = (baconhep::TJet*)js[0];
  baconhep::TJet* jet2 = (baconhep::TJet*)js[1];

//   double delra_R_jet1 = pow(pow(jet1->genphi - jet1->phi,2) + pow(jet1->geneta - jet1->eta,2),0.5);
//   double delra_R_jet2 = pow(pow(jet2->genphi - jet2->phi,2) + pow(jet2->geneta - jet2->eta,2),0.5);
  double delra_R_jet1 = pow(pow(TVector2::Phi_mpi_pi(jet1->genphi - jet1->phi),2) + pow(jet1->geneta - jet1->eta,2),0.5);
  double delra_R_jet2 = pow(pow(TVector2::Phi_mpi_pi(jet2->genphi - jet2->phi),2) + pow(jet2->geneta - jet2->eta,2),0.5);

  if ((delra_R_jet1 > s_delta_R) || (delra_R_jet2 > s_delta_R)) return false;
 // std::cout << " runNum: "<< eventInfo->runNum<< " evtNum: "<< eventInfo->evtNum <<" delra_R_jet1 = "<< delra_R_jet1 << std::endl;

 return true;
}

bool JetCorrections::JetResolutionSmearer(int  direction)
{
  assert(event);

  const TClonesArray & js = event->get(h_jets);
  baconhep::TEventInfo & info = event->get(h_eventInfo);

 // assert(eventInfo);

  Int_t njets = js.GetEntries();
  baconhep::TEventInfo* eventInfo= &info;//new baconhep::TEventInfo(info);

  TVector2 met;
  met.SetMagPhi(eventInfo->pfMET ,eventInfo->pfMETphi);

//   cout<<"old  MET: "<<eventInfo->pfMET<<" old phi MET: "<<eventInfo->pfMETphi<<endl;
  // float recopt, rawpt, new_pt;
 float recopt, new_pt;
  for(int i=0; i < njets; ++i) {

    baconhep::TJet* jet = (baconhep::TJet*)js[i];
    float genpt = jet->genpt;
    float pt_sm = jet->pt;

    //ignore unmatched jets (which have zero vector) or jets with very low pt:
    if(genpt < 15.0) continue;

    recopt = jet->pt;
    float abseta = fabs(jet->eta);
    size_t ieta = 0;
    while(ieta < n && eta_hi[ieta] < abseta) ++ieta;
    if(ieta == n) ieta = n-1;
    float c;
    if(direction == 0){
        c = c_nominal[ieta]; //central
    } else if(direction == 1){
        c = c_nominal[ieta] + c_err_nominal[ieta]; //scale up
    } else{
        c = c_nominal[ieta] - c_err_nominal[ieta]; //scale down
    }

    new_pt = std::max(0.0f, genpt + c * (recopt - genpt));
    pt_sm *= new_pt / recopt;
    
    //propagate JER shifts to MET by using same factor, but for raw jet p4:
    //    rawpt = jet->ptRaw;
    float scalechange = new_pt / recopt;
    TVector2 jetvect;//(jet->ptRaw*cos(jet->phi),jet->ptRaw*sin(jet->phi));
    jetvect.SetMagPhi(jet->pt,jet->phi);

    met += jetvect;
    jetvect *= scalechange;
    met -= jetvect;

    jet->pt = new_pt;
  }


  eventInfo->pfMET = met.Mod();
  eventInfo->pfMETphi = met.Phi();
//   cout<<"new  MET: "<<eventInfo->pfMET<<" new phi MET: "<<eventInfo->pfMETphi<<endl;

  return true;
}

bool JetCorrections::FullJetCorrections()
{
    return JetMatching();

}

JetCorrections::~JetCorrections()
{
}

}