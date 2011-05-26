#ifndef TtSemiLepHitFitProducer_h
#define TtSemiLepHitFitProducer_h

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "PhysicsTools/JetMCUtils/interface/combination.h"
#include "AnalysisDataFormats/TopObjects/interface/TtSemiLepEvtPartons.h"
#include "DataFormats/PatCandidates/interface/Lepton.h"
#include "AnalysisDataFormats/TopObjects/interface/TtSemiEvtSolution.h"

#include "TopQuarkAnalysis/TopHitFit/interface/RunHitFit.h"
#include "TopQuarkAnalysis/TopHitFit/interface/Top_Decaykin.h"
#include "TopQuarkAnalysis/TopHitFit/interface/LeptonTranslatorBase.h"
#include "TopQuarkAnalysis/TopHitFit/interface/JetTranslatorBase.h"
#include "TopQuarkAnalysis/TopHitFit/interface/METTranslatorBase.h"

template <typename LeptonCollection>
class TtSemiLepHitFitProducer : public edm::EDProducer {
  
 public:
  
  explicit TtSemiLepHitFitProducer(const edm::ParameterSet&);
  ~TtSemiLepHitFitProducer();
  
 private:
  // produce
  virtual void produce(edm::Event&, const edm::EventSetup&);

  edm::InputTag jets_;
  edm::InputTag leps_;
  edm::InputTag mets_;
  
  /// maximal number of jets (-1 possible to indicate 'all')
  int maxNJets_;
  /// maximal number of combinations to be written to the event
  int maxNComb_;

  /// maximal number of iterations to be performed for the fit
  unsigned int maxNrIter_;
  /// maximal chi2 equivalent
  double maxDeltaS_;
  /// maximal deviation for contstraints
  double maxF_;
  unsigned int jetParam_;
  unsigned int lepParam_;
  unsigned int metParam_;
  /// constrains
  std::vector<unsigned> constraints_;
  double mW_;
  double mTop_;
  /// smear factor for jet resolutions
  double jetEnergyResolutionSmearFactor_;

  struct HitFitResult {
    int Status;
    double Chi2;
    double Prob;
    double MT;
    double SigMT;
    pat::Particle HadB;
    pat::Particle HadP;
    pat::Particle HadQ;
    pat::Particle LepB;
    pat::Particle LepL;
    pat::Particle LepN;
    std::vector<int> JetCombi;
    bool operator< (const HitFitResult& rhs) { return Chi2 < rhs.Chi2; };
  };
  
  typedef hitfit::RunHitFit<pat::Electron,pat::Muon,pat::Jet,pat::MET> PatHitFit;
  
  edm::FileInPath hitfitDefault_;
  edm::FileInPath hitfitElectronResolution_;
  edm::FileInPath hitfitMuonResolution_;
  edm::FileInPath hitfitUdscJetResolution_;
  edm::FileInPath hitfitBJetResolution_;
  edm::FileInPath hitfitMETResolution_;
  
  double      hitfitLepWMass_;
  double      hitfitHadWMass_;
  double      hitfitTopMass_;
  
  hitfit::LeptonTranslatorBase<pat::Electron> electronTranslator_;
  hitfit::LeptonTranslatorBase<pat::Muon>     muonTranslator_;
  hitfit::JetTranslatorBase<pat::Jet>         jetTranslator_;
  hitfit::METTranslatorBase<pat::MET>         metTranslator_;
  
  PatHitFit* HitFit;
};

