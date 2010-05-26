// -*- C++ -*-
//
// Package:    WenuCandidateFilter
// Class:      WenuCandidateFilter
// 
/**\class WenuCandidateFilter WenuCandidateFilter.cc EWKSoftware/EDMTupleSkimmerFilter/src/WenuCandidateFilter.cc

 Description: <one line class summary>

 Implementation:
    This class contains a filter that searches the event and finds whether
    it fulfills the W Candidate Criteria. If it fullfills them it creates a
    WenuCandidate and stores it in the event ..............................
    Definition of the Wenu Caldidate: 
    *   event that passes the trigger
    *   with an Gsf electron in fiducial
    *   with ET greater than a (configurable) threshold
    *   matched to an HLT object (configurable) with DR < (configurable)
    *   no second electron  in fiducial (configurable) with ET greater 
        than another (configurable) threshold.
    
    You can keep track of the definition of the Wenu Candidate by searching
    for  or grepping  RETURN (upper case)

 Changes Log:
 12Feb09  First Release of the code for CMSSW_2_2_X
 16Sep09  First Release for CMSSW_3_1_X
 09Dec09  Option to ignore trigger
 23Feb10  Added options to use Conversion Rejection, Expected missing hits
          and valid hit at first PXB
          Added option to calculate these criteria and store them in the pat electron object
          this is done by setting in the configuration the flags
	  calculateValidFirstPXBHit = true
          calculateConversionRejection = true
          calculateExpectedMissinghits = true
          Then the code calculates them and you can access all these from pat::Electron
	  myElec.userInt("PassValidFirstPXBHit")      0 fail, 1 passes
          myElec.userInt("PassConversionRejection")   0 fail, 1 passes
          myElec.userInt("NumberOfExpectedMissingHits") the number of lost hits
 01Mar10  2nd electron option to be isolated if requested by user
 Contact:
 Nikolaos.Rompotis@Cern.ch
 Imperial College London

*/


#ifndef WenuCandidateFilter_H
#define WenuCandidateFilter_H
// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDFilter.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
//
#include <vector>
#include <iostream>
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "FWCore/Common/interface/TriggerNames.h"
//
#include "TString.h"
#include "TMath.h"
#include "DataFormats/PatCandidates/interface/MET.h"
// this file used to exist in 22X, in 312 replaced by TriggerObject.h
//#include "DataFormats/PatCandidates/interface/TriggerPrimitive.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/TriggerObject.h"
// for conversion finder
#include "RecoEgamma/EgammaTools/interface/ConversionFinder.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
//
#include "DataFormats/Scalers/interface/DcsStatus.h"
//
// class declaration
//

class WenuCandidateFilter : public edm::EDFilter {
   public:
      explicit WenuCandidateFilter(const edm::ParameterSet&);
      ~WenuCandidateFilter();

