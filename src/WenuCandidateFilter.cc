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

//
// class declaration
//

class WenuCandidateFilter : public edm::EDFilter {
   public:
      explicit WenuCandidateFilter(const edm::ParameterSet&);
      ~WenuCandidateFilter();

   private:
      virtual bool filter(edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;
      bool isInFiducial(double eta);
      
      // ----------member data ---------------------------
  double ETCut_;
  double METCut_;
  double ETCut2ndEle_;
  edm::InputTag triggerCollectionTag_;
  edm::InputTag triggerEventTag_;
  std::string hltpath_;
  edm::InputTag hltpathFilter_;
  edm::InputTag electronCollectionTag_;
  edm::InputTag metCollectionTag_;

  double BarrelMaxEta_;
  double EndCapMaxEta_;
  double EndCapMinEta_;
  bool useTriggerInfo_;
  bool electronMatched2HLT_;
  double electronMatched2HLT_DR_;
  bool vetoSecondElectronEvents_;
  bool useValidFirstPXBHit_;
  bool calculateValidFirstPXBHit_;
  bool useConversionRejection_;
  bool calculateConversionRejection_;
  bool useExpectedMissingHits_;
  bool calculateExpectedMissingHits_;
  int  maxNumberOfExpectedMissingHits_;
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
  double ETCut_D = 30.;
  ETCut_ = iConfig.getUntrackedParameter<double>("ETCut",ETCut_D);
  double METCut_D = 0.;
  METCut_ = iConfig.getUntrackedParameter<double>("METCut",METCut_D);
  double ETCut2ndEle_D = 20.;
  ETCut2ndEle_ = iConfig.getUntrackedParameter<double>("ETCut2ndEle",
						       ETCut2ndEle_D);
  vetoSecondElectronEvents_ = iConfig.getUntrackedParameter<bool>("vetoSecondElectronEvents",false);
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
  useExpectedMissingHits_ = 
    iConfig.getUntrackedParameter<Bool_t>("useExpectedMissingHits",false);
  calculateExpectedMissingHits_ = 
    iConfig.getUntrackedParameter<Bool_t>("calculateExpectedMissingHits",false);
  maxNumberOfExpectedMissingHits_ = 
    iConfig.getUntrackedParameter<int>("maxNumberOfExpectedMissingHits",1);
  //
  //
  //
  double BarrelMaxEta_D = 1.4442;
  double EndCapMinEta_D = 1.56;
  double EndCapMaxEta_D = 2.5;
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
   int passTrigger = 0;  
   if (HLTResults.isValid()) {
     const edm::TriggerNames & triggerNames = iEvent.triggerNames(*HLTResults);
     unsigned int trigger_size = HLTResults->size();
     unsigned int trigger_position = triggerNames.triggerIndex(hltpath_);
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
   int numberOfHLTFilterObjects = 0;
   //
   edm::Handle<trigger::TriggerEvent> pHLT;
   iEvent.getByLabel(triggerEventTag_, pHLT);
   const int nF(pHLT->sizeFilters());
   const int filterInd = pHLT->filterIndex(hltpathFilter_);
   if (nF != filterInd) {
     const trigger::Vids& VIDS (pHLT->filterIds(filterInd));
     const trigger::Keys& KEYS(pHLT->filterKeys(filterInd));
     const int nI(VIDS.size());
     const int nK(KEYS.size());
     numberOfHLTFilterObjects = (nI>nK)? nI:nK;
   }
   else {
     //std::cout << "HLT Filter " << hltpathFilter_.label() 
     //	       << " was not found in this event..." << std::endl;
     if (useTriggerInfo_)
       return false; // RETURN if event fails the trigger
   }
   //std::cout << "==> HLT Filter " << hltpathFilter_.label() 
   //	     << " was found in this event..." << std::endl;
   // for trigger object matching   
   //const int nF(pHLT->sizeFilters());
   //const int iF = pHLT->filterIndex(hltpathFilter_);
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
   const int Nelecs = pElecs->size();
   if (Nelecs == 0) {
     //std::cout << "No electrons found in this event" << std::endl;
     return false; // RETURN if no elecs in the event
   }
   //
   int  counter = 0;
   std::vector<int> indices;
   std::vector<double> ETs;
   pat::ElectronCollection myElectrons;
   for (elec = pElecs->begin(); elec != pElecs->end(); ++elec) {
     if (isInFiducial(elec->caloPosition().eta())) {
       double sc_et = elec->caloEnergy()/cosh(elec->caloPosition().eta());
       indices.push_back(counter); ETs.push_back(sc_et);
       myElectrons.push_back(*elec);
       ++counter;
     }
   }
   const int  event_elec_number = (int) indices.size();
   if (event_elec_number == 0) {
     //std::cout << "No electrons in fiducial were found" << std::endl;
     return false; // RETURN if none of the electron in fiducial
   }
   // be careful now: you allocate some memory with new ...................
   // so whenever you return you must release this memory .................
   int *sorted = new int[event_elec_number];
   double *et = new double[event_elec_number];
   for (int i=0; i<event_elec_number; ++i) {
     et[i] = ETs[i];
   }
   // array sorted now has the indices of the highest ET electrons
   TMath::Sort(event_elec_number, et, sorted, true);
   //
   // if the highest electron in the event has ET < ETCut_ return
   int max_et_index = sorted[0];
   if (ETs[max_et_index] < ETCut_) {
     //std::cout << "Highest ET electron is " << ETs[max_et_index]<<std::endl;
     delete [] sorted;  delete [] et;
     return false; // RETURN if the highest ET elec has ET< ETcut
   }
   //for (int i=0; i< event_elec_number; ++i) {
   //  cout << "elec #"<<i << " " << myElectrons[ sorted[i] ].et() << endl;
   //}
   //
   //
   // now search for a second Gsf electron with ET > ETCut2ndEle_
   if (event_elec_number>=2 && vetoSecondElectronEvents_) {
     if (ETs[ sorted[1] ] > ETCut2ndEle_) {
       //std::cout<<"Second Electron with ET=" << ETs[ sorted[1] ]<<std::endl;
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
     bool fail = 
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
     int numberOfLostInnerHits = (int) maxETelec.gsfTrack()->trackerExpectedHitsInner().numberOfLostHits();
     if (numberOfLostInnerHits > maxNumberOfExpectedMissingHits_
	 && useExpectedMissingHits_) {
       delete [] sorted;  delete [] et;
       return false;
     }
     if (calculateExpectedMissingHits_) {
       maxETelec.addUserInt("NumberOfExpectedMissingHits",numberOfLostInnerHits);
     }
   }
   if (useConversionRejection_ || calculateConversionRejection_) {
     // use of conversion rejection as it is implemented in egamma
     // you have to get the general track collection to do that
     // WARNING! you have to supply the correct B-field in Tesla
     // the magnetic field
     edm::ESHandle<MagneticField> magneticField;
     iSetup.get<IdealMagneticFieldRecord>().get(magneticField);
     const  MagneticField *mField = magneticField.product();
     edm::Handle<reco::TrackCollection> ctfTracks;
     if ( iEvent.getByLabel("generalTracks", ctfTracks) ) {
       ConversionFinder cf;
       const math::XYZPoint tpoint = maxETelec.gsfTrack()->referencePoint();
       const GlobalPoint gp(tpoint.x(), tpoint.y(), tpoint.z());
       double bfield = mField->inTesla(gp).mag();  
       bool isConv = cf.isElFromConversion(maxETelec, ctfTracks, bfield);
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
   // these 3 objects have been declared outside the loop
   //const int nF(pHLT->sizeFilters());
   //const int iF = pHLT->filterIndex(hltpathFilter_);
   //const trigger::TriggerObjectCollection& TOC(pHLT->getObjects());
   //
   if (electronMatched2HLT_ && useTriggerInfo_) {
     int trigger_int_probe = 0;
     if (nF != filterInd) {
       const trigger::Keys& KEYS(pHLT->filterKeys(filterInd));
       const int nK(KEYS.size());
       //std::cout << "Found trig objects #" << nK << std::endl;
       for (int iTrig = 0;iTrig <nK; ++iTrig ) {
	 const trigger::TriggerObject& TO(TOC[KEYS[iTrig]]);
	 //std::cout << "..TO#" << iTrig << ", phi= "  << TO.phi()
	 //	   << ", eta=" << TO.eta() << ", id: " << TO.id()
	 //	   << std::endl;
	 // it has to be checked!! why some times TO.id()==0
	 // should not be like that ?!?!
	 //if (abs(TO.id())==11 || TO.id()==0) { // demand it to be an electron
	 // no demand for an electron in the HLT object
	 double dr_ele_HLT = 
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
   // demand that this electron is matched to an HLT object
   /* //custom PAT way: this may need some extra tags for TAG
      // for this reason I have set up the default to be 
      // without PAT (see previous lines)
   const pat::TriggerObjectStandAloneCollection  trigFilter = 
     maxETelec.triggerObjectMatchesByFilter(hltpathFilter_.label());
   if ((int) trigFilter.size()==0) {
     delete [] sorted; delete [] et;
     std::cout << "Electron is not an HLT object" << std::endl;
     return false; // RETURN if elec fails the trigger
   }
   */
   /*
   // this is how to access the trigger info related to that electron in pat
   vector<pat::TriggerPrimitive> trigPrim = maxETelec.triggerMatches();
   for (int i=0; i< (int) trigPrim.size(); ++i) {
     cout << trigPrim[i].filterName() << ", type: " 
	  << trigPrim[i].triggerObjectType() << ", id: " 
	  << trigPrim[i].triggerObjectId() << endl;
   }
   vector<pat::TriggerPrimitive> trigFilter = 
     maxETelec.triggerMatchesByFilter(hltpathFilter_.label());
   cout << "Test " << hltpathFilter_ << endl;
   for (int i=0; i< (int) trigFilter.size(); ++i) {
     cout << trigFilter[i].filterName() << ", type: " 
	  << trigFilter[i].triggerObjectType() << ", id: " 
	  << trigFilter[i].triggerObjectId() << endl;
   }
   */

   // get the met now:
   const pat::METCollection *pMet = patMET.product();
   const pat::METCollection::const_iterator met = pMet->begin();
   const pat::MET theMET = *met;
   double metEt = met->et();
   //double metEta = met->eta();
   //double metMt = met->mt();
   //double metPhi = met->phi();
   //double metSig = met->mEtSig();
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

bool WenuCandidateFilter::isInFiducial(double eta)
{
  if (fabs(eta) < BarrelMaxEta_) return true;
  else if (fabs(eta) < EndCapMaxEta_ && fabs(eta) > EndCapMinEta_)
    return true;
  return false;

}

//define this as a plug-in
DEFINE_FWK_MODULE(WenuCandidateFilter);