template<typename LeptonCollection>
TtSemiLepHitFitProducer<LeptonCollection>::TtSemiLepHitFitProducer(const edm::ParameterSet& cfg):
  jets_                    (cfg.getParameter<edm::InputTag>("jets")),
  leps_                    (cfg.getParameter<edm::InputTag>("leps")),
  mets_                    (cfg.getParameter<edm::InputTag>("mets")),
  maxNJets_                (cfg.getParameter<int>          ("maxNJets"            )),
  maxNComb_                (cfg.getParameter<int>          ("maxNComb"            )),
  mW_                      (cfg.getParameter<double>       ("mW"                  )),
  mTop_                    (cfg.getParameter<double>       ("mTop"                )),
  
  // The following five initializers read the config parameters for the
  // ASCII text files which contains the physics object resolutions.
  hitfitDefault_           (cfg.getUntrackedParameter<edm::FileInPath>(std::string("hitfitDefault"),
                            edm::FileInPath(std::string("TopQuarkAnalysis/TopHitFit/data/setting/RunHitFitConfiguration.txt")))),
  hitfitElectronResolution_(cfg.getUntrackedParameter<edm::FileInPath>(std::string("hitfitElectronResolution"),
                            edm::FileInPath(std::string("TopQuarkAnalysis/TopHitFit/data/resolution/tqafElectronResolution.txt")))),
  hitfitMuonResolution_    (cfg.getUntrackedParameter<edm::FileInPath>(std::string("hitfitMuonResolution"),
                            edm::FileInPath(std::string("TopQuarkAnalysis/TopHitFit/data/resolution/tqafMuonResolution.txt")))),
  hitfitUdscJetResolution_ (cfg.getUntrackedParameter<edm::FileInPath>(std::string("hitfitUdscJetResolution"),
                            edm::FileInPath(std::string("TopQuarkAnalysis/TopHitFit/data/resolution/tqafUdscJetResolution.txt")))),
  hitfitBJetResolution_    (cfg.getUntrackedParameter<edm::FileInPath>(std::string("hitfitBJetResolution"),
                            edm::FileInPath(std::string("TopQuarkAnalysis/TopHitFit/data/resolution/tqafBJetResolution.txt")))),
  hitfitMETResolution_     (cfg.getUntrackedParameter<edm::FileInPath>(std::string("hitfitElectronResolution"),
                            edm::FileInPath(std::string("TopQuarkAnalysis/TopHitFit/data/resolution/tqafKtResolution.txt")))),

  // The following three initializers instantiate the translator between PAT objects
  // and HitFit objects using the ASCII text files which contains the resolutions.
  electronTranslator_(hitfitElectronResolution_.fullPath()),
  muonTranslator_    (hitfitMuonResolution_.fullPath()),
  jetTranslator_     (hitfitUdscJetResolution_.fullPath(), hitfitBJetResolution_.fullPath()),
  metTranslator_     (hitfitMETResolution_.fullPath())

{
  // Create an instance of RunHitFit and initialize it.
  HitFit = new PatHitFit(electronTranslator_,
                         muonTranslator_,
                         jetTranslator_,
                         metTranslator_,
                         hitfitDefault_.fullPath(),
                         mW_,
                         mW_,
                         mTop_);

  produces< std::vector<pat::Particle> >("PartonsHadP");
  produces< std::vector<pat::Particle> >("PartonsHadQ");
  produces< std::vector<pat::Particle> >("PartonsHadB");
  produces< std::vector<pat::Particle> >("PartonsLepB");
  produces< std::vector<pat::Particle> >("Leptons");
  produces< std::vector<pat::Particle> >("Neutrinos");

  produces< std::vector<std::vector<int> > >();
  produces< std::vector<double> >("Chi2");
  produces< std::vector<double> >("Prob");
  produces< std::vector<double> >("MT");
  produces< std::vector<double> >("SigMT");
  produces< std::vector<int> >("Status");
}

template<typename LeptonCollection>
TtSemiLepHitFitProducer<LeptonCollection>::~TtSemiLepHitFitProducer()
{
  delete HitFit;
}