   private:
      virtual Bool_t filter(edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;
      Bool_t isInFiducial(Double_t eta);
  Bool_t passEleIDCuts(pat::Electron *ele);
      // ----------member data ---------------------------
  Double_t ETCut_;
  Double_t METCut_;
  Double_t ETCut2ndEle_;
  edm::InputTag triggerCollectionTag_;
  edm::InputTag triggerEventTag_;
  std::string hltpath_;
  edm::InputTag hltpathFilter_;
  edm::InputTag electronCollectionTag_;
  edm::InputTag metCollectionTag_;

  Double_t BarrelMaxEta_;
  Double_t EndCapMaxEta_;
  Double_t EndCapMinEta_;
  Bool_t useTriggerInfo_;
  Bool_t electronMatched2HLT_;
  Double_t electronMatched2HLT_DR_;
  Bool_t vetoSecondElectronEvents_;
  Bool_t useVetoSecondElectronID_;
  std::string vetoSecondElectronIDType_;
  std::string vetoSecondElectronIDSign_;
  Double_t vetoSecondElectronIDValue_;
  Bool_t useValidFirstPXBHit_;
  Bool_t calculateValidFirstPXBHit_;
  Bool_t useConversionRejection_;
  Bool_t calculateConversionRejection_;
  Double_t dist_, dcot_;
  Bool_t useExpectedMissingHits_;
  Bool_t calculateExpectedMissingHits_;
  Int_t  maxNumberOfExpectedMissingHits_;
  Bool_t dataMagneticFieldSetUp_;
  edm::InputTag dcsTag_;
};
#endif
//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
WenuCandidateFilter::WenuCandidateFilter(const edm::ParameterSet& iConfig)
{
   //now do what ever initialization is needed
  // *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
  // I N P U T      P A R A M E T E R S  *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
  // *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
  // Cuts
  Double_t ETCut_D = 30.;
  ETCut_ = iConfig.getUntrackedParameter<double>("ETCut",ETCut_D);
  Double_t METCut_D = 0.;
  METCut_ = iConfig.getUntrackedParameter<double>("METCut",METCut_D);
  Double_t ETCut2ndEle_D = 20.;
  ETCut2ndEle_ = iConfig.getUntrackedParameter<double>("ETCut2ndEle",
						       ETCut2ndEle_D);
  vetoSecondElectronEvents_ = iConfig.getUntrackedParameter<bool>("vetoSecondElectronEvents",false);
  vetoSecondElectronIDType_ = iConfig.getUntrackedParameter<std::string>
    ("vetoSecondElectronIDType", "NULL");
  if (vetoSecondElectronIDType_ != "NULL") {
    useVetoSecondElectronID_ = true;
    vetoSecondElectronIDValue_ = iConfig.getUntrackedParameter<double>
      ("vetoSecondElectronIDValue");
    vetoSecondElectronIDSign_ = iConfig.getUntrackedParameter<std::string>
      ("vetoSecondElectronIDSign","=");
  }
  else useVetoSecondElectronID_ = false;
  //
  // preselection criteria: hit pattern
  useValidFirstPXBHit_ = 
    iConfig.getUntrackedParameter<Bool_t>("useValidFirstPXBHit",false);
  calculateValidFirstPXBHit_ = 
    iConfig.getUntrackedParameter<Bool_t>("calculateValidFirstPXBHit",false);
  useConversionRejection_ = 
    iConfig.getUntrackedParameter<Bool_t>("useConversionRejection",false);
  calculateConversionRejection_ =
    iConfig.getUntrackedParameter<Bool_t>("calculateConversionRejection",false);
  dist_ = iConfig.getUntrackedParameter<Double_t>("conversionRejectionDist", 0.02);
  dcot_ = iConfig.getUntrackedParameter<Double_t>("conversionRejectionDcot", 0.02);
  dataMagneticFieldSetUp_ = iConfig.getUntrackedParameter<Bool_t>("dataMagneticFieldSetUp",false);
  if (dataMagneticFieldSetUp_) {
    dcsTag_ = iConfig.getUntrackedParameter<edm::InputTag>("dcsTag");
  }
  useExpectedMissingHits_ = 
    iConfig.getUntrackedParameter<Bool_t>("useExpectedMissingHits",false);
  calculateExpectedMissingHits_ = 
    iConfig.getUntrackedParameter<Bool_t>("calculateExpectedMissingHits",false);
  maxNumberOfExpectedMissingHits_ = 
    iConfig.getUntrackedParameter<int>("maxNumberOfExpectedMissingHits",1);
  //
  //
  //
  Double_t BarrelMaxEta_D = 1.4442;
  Double_t EndCapMinEta_D = 1.56;
  Double_t EndCapMaxEta_D = 2.5;
  BarrelMaxEta_ = iConfig.getUntrackedParameter<double>("BarrelMaxEta",
                                                        BarrelMaxEta_D);
  EndCapMaxEta_ = iConfig.getUntrackedParameter<double>("EndCapMaxEta",
                                                        EndCapMaxEta_D);
  EndCapMinEta_ = iConfig.getUntrackedParameter<double>("EndCapMinEta",
                                                        EndCapMinEta_D);
  // trigger related
  std::string hltpath_D = "HLT_Ele15_LW_L1R";
  edm::InputTag triggerCollectionTag_D("TriggerResults","","HLT");
  edm::InputTag triggerEventTag_D("hltTriggerSummaryAOD", "", "HLT");
  edm::InputTag hltpathFilter_D("hltL1NonIsoHLTNonIsoSingleElectronLWEt15Track\
IsolFilter","","HLT");
  hltpath_=iConfig.getUntrackedParameter<std::string>("hltpath", hltpath_D);
  triggerCollectionTag_=iConfig.getUntrackedParameter<edm::InputTag>
    ("triggerCollectionTag", triggerCollectionTag_D);
  triggerEventTag_=iConfig.getUntrackedParameter<edm::InputTag>
    ("triggerEventTag", triggerEventTag_D);
  hltpathFilter_=iConfig.getUntrackedParameter<edm::InputTag>
    ("hltpathFilter",hltpathFilter_D);
  // trigger matching related:
  useTriggerInfo_ = iConfig.getUntrackedParameter<bool>("useTriggerInfo",true);
  electronMatched2HLT_ = iConfig.getUntrackedParameter<bool>("electronMatched2HLT");
  electronMatched2HLT_DR_=iConfig.getUntrackedParameter<double>("electronMatched2HLT_DR");
  // electrons and other
  edm::InputTag electronCollectionTag_D("selectedLayer1Electrons","","PAT");
  electronCollectionTag_=iConfig.getUntrackedParameter<edm::InputTag>
    ("electronCollectionTag",electronCollectionTag_D);
  //
  edm::InputTag metCollectionTag_D("selectedLayer1METs","","PAT");
  metCollectionTag_ = iConfig.getUntrackedParameter<edm::InputTag>
    ("metCollectionTag", metCollectionTag_D);

  //
  // *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
  //
  // now print a summary with what exactly the filter does:
  std::cout << "WenuCandidateFilter: Running Wenu Filter..." << std::endl;
  if (useTriggerInfo_) {
    std::cout << "WenuCandidateFilter: HLT Path   " << hltpath_ << std::endl;
    std::cout << "WenuCandidateFilter: HLT Filter "<<hltpathFilter_<<std::endl;
  }
  else {
    std::cout << "WenuCandidateFilter: Trigger info will not be used here" 
	      << std::endl;
  }
  std::cout << "WenuCandidateFilter: ET  > " << ETCut_ << std::endl;
  std::cout << "WenuCandidateFilter: MET > " << METCut_ << std::endl;
  if (vetoSecondElectronEvents_) {
    std::cout << "WenuCandidateFilter: VETO 2nd electron with ET > " 
	      << ETCut2ndEle_ << std::endl;
    if (useVetoSecondElectronID_) {
      std::cout<<"WenuCandidateFilter: VETO 2nd ele ID "  
	       << vetoSecondElectronIDType_ << vetoSecondElectronIDSign_
	       << vetoSecondElectronIDValue_
	       << std::endl;
    }
  }
  else {
    std::cout << "WenuCandidateFilter: No veto for 2nd electron applied " 
	      << std::endl;
  }
  if (electronMatched2HLT_ && useTriggerInfo_) {
    std::cout << "WenuCandidateFilter: Electron Candidate is required to "
	      << "match an HLT object with DR < " << electronMatched2HLT_DR_
	      << std::endl;
  } else {
    std::cout << "WenuCandidateFilter: Electron Candidate NOT required to "
	      << "match HLT object " << std::endl;
  }
  if (useValidFirstPXBHit_) {
    std::cout << "WenuCandidateFilter: Electron Candidate required to have "
              << "a valid hit in 1st PXB layer " << std::endl;
  }
  if (calculateValidFirstPXBHit_) {
    std::cout << "WenuCandidateFilter: Info about whether there is a valid 1st layer PXB hit "
	      << "will be stored: you can access that later by "
	      << "myElec.userInt(\"PassValidFirstPXBHit\")==1" << std::endl;
  }
  if (useExpectedMissingHits_) {
    std::cout << "WenuCandidateFilter: Electron Candidate required to have "
	      << " less than " << maxNumberOfExpectedMissingHits_ 
	      << " expected hits missing " << std::endl;
    ;
  }
  if (calculateExpectedMissingHits_) {
    std::cout << "WenuCandidateFilter: Missing Hits from expected inner layers "
              << "will be calculated and stored: you can access them later by " 
	      << "myElec.userInt(\"NumberOfExpectedMissingHits\")"   << std::endl;
  }
  if (useConversionRejection_) {
    std::cout << "WenuCandidateFilter: Electron Candidate required to pass "
	      << "EGAMMA Conversion Rejection criteria " << std::endl;
  }
  if (calculateConversionRejection_) {
    std::cout << "WenuCandidateFilter: EGAMMA Conversion Rejection criteria "
	      << "will be calculated and stored: you can access them later by " 
	      << "demanding for a successful electron "
	      << "myElec.userInt(\"PassConversionRejection\")==1"
	      << std::endl;
  }
  std::cout << "WenuCandidateFilter: Fiducial Cut: " << std::endl;
  std::cout << "WenuCandidateFilter:    BarrelMax: "<<BarrelMaxEta_<<std::endl;
  std::cout << "WenuCandidateFilter:    EndcapMin: " << EndCapMinEta_
	    << "  EndcapMax: " << EndCapMaxEta_
	    <<std::endl;
  //
  // extra info in the event
  // produces< reco::WenuCandidates > ("selectedWenuCandidates").setBranchAlias
  //  ("selectedWenuCandidates");
  produces< pat::CompositeCandidateCollection > 
    ("selectedWenuCandidates").setBranchAlias("selectedWenuCandidates");

}


WenuCandidateFilter::~WenuCandidateFilter()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called on each new Event  ------------
bool
WenuCandidateFilter::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;
   using namespace std;
   using namespace pat;
   // *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
   // TRIGGER REQUIREMENT                       *-*-*-*-*-*-*-*-*-*-*-*-*
   // *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
   //                                           *-*-*-*-*-*-*-*-*-*-*-*-*
   // the event should pass the trigger, otherwise no wenu candidate -*-*
   edm::Handle<edm::TriggerResults> HLTResults;
   iEvent.getByLabel(triggerCollectionTag_, HLTResults);
   Int_t passTrigger = 0;  
   if (HLTResults.isValid()) {
     const edm::TriggerNames & triggerNames = iEvent.triggerNames(*HLTResults);
     UInt_t trigger_size = HLTResults->size();
     UInt_t trigger_position = triggerNames.triggerIndex(hltpath_);
     if (trigger_position < trigger_size)
       passTrigger = (int) HLTResults->accept(trigger_position);
   }
   else {
     //std::cout << "TriggerResults missing from this event.." << std::endl;
     if (useTriggerInfo_)
       return false; // RETURN if trigger is missing
   }
   if (passTrigger == 0 && useTriggerInfo_) {
     //std::cout<<"HLT path "<<hltpath_<<" does not fire in this event"
     //     <<std::endl;
     return false; // RETURN if event fails the trigger
   }
   Int_t numberOfHLTFilterObjects = 0;
   //
   edm::Handle<trigger::TriggerEvent> pHLT;
   iEvent.getByLabel(triggerEventTag_, pHLT);
   const Int_t nF(pHLT->sizeFilters());
   const Int_t filterInd = pHLT->filterIndex(hltpathFilter_);
   if (nF != filterInd) {
     const trigger::Vids& VIDS (pHLT->filterIds(filterInd));
     const trigger::Keys& KEYS(pHLT->filterKeys(filterInd));
     const Int_t nI(VIDS.size());
     const Int_t nK(KEYS.size());
     numberOfHLTFilterObjects = (nI>nK)? nI:nK;
   }
   else {
     //std::cout << "HLT Filter " << hltpathFilter_.label() 
     //	       << " was not found in this event..." << std::endl;
     if (useTriggerInfo_)
       return false; // RETURN if event fails the trigger
   }
   const trigger::TriggerObjectCollection& TOC(pHLT->getObjects());
   // *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
   // ET CUT: at least one electron in the event with ET>ETCut_-*-*-*-*-*
   // *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
   // electron collection
   edm::Handle<pat::ElectronCollection> patElectron;
   iEvent.getByLabel(electronCollectionTag_, patElectron);
   if ( ! patElectron.isValid()) {
     //std::cout << "No electrons found in this event with tag " 
     //	       << electronCollectionTag_  << std::endl;
     return false; // RETURN if no elecs in the event
   }
   const pat::ElectronCollection *pElecs = patElectron.product();
   // MET collection
   edm::Handle<pat::METCollection> patMET;
   iEvent.getByLabel(metCollectionTag_,  patMET);
   //
   // Note: best to do Duplicate removal here, since the current
   // implementation does not remove triplicates
   // duplicate removal is on at PAT, but does it remove triplicates?
   //
   //  ------------------------------------------------------------------
   //  Order your electrons: first the ones with the higher ET
   //  ------------------------------------------------------------------
   pat::ElectronCollection::const_iterator elec;
   // check how many electrons there are in the event
   const Int_t Nelecs = pElecs->size();
   if (Nelecs == 0) {
     //std::cout << "No electrons found in this event" << std::endl;
     return false; // RETURN if no elecs in the event
   }
   //
   Int_t  counter = 0;
   std::vector<int> indices;
   std::vector<double> ETs;
   pat::ElectronCollection myElectrons;
   for (elec = pElecs->begin(); elec != pElecs->end(); ++elec) {
     if (isInFiducial(elec->caloPosition().eta())) {
       Double_t sc_et = elec->caloEnergy()/cosh(elec->caloPosition().eta());
       indices.push_back(counter); ETs.push_back(sc_et);
       myElectrons.push_back(*elec);
       ++counter;
     }
   }
   const Int_t  event_elec_number = (int) indices.size();
   if (event_elec_number == 0) {
     //std::cout << "No electrons in fiducial were found" << std::endl;
     return false; // RETURN if none of the electron in fiducial
   }
   // be careful now: you allocate some memory with new ...................
   // so whenever you return you must release this memory .................
   Int_t *sorted = new int[event_elec_number];
   Double_t *et = new double[event_elec_number];
   for (Int_t i=0; i<event_elec_number; ++i) {
     et[i] = ETs[i];
   }
   // array sorted now has the indices of the highest ET electrons
   TMath::Sort(event_elec_number, et, sorted, true);
   //
   // if the highest electron in the event has ET < ETCut_ return
   Int_t max_et_index = sorted[0];
   if (ETs[max_et_index] < ETCut_) {
     //std::cout << "Highest ET electron is " << ETs[max_et_index]<<std::endl;
     delete [] sorted;  delete [] et;
     return false; // RETURN if the highest ET elec has ET< ETcut
   }
   //for (Int_t i=0; i< event_elec_number; ++i) {
   //  cout << "elec #"<<i << " " << myElectrons[ sorted[i] ].et() << endl;
   //}
   //
   //
   // now search for a second Gsf electron with ET > ETCut2ndEle_
   if (event_elec_number>=2 && vetoSecondElectronEvents_) {
     pat::Electron secElec = myElectrons[ sorted[1] ];
     //std::cout << "Second Electron with ET=" << ETs[ sorted[1] ] << " and ID: " << passEleIDCuts(&secElec)  <<std::endl;
     if (ETs[ sorted[1] ] > ETCut2ndEle_ && passEleIDCuts(&secElec)) {
       delete [] sorted;  delete [] et;
       return false;  // RETURN if you have more that 1 electron with ET>cut
     }
   }
   //
   // get the most high-ET electron:
   pat::Electron maxETelec = myElectrons[max_et_index];
   //std::cout << "** selected ele phi: " << maxETelec.phi()
   //	     << ", eta=" << maxETelec.eta() << ", sihih="
   //	     << maxETelec.scSigmaIEtaIEta() << ", hoe=" 
   //	     << maxETelec.hadronicOverEm() << ", trackIso: " 
   //	     << maxETelec.trackIso() << ", ecalIso: " << maxETelec.ecalIso()
   //	     << ", hcalIso: " << maxETelec.hcalIso()
   //	     << std::endl;
   //
   // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
   // special pre-selection requirements ^^^
   // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
   // hit pattern and conversion rejection
   if (useValidFirstPXBHit_ || calculateValidFirstPXBHit_) {
     Bool_t fail = 
       not maxETelec.gsfTrack()->hitPattern().hasValidHitInFirstPixelBarrel();
    if(useValidFirstPXBHit_ && fail) 
      {
	delete [] sorted;  delete [] et;
	//std::cout << "Filter: there is no valid hit in 1st layer PXB" << std::endl;
	return false;
      }
    if (calculateValidFirstPXBHit_) {
      std::string vfpx("PassValidFirstPXBHit");
      if (fail)
	maxETelec.addUserInt(vfpx,0);
      else
      	maxETelec.addUserInt(vfpx,1);
    }
   }
   if (useExpectedMissingHits_ || calculateExpectedMissingHits_) {
     Int_t numberOfInnerHits = (Int_t) maxETelec.gsfTrack()->trackerExpectedHitsInner().numberOfHits();
     if (numberOfInnerHits > maxNumberOfExpectedMissingHits_
	 && useExpectedMissingHits_) {
       delete [] sorted;  delete [] et;
       return false;
     }
     if (calculateExpectedMissingHits_) {
       maxETelec.addUserInt("NumberOfExpectedMissingHits",numberOfInnerHits);
     }
   }
   if (useConversionRejection_ || calculateConversionRejection_) {
     // use of conversion rejection as it is implemented in egamma
     // you have to get the general track collection to do that
     // WARNING! you have to supply the correct B-field in Tesla
     // the magnetic field
     Double_t bfield;
     if (dataMagneticFieldSetUp_) {
       edm::Handle<DcsStatusCollection> dcsHandle;
       iEvent.getByLabel(dcsTag_, dcsHandle);
       // scale factor = 3.801/18166.0 which are
       // average values taken over a stable two
       // week period
       Double_t currentToBFieldScaleFactor = 2.09237036221512717e-04;
       Double_t current = (*dcsHandle)[0].magnetCurrent();
       bfield = current*currentToBFieldScaleFactor;
     } else {
       edm::ESHandle<MagneticField> magneticField;
       iSetup.get<IdealMagneticFieldRecord>().get(magneticField);
       const  MagneticField *mField = magneticField.product();
       bfield = mField->inTesla(GlobalPoint(0.,0.,0.)).z();
     }
     edm::Handle<reco::TrackCollection> ctfTracks;
     if ( iEvent.getByLabel("generalTracks", ctfTracks) ) {
       ConversionFinder cf;
       //Double_t bfield = mField->inTesla(GlobalPoint(0.,0.,0.)).z();
       Bool_t isConv = cf.isElFromConversion(maxETelec, ctfTracks, bfield, dist_,dcot_);
       //std::cout << "Filter: for this elec the conversion says " << isConv << std::endl;
       if (isConv && useConversionRejection_) {
       	 delete [] sorted;  delete [] et;
       	 return false;	 
       }
       if (calculateConversionRejection_) {
	 if (isConv) 
	   maxETelec.addUserInt("PassConversionRejection",0);
	 else
	   maxETelec.addUserInt("PassConversionRejection",1);
       }
     } else {
       std::cout << "WARNING! Track Collection with input name: generalTracks" 
		 << " was not found. Conversion Rejection is not going to be"
		 << " applied!!!" << std::endl;
     }

   }
   //
   if (electronMatched2HLT_ && useTriggerInfo_) {
     Int_t trigger_int_probe = 0;
     if (nF != filterInd) {
       const trigger::Keys& KEYS(pHLT->filterKeys(filterInd));
       const Int_t nK(KEYS.size());
       //std::cout << "Found trig objects #" << nK << std::endl;
       for (Int_t iTrig = 0;iTrig <nK; ++iTrig ) {
	 const trigger::TriggerObject& TO(TOC[KEYS[iTrig]]);
	 Double_t dr_ele_HLT = 
	   reco::deltaR(maxETelec.eta(),maxETelec.phi(),TO.eta(),TO.phi());
	 //std::cout << "-->found dr=" << dr_ele_HLT << std::endl;
	 if (fabs(dr_ele_HLT) < electronMatched2HLT_DR_) {
	   ++trigger_int_probe; break;}
	 //}
       }
       if (trigger_int_probe == 0) {
	 delete [] sorted;  delete [] et;
	 //std::cout << "Electron could not be matched to an HLT object with "
	 //   << std::endl;
	 return false; // RETURN: electron is not matched to an HLT object
       }
     }
     else {
       delete [] sorted;  delete [] et;
       //std::cout << "Electron filter not found - should not be like that... "
       // << std::endl;
       return false; // RETURN: electron is not matched to an HLT object
     }
   }
   //
   // get the met now:
   const pat::METCollection *pMet = patMET.product();
   const pat::METCollection::const_iterator met = pMet->begin();
   const pat::MET theMET = *met;
   Double_t metEt = met->et();
   //Double_t metEta = met->eta();
   //Double_t metMt = met->mt();
   //Double_t metPhi = met->phi();
   //Double_t metSig = met->mEtSig();
   //std::cout<<"met properties: et=" << met->et() << ", eta: " <<  met->eta()
   //	     << std::endl;
   // 
   if (metEt < METCut_) {
     delete [] sorted;  delete [] et;
     //std::cout << "MET is " << metEt << std::endl;
     return false;  // RETURN if MET is < Metcut
   }
   //
   //
   // now we have a W candidate ...........................................
   pat::CompositeCandidate wenu;
   wenu.addDaughter(maxETelec, "electron");
   wenu.addDaughter(theMET, "met");

   auto_ptr<pat::CompositeCandidateCollection> 
     selectedWenuCandidates(new pat::CompositeCandidateCollection);
   selectedWenuCandidates->push_back(wenu);
   //
   iEvent.put( selectedWenuCandidates, "selectedWenuCandidates");

   //
   // release your memory
   delete [] sorted;  delete [] et;
   //
   //std::cout << "Filter accepts this event" << std::endl;
   //
   return true;

}

// ------------ method called once each job just after ending the event loop  -
void 
WenuCandidateFilter::endJob() {
}

Bool_t WenuCandidateFilter::isInFiducial(Double_t eta)
{
  if (fabs(eta) < BarrelMaxEta_) return true;
  else if (fabs(eta) < EndCapMaxEta_ && fabs(eta) > EndCapMinEta_)
    return true;
  return false;

}

Bool_t WenuCandidateFilter::passEleIDCuts(pat::Electron *ele)
{
  if (not useVetoSecondElectronID_)  return true;
  if (not ele->isElectronIDAvailable(vetoSecondElectronIDType_)) {
    std::cout << "WenuCandidateFilter: request ignored: 2nd electron ID type "
	      << "not found in electron object" << std::endl;
    return true;
  }
  if (vetoSecondElectronIDSign_ == ">") {
    if (ele->electronID(vetoSecondElectronIDType_)>vetoSecondElectronIDValue_)
      return true;
    else return false;
  }
  else if (vetoSecondElectronIDSign_ == "<") {
    if (ele->electronID(vetoSecondElectronIDType_)<vetoSecondElectronIDValue_)
      return true;
    else return false;
  }
  else {
    if (fabs(ele->electronID(vetoSecondElectronIDType_)-
	     vetoSecondElectronIDValue_) < 0.1)
      return true;
    else return false;    
  }
}

//define this as a plug-in
DEFINE_FWK_MODULE(WenuCandidateFilter);