template<typename LeptonCollection>
void TtSemiLepHitFitProducer<LeptonCollection>::produce(edm::Event& evt, const edm::EventSetup& setup)
{
  std::auto_ptr< std::vector<pat::Particle> > pPartonsHadP( new std::vector<pat::Particle> );
  std::auto_ptr< std::vector<pat::Particle> > pPartonsHadQ( new std::vector<pat::Particle> );
  std::auto_ptr< std::vector<pat::Particle> > pPartonsHadB( new std::vector<pat::Particle> );
  std::auto_ptr< std::vector<pat::Particle> > pPartonsLepB( new std::vector<pat::Particle> );
  std::auto_ptr< std::vector<pat::Particle> > pLeptons    ( new std::vector<pat::Particle> );
  std::auto_ptr< std::vector<pat::Particle> > pNeutrinos  ( new std::vector<pat::Particle> );

  std::auto_ptr< std::vector<std::vector<int> > > pCombi ( new std::vector<std::vector<int> > );
  std::auto_ptr< std::vector<double>            > pChi2  ( new std::vector<double> );
  std::auto_ptr< std::vector<double>            > pProb  ( new std::vector<double> );
  std::auto_ptr< std::vector<double>            > pMT    ( new std::vector<double> );
  std::auto_ptr< std::vector<double>            > pSigMT ( new std::vector<double> );
  std::auto_ptr< std::vector<int>               > pStatus( new std::vector<int> );

  edm::Handle<std::vector<pat::Jet> > jets;
  evt.getByLabel(jets_, jets);

  edm::Handle<std::vector<pat::MET> > mets;
  evt.getByLabel(mets_, mets);

  edm::Handle<LeptonCollection> leps;
  evt.getByLabel(leps_, leps);

  unsigned int nPartons = 4;

  // -----------------------------------------------------
  // skip events with no appropriate lepton candidate in
  // or empty MET or less jets than partons
  // -----------------------------------------------------

  if( leps->empty() || mets->empty() || jets->size()<nPartons ) {
    // the kinFit getters return empty objects here
    pPartonsHadP->push_back( pat::Particle() );
    pPartonsHadQ->push_back( pat::Particle() );
    pPartonsHadB->push_back( pat::Particle() );
    pPartonsLepB->push_back( pat::Particle() );
    pLeptons    ->push_back( pat::Particle() );
    pNeutrinos  ->push_back( pat::Particle() );
    // indices referring to the jet combination
    std::vector<int> invalidCombi;
    for(unsigned int i = 0; i < nPartons; ++i) 
      invalidCombi.push_back( -1 );
    pCombi->push_back( invalidCombi );
    // chi2
    pChi2->push_back( -1. );
    // chi2 probability
    pProb->push_back( -1. );
    // fitted top mass
    pMT->push_back( -1. );
    pSigMT->push_back( -1. );
    // status of the fitter
    pStatus->push_back( -1 );
    // feed out all products
    evt.put(pCombi);
    evt.put(pPartonsHadP, "PartonsHadP");
    evt.put(pPartonsHadQ, "PartonsHadQ");
    evt.put(pPartonsHadB, "PartonsHadB");
    evt.put(pPartonsLepB, "PartonsLepB");
    evt.put(pLeptons    , "Leptons"    );
    evt.put(pNeutrinos  , "Neutrinos"  );
    evt.put(pChi2       , "Chi2"       );
    evt.put(pProb       , "Prob"       );
    evt.put(pMT         , "MT"         );
    evt.put(pSigMT      , "SigMT"      );
    evt.put(pStatus     , "Status"     );
    return;
  }

  std::list<HitFitResult> HitFitResultList;
  
  // Boolean flag for
  // true  = ejets
  // false = mujets
  bool channel = false;
  
  // Clear the internal state
  HitFit->clear();
  
  // Add the lepton into HitFit
  if (channel) {
      HitFit->AddLepton((*leps)[0]);
  } else {
      HitFit->AddLepton((*leps)[0]);
  }

  // Add jets into HitFit
  for (int jet = 0 ; jet != maxNJets_ ; ++jet) {
      HitFit->AddJet((*jets)[jet]);
  }

  // Add missing transverse energy into HitFit
  HitFit->SetMet((*mets)[0]);
  
  //
  // BEGIN DECLARATION OF VARIABLES FROM KINEMATIC FIT
  //

  // In this part are variables from the
  // kinematic fit procedure

  // Number of all permutation of the event
  size_t nHitFit    = 0 ;

  // Number of jets in the event
  size_t nHitFitJet = 0 ;

  // Results of the fit for all jet permutation of the event
  std::vector<hitfit::Fit_Result>     hitfitResult;

//TODO
  // String-encoded jet permutation
  // b = leptonic b-jet
  // B = hadronic b-jet
  // w = hadronic jet from W boson decay
  std::vector<std::string>            jetPermutation;

  // mass of the top quark after fit
  std::vector<double> fittedTopMass;

  // uncertainty in the mass of the top quark after fit
  std::vector<double> fittedTopMassSigma;

  // the chi-square of the fit
  // a negative value means the fit of a particular permutation did not converge
  std::vector<double> fitChi2;

  // boolean flag if the fit of a particular permutation converges or not
  std::vector<bool>   fitConverge;

  // normalized weight of a particular permutation
  std::vector<double> fitWeight;

  //
  // R U N   H I T F I T
  //
  // Run the kinematic fit and get how many permutations is possible
  // in the fit
  
  nHitFit         = HitFit->FitAllPermutation();
  
  //
  // BEGIN PART WHICH EXTRACT INFORMATION FROM HITFIT
  //

  // Get the number of jets
  nHitFitJet      = HitFit->GetUnfittedEvent()[0].njets();

  // Get the fit results for all permutation
  hitfitResult    = HitFit->GetFitAllPermutation();

  // Loop over all permutation and extract the information
  for (size_t fit = 0 ; fit != nHitFit ; ++fit) {

      // Get the event after the fit
      hitfit::Lepjets_Event fittedEvent   = hitfitResult[fit].ev();
      
      std::vector<int> hitcombi(4);
      /*
        11 : leptonic b
        12 : hadronic b
        13 : hadronic W
        14 : hadronic W
      */
      for (size_t jet = 0 ; jet != nHitFitJet ; ++jet) {
          int jet_type = fittedEvent.jet(jet).type();
          
          switch(jet_type) {
            case 11: hitcombi[TtSemiLepEvtPartons::LepB     ] = jet;
              break;
            case 12: hitcombi[TtSemiLepEvtPartons::HadB     ] = jet;
              break;
            case 13: hitcombi[TtSemiLepEvtPartons::LightQ   ] = jet;
              break;
            case 14: hitcombi[TtSemiLepEvtPartons::LightQBar] = jet;
              break;
          }
      }

      // Store the kinematic quantities in the corresponding containers.

      fittedTopMass.push_back(hitfitResult[fit].mt());
      fittedTopMassSigma.push_back(hitfitResult[fit].sigmt());
      fitChi2.push_back(hitfitResult[fit].chisq());
      fitConverge.push_back(hitfitResult[fit].chisq() > 0);
      if (fitConverge.back()) {
          fitWeight.push_back(exp(-1.0*(fitChi2.back())/2.0));
      } else {
          fitWeight.push_back(0.0);
      }

      jetPermutation.push_back(fittedEvent.jet_permutation());
      
      hitfit::Lepjets_Event_Jet hadP_ = fittedEvent.jet(hitcombi[TtSemiLepEvtPartons::LightQ   ]);
      hitfit::Lepjets_Event_Jet hadQ_ = fittedEvent.jet(hitcombi[TtSemiLepEvtPartons::LightQBar]);
      hitfit::Lepjets_Event_Jet hadB_ = fittedEvent.jet(hitcombi[TtSemiLepEvtPartons::HadB     ]);
      hitfit::Lepjets_Event_Jet lepB_ = fittedEvent.jet(hitcombi[TtSemiLepEvtPartons::LepB     ]);
      
      hitfit::Lepjets_Event_Lep lepL_ = fittedEvent.lep(0);
      
      if (hitfitResult[fit].chisq() > 0) { // only take into account converged fits
        HitFitResult hitresult;
        hitresult.Status = 0;
        hitresult.Chi2 = hitfitResult[fit].chisq();
        hitresult.Prob = exp(-1.0*(hitfitResult[fit].chisq())/2.0);
        hitresult.MT   = hitfitResult[fit].mt();
        hitresult.SigMT= hitfitResult[fit].sigmt();
        hitresult.HadB = pat::Particle(reco::LeafCandidate(0,
                                       math::XYZTLorentzVector(hadB_.p().x(), hadB_.p().y(),
                                       hadB_.p().z(), hadB_.p().t()), math::XYZPoint()));
        hitresult.HadP = pat::Particle(reco::LeafCandidate(0,
                                       math::XYZTLorentzVector(hadP_.p().x(), hadP_.p().y(),
                                       hadP_.p().z(), hadP_.p().t()), math::XYZPoint()));
        hitresult.HadQ = pat::Particle(reco::LeafCandidate(0,
                                       math::XYZTLorentzVector(hadQ_.p().x(), hadQ_.p().y(),
                                       hadQ_.p().z(), hadQ_.p().t()), math::XYZPoint()));
        hitresult.LepB = pat::Particle(reco::LeafCandidate(0,
                                       math::XYZTLorentzVector(lepB_.p().x(), lepB_.p().y(),
                                       lepB_.p().z(), lepB_.p().t()), math::XYZPoint()));
        hitresult.LepL = pat::Particle(reco::LeafCandidate(0,
                                       math::XYZTLorentzVector(lepL_.p().x(), lepL_.p().y(),
                                       lepL_.p().z(), lepL_.p().t()), math::XYZPoint()));
        hitresult.LepN = pat::Particle(reco::LeafCandidate(0,
                                       math::XYZTLorentzVector(fittedEvent.met().x(), fittedEvent.met().y(),
                                       fittedEvent.met().z(), fittedEvent.met().t()), math::XYZPoint()));
        hitresult.JetCombi = hitcombi;

        HitFitResultList.push_back(hitresult);
      }

  }

  // sort results w.r.t. chi2 values
  HitFitResultList.sort();
  
  // -----------------------------------------------------
  // feed out result
  // starting with the JetComb having the smallest chi2
  // -----------------------------------------------------

  if( HitFitResultList.size() < 1 ) { // in case no fit results were stored in the list (all fits aborted)
    pPartonsHadP->push_back( pat::Particle() );
    pPartonsHadQ->push_back( pat::Particle() );
    pPartonsHadB->push_back( pat::Particle() );
    pPartonsLepB->push_back( pat::Particle() );
    pLeptons    ->push_back( pat::Particle() );
    pNeutrinos  ->push_back( pat::Particle() );
    // indices referring to the jet combination
    std::vector<int> invalidCombi;
    for(unsigned int i = 0; i < nPartons; ++i) 
      invalidCombi.push_back( -1 );
    pCombi->push_back( invalidCombi );
    // chi2
    pChi2->push_back( -1. );
    // chi2 probability
    pProb->push_back( -1. );
    // fitted top mass
    pMT->push_back( -1. );
    pSigMT->push_back( -1. );
    // status of the fitter
    pStatus->push_back( -1 );
  }
  else {
    unsigned int iComb = 0;
    for(typename std::list<HitFitResult>::const_iterator result = HitFitResultList.begin(); result != HitFitResultList.end(); ++result) {
      if(maxNComb_ >= 1 && iComb == (unsigned int) maxNComb_) break;
      iComb++;
      // partons
      pPartonsHadP->push_back( result->HadP );
      pPartonsHadQ->push_back( result->HadQ );
      pPartonsHadB->push_back( result->HadB );
      pPartonsLepB->push_back( result->LepB );
      // lepton
      pLeptons->push_back( result->LepL );
      // neutrino
      pNeutrinos->push_back( result->LepN );
      // indices referring to the jet combination
      pCombi->push_back( result->JetCombi );
      // chi2
      pChi2->push_back( result->Chi2 );
      // chi2 probability
      pProb->push_back( result->Prob );
      // fitted top mass
      pMT->push_back( result->MT );
      pSigMT->push_back( result->SigMT );
      // status of the fitter
      pStatus->push_back( result->Status );
    }
  }
  evt.put(pCombi);
  evt.put(pPartonsHadP, "PartonsHadP");
  evt.put(pPartonsHadQ, "PartonsHadQ");
  evt.put(pPartonsHadB, "PartonsHadB");
  evt.put(pPartonsLepB, "PartonsLepB");
  evt.put(pLeptons    , "Leptons"    );
  evt.put(pNeutrinos  , "Neutrinos"  );
  evt.put(pChi2       , "Chi2"       );
  evt.put(pProb       , "Prob"       );
  evt.put(pMT         , "MT"         );
  evt.put(pSigMT      , "SigMT"      );
  evt.put(pStatus     , "Status"     );
}

#endif
