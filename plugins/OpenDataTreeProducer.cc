

// Forked from SMPJ Analysis Framework
// https://twiki.cern.ch/twiki/bin/viewauth/CMS/SMPJAnalysisFW
// https://github.com/cms-smpj/SMPJ/tree/v1.0


#include <iostream>
#include <sstream>
#include <istream>
#include <fstream>
#include <iomanip>
#include <string>
#include <cmath>
#include <functional>
#include "TTree.h"
#include <vector>
#include <cassert>
#include <TLorentzVector.h>

#include "cms-opendata-2011-jets/AnalysisFW/plugins/OpenDataTreeProducer.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "FWCore/Common/interface/TriggerResultsByName.h"

#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/JetReco/interface/Jet.h"
#include "DataFormats/JetReco/interface/PFJet.h"
#include "DataFormats/JetReco/interface/PFJetCollection.h"
#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/JetReco/interface/GenJetCollection.h"
#include "DataFormats/JetReco/interface/JetExtendedAssociation.h"
#include "DataFormats/JetReco/interface/JetID.h"
#include "DataFormats/METReco/interface/PFMET.h"
#include "DataFormats/METReco/interface/PFMETCollection.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenRunInfoProduct.h"
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"

#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#include "JetMETCorrections/Objects/interface/JetCorrectionsRecord.h"

OpenDataTreeProducer::OpenDataTreeProducer(edm::ParameterSet const &cfg) {
  mMinPFPt           = cfg.getParameter<double>                    ("minPFPt");
  mMinJJMass         = cfg.getParameter<double>                    ("minJJMass");
  mMaxY              = cfg.getParameter<double>                    ("maxY");
  mMinNPFJets        = cfg.getParameter<int>                       ("minNPFJets");
  mPFak5JetsName     = cfg.getParameter<edm::InputTag>             ("pfak5jets");
  mPFak7JetsName     = cfg.getParameter<edm::InputTag>             ("pfak7jets");
  mOfflineVertices   = cfg.getParameter<edm::InputTag>             ("offlineVertices");
  mGoodVtxNdof       = cfg.getParameter<double>                    ("goodVtxNdof");
  mGoodVtxZ          = cfg.getParameter<double>                    ("goodVtxZ");
  mSrcPFRho          = cfg.getParameter<edm::InputTag>             ("srcPFRho");
  mPFMET             = cfg.getParameter<edm::InputTag>             ("pfmet");
  mGenJetsName       = cfg.getUntrackedParameter<edm::InputTag>    ("genjets",edm::InputTag(""));
  mPrintTriggerMenu  = cfg.getUntrackedParameter<bool>             ("printTriggerMenu",false);
  mIsMCarlo          = cfg.getUntrackedParameter<bool>             ("isMCarlo",false);
  mUseGenInfo        = cfg.getUntrackedParameter<bool>             ("useGenInfo",true);
  mMinGenPt          = cfg.getUntrackedParameter<double>           ("minGenPt",30);
  processName_       = cfg.getParameter<std::string>               ("processName");
  triggerNames_      = cfg.getParameter<std::vector<std::string> > ("triggerNames");
  triggerResultsTag_ = cfg.getParameter<edm::InputTag>             ("triggerResults");
}


void OpenDataTreeProducer::beginJob() {
    mTree = fs->make< TTree >("OpenDataTree", "OpenDataTree");

    // Variables of the flat tuple
    mTree->Branch("njet", &njet, "njet/i");
    mTree->Branch("jet_pt", jet_pt, "jet_pt[njet]/F");
    mTree->Branch("jet_eta", jet_eta, "jet_eta[njet]/F");
    mTree->Branch("jet_phi", jet_phi, "jet_phi[njet]/F");
    mTree->Branch("jet_E", jet_E, "jet_E[njet]/F");   
    mTree->Branch("jet_tightID", jet_tightID, "jet_tightID[njet]/O");
    mTree->Branch("jet_area", jet_area, "jet_area[njet]/F");
    mTree->Branch("jet_jes", jet_jes, "jet_jes[njet]/F");
    mTree->Branch("jet_igen", jet_igen, "jet_igen[njet]/I");
    mTree->Branch("jet_ID", jet_ID, "jet_ID[njet]/I");

    // AK7 variables
    mTree->Branch("njet_ak7", &njet_ak7, "njet_ak7/i");
    mTree->Branch("jet_pt_ak7", jet_pt_ak7, "jet_pt_ak7[njet_ak7]/F");
    mTree->Branch("jet_eta_ak7", jet_eta_ak7, "jet_eta_ak7[njet_ak7]/F");
    mTree->Branch("jet_phi_ak7", jet_phi_ak7, "jet_phi_ak7[njet_ak7]/F");
    mTree->Branch("jet_E_ak7", jet_E_ak7, "jet_E_ak7[njet_ak7]/F");
    mTree->Branch("jet_area_ak7", jet_area_ak7, "jet_area_ak7[njet_ak7]/F");
    mTree->Branch("jet_jes_ak7", jet_jes_ak7, "jet_jes_ak7[njet_ak7]/F");
    mTree->Branch("ak7_to_ak5", ak7_to_ak5, "ak7_to_ak5[njet_ak7]/I");

    mTree->Branch("ngen", &ngen, "ngen/i");
    mTree->Branch("gen_pt", gen_pt, "gen_pt[ngen]/F");
    mTree->Branch("gen_eta", gen_eta, "gen_eta[ngen]/F");
    mTree->Branch("gen_phi", gen_phi, "gen_phi[ngen]/F");
    mTree->Branch("gen_E", gen_E, "gen_E[ngen]/F");

    mTree->Branch("run", &run, "run/i");
    mTree->Branch("lumi", &lumi, "lumi/i");
    mTree->Branch("event", &event, "event/l");
    mTree->Branch("ntrg", &ntrg, "ntrg/i");
    mTree->Branch("triggers", triggers, "triggers[ntrg]/O");
    mTree->Branch("triggernames", &triggernames);
    mTree->Branch("prescales", prescales, "prescales[ntrg]/i");
    mTree->Branch("met", &met, "met/F");
    mTree->Branch("sumet", &sumet, "sumet/F");
    mTree->Branch("rho", &rho, "rho/F");
    mTree->Branch("pthat", &pthat, "pthat/F");
    mTree->Branch("mcweight", &mcweight, "mcweight/F");

    mTree->Branch("chf", chf, "chf[njet]/F");   
    mTree->Branch("nhf", nhf, "nhf[njet]/F");   
    mTree->Branch("phf", phf, "phf[njet]/F");   
    mTree->Branch("elf", elf, "elf[njet]/F");   
    mTree->Branch("muf", muf, "muf[njet]/F");   
    mTree->Branch("hf_hf", hf_hf, "hf_hf[njet]/F");   
    mTree->Branch("hf_phf", hf_phf, "hf_phf[njet]/F");   
    mTree->Branch("hf_hm", hf_hm, "hf_hm[njet]/i");    
    mTree->Branch("hf_phm", hf_phm, "hf_phm[njet]/i");
    mTree->Branch("chm", chm, "chm[njet]/i");   
    mTree->Branch("nhm", nhm, "nhm[njet]/i");   
    mTree->Branch("phm", phm, "phm[njet]/i");   
    mTree->Branch("elm", elm, "elm[njet]/i");   
    mTree->Branch("mum", mum, "mum[njet]/i");
    mTree->Branch("beta", beta, "beta[njet]/F");   
    mTree->Branch("bstar", bstar, "bstar[njet]/F");
    
}

void OpenDataTreeProducer::endJob() {
}


void OpenDataTreeProducer::beginRun(edm::Run const &iRun,
                                     edm::EventSetup const &iSetup) {

    // Mapping trigger indices 
    bool changed(true);
    if (hltConfig_.init(iRun, iSetup, processName_, changed) && changed) {

        // List of trigger names and indices 
        // are not emptied between events, must be done here
        triggerIndex_.clear();
        triggernames.clear();

        // Iterate over all active triggers of the AOD file
        auto name_list = hltConfig_.triggerNames();
        for (std::string name_to_search: triggerNames_) {

            // Find the version of jet trigger that is active in this run 
            for (std::string name_candidate: name_list) {

                // Match the prefix to the full name (eg. HLT_Jet30 to HLT_Jet30_v10)
                if ( name_candidate.find(name_to_search + "_v") != std::string::npos ) {
                    // Save index corresponding to the trigger
                    triggerIndex_.push_back(hltConfig_.triggerIndex(name_candidate));

                    // Save the trigger name
                    triggernames.push_back("jt" + name_to_search.substr(7, string::npos));
                    break;            
                }
            }
        }
    }

    // Retrieve cross section of the simulated process
    if (mIsMCarlo) {

        edm::Handle<GenRunInfoProduct> genRunInfo;
        iRun.getByLabel("generator", genRunInfo );

        // Save only the cross section, since the total number of 
        // generated events is not available in this context (!!)
        mcweight = genRunInfo->crossSection();
        std::cout << "Cross section: " <<  mcweight << std::endl;
    }
    
}


void OpenDataTreeProducer::analyze(edm::Event const &event_obj,
                                    edm::EventSetup const &iSetup) {

    // Event info
    run = event_obj.id().run();
    lumi = event_obj.luminosityBlock();
    event = event_obj.id().event();

    // Triggers
    edm::Handle<edm::TriggerResults>   triggerResultsHandle_;
    event_obj.getByLabel(triggerResultsTag_, triggerResultsHandle_);

    // Sanity checks
    assert(triggerResultsHandle_.isValid() && "Error in getting TriggerResults from Event!");
    assert(triggerResultsHandle_->size() == hltConfig_.size() && "Size mismatch between triggerResultsHandle_ and hltConfig_");
    
    // Number of triggers to be saved
    ntrg = triggerIndex_.size();

    // Iterate only over the selected jet triggers
    for (unsigned itrig = 0; itrig < ntrg; itrig++) {

        // Trigger bit
        Bool_t isAccepted = triggerResultsHandle_->accept(triggerIndex_[itrig]);
        triggers[itrig] = isAccepted;

        // Trigger prescales are retrieved using the trigger name
        std::string trgName = hltConfig_.triggerName(triggerIndex_[itrig]);
        const std::pair< int, int > prescalePair(hltConfig_.prescaleValues(event_obj, iSetup, trgName));

        // Total prescale: PreL1*PreHLT 
        prescales[itrig] = prescalePair.first*prescalePair.second;   
    }    

    // Rho
    Handle< double > rho_handle;
    event_obj.getByLabel(mSrcPFRho, rho_handle);
    rho = *rho_handle;


    // Generator Info

    // Retrieve pthat and mcweight (only MC)
    if (mIsMCarlo && mUseGenInfo) {
        Handle< GenEventInfoProduct > hEventInfo;
        event_obj.getByLabel("generator", hEventInfo);

        // Monte Carlo weight (NOT AVAILABLE FOR 2011 MC!!)
        //mcweight = hEventInfo->weight();
        
        // Pthat 
        if (hEventInfo->hasBinningValues()) {
            pthat = hEventInfo->binningValues()[0];
        }
    }

    // Generator-level jets
    if (mIsMCarlo) {

        Handle< GenJetCollection > genjets;
        event_obj.getByLabel(mGenJetsName, genjets);
    
        // Index of the simulated jet
        int gen_index = 0; 

        for (GenJetCollection::const_iterator i_gen = genjets->begin(); i_gen != genjets->end(); i_gen++)  {

            // pT and rapidity selection
            if (i_gen->pt() > mMinGenPt && fabs(i_gen->y()) < mMaxY) {
                gen_pt[gen_index] = i_gen->pt();
                gen_eta[gen_index] = i_gen->eta();
                gen_phi[gen_index] = i_gen->phi();
                gen_E[gen_index] = i_gen->energy();
		gen_index++;
            }
        }

        // Number of generated jets in this event
        ngen = gen_index;
    }


    // PF AK5 Jets

    edm::Handle< std::vector< pat::Jet > > ak5_handle;
    event_obj.getByLabel(mPFak5JetsName, ak5_handle);

    // Copy vector of jets (they are sorted wrt. pT)
    std::vector< pat::Jet > patjets(ak5_handle->begin(), ak5_handle->end());

    // Index of the selected jet 
    int ak5_index = 0;

    // Vertex Info
    Handle<reco::VertexCollection> recVtxs;
    event_obj.getByLabel(mOfflineVertices, recVtxs);

    // Iterate over the jets of the event
    for (auto i_ak5jet = patjets.begin(); i_ak5jet != patjets.end(); ++i_ak5jet) {

        // Skip the current iteration if jet is not selected
        if (!i_ak5jet->isPFJet() || 
            fabs(i_ak5jet->y()) > mMaxY || 
            (i_ak5jet->pt()) < mMinPFPt) {
            continue;
        }

        // Computing beta and beta*

        // Get tracks
        reco::TrackRefVector tracks(i_ak5jet->associatedTracks());

        float sumTrkPt(0.0), sumTrkPtBeta(0.0),sumTrkPtBetaStar(0.0);
        beta[ak5_index] = 0.0;
        bstar[ak5_index] = 0.0;
        
        // Loop over tracks of the jet
        for(auto i_trk = tracks.begin(); i_trk != tracks.end(); i_trk++) {

            if (recVtxs->size() == 0) break;
            
            // Sum pT
            sumTrkPt += (*i_trk)->pt();
            
            // Loop over vertices
            for (unsigned ivtx = 0; ivtx < recVtxs->size(); ivtx++) {
                reco::Vertex vertex = (*recVtxs)[ivtx];

                // Loop over tracks associated with the vertex
                if (!(vertex.isFake()) && 
                    vertex.ndof() >= mGoodVtxNdof && 
                    fabs(vertex.z()) <= mGoodVtxZ) {
                    
                    for(auto i_vtxTrk = vertex.tracks_begin(); i_vtxTrk != vertex.tracks_end(); ++i_vtxTrk) {
                        
                        // Match the jet track to the track from the vertex
                        reco::TrackRef trkRef(i_vtxTrk->castTo<reco::TrackRef>());
                        
                        // Check for matching vertices
                        if (trkRef == (*i_trk)) {
                            if (ivtx == 0) {
                                sumTrkPtBeta += (*i_trk)->pt();
                            }
                            else {
                                sumTrkPtBetaStar += (*i_trk)->pt();
                            } 
                        } 
                    } 
                } 
            } 
        }
        if (sumTrkPt > 0) {
            beta[ak5_index]   = sumTrkPtBeta/sumTrkPt;
            bstar[ak5_index]  = sumTrkPtBetaStar/sumTrkPt;
        } 


        // Jet composition
        chf[ak5_index]     = i_ak5jet->chargedHadronEnergyFraction();
        nhf[ak5_index]     = i_ak5jet->neutralHadronEnergyFraction() + i_ak5jet->HFHadronEnergyFraction();
        phf[ak5_index]     = i_ak5jet->photonEnergyFraction();
        elf[ak5_index]     = i_ak5jet->electronEnergyFraction();
        muf[ak5_index]     = i_ak5jet->muonEnergyFraction();
        hf_hf[ak5_index]   = i_ak5jet->HFHadronEnergyFraction();
        hf_phf[ak5_index]  = i_ak5jet->HFEMEnergyFraction();
        hf_hm[ak5_index]   = i_ak5jet->HFHadronMultiplicity();
        hf_phm[ak5_index]  = i_ak5jet->HFEMMultiplicity();
        chm[ak5_index]     = i_ak5jet->chargedHadronMultiplicity();
        nhm[ak5_index]     = i_ak5jet->neutralHadronMultiplicity();
        phm[ak5_index]     = i_ak5jet->photonMultiplicity();
        elm[ak5_index]     = i_ak5jet->electronMultiplicity();
        mum[ak5_index]     = i_ak5jet->muonMultiplicity();
	jet_ID[ak5_index] = i_ak5jet->partonFlavour();
       
        int npr      = i_ak5jet->chargedMultiplicity() + i_ak5jet->neutralMultiplicity();

        bool isHighEta = fabs(i_ak5jet->eta()) > 2.4; 
        bool isLowEta = fabs(i_ak5jet->eta()) <= 2.4 && 
                        nhf[ak5_index] < 0.9 &&
                        phf[ak5_index] < 0.9 && 
                        elf[ak5_index] < 0.99 && 
                        chf[ak5_index] > 0 && 
                        chm[ak5_index] > 0;
        bool tightID =  npr > 1 && 
                        phf[ak5_index] < 0.99 && 
                        nhf[ak5_index] < 0.99 &&
                        (isLowEta || isHighEta);


        // Variables of the tuple
        jet_tightID[ak5_index] = tightID;
        jet_area[ak5_index] = i_ak5jet->jetArea();
        jet_jes[ak5_index] = 1/i_ak5jet->jecFactor(0); // JEC factor (pfjet is already corrected !!)

        // p4 is already corrected!
        auto p4 = i_ak5jet->p4();
        jet_pt[ak5_index]   = p4.Pt();
        jet_eta[ak5_index]  = p4.Eta();
        jet_phi[ak5_index]  = p4.Phi();
        jet_E[ak5_index]    = p4.E(); 
//	jet_ID[ak5_index] = p4.PartonFlavour();
//        jet_ID[ak5_index] = p4.pdgId();
      
        // Matching a GenJet to this PFjet
        if (mIsMCarlo && ngen > 0) {

            // Index of the generated jet matching this PFjet
            jet_igen[ak5_index] = -1; // is -1 if no matching jet

            // Search generated jet with minimum distance to this PFjet   
            float r2min(999);
            for (unsigned int gen_index = 0; gen_index != ngen; gen_index++) {
                double deltaR2 = reco::deltaR2( jet_eta[ak5_index], 
                                                jet_phi[ak5_index],
                                                gen_eta[gen_index], 
                                                gen_phi[gen_index]);
                if (deltaR2 < r2min) {
                    r2min = deltaR2;
                    jet_igen[ak5_index] = gen_index;
                }
            }
        }
        
    ak5_index++;
    }  
    // Number of selected jets in the event
    njet = ak5_index;    


    // Four leading AK7 Jets
    edm::Handle< std::vector< pat::Jet > > ak7_handle;
    event_obj.getByLabel(mPFak7JetsName, ak7_handle);

    // Copy vector of jets (they are sorted wrt. pT)
    std::vector< pat::Jet > ak7_patjets(ak7_handle->begin(), ak7_handle->end());

    // Index of the selected jet 
    int ak7_index = 0;

    // Iterate only over four leading jets
    for (auto i_ak7jet = ak7_patjets.begin(); i_ak7jet != ak7_patjets.end() && i_ak7jet - ak7_patjets.begin() != 4; ++i_ak7jet) {

        // Skip the current iteration if jet is not selected
        if (!i_ak7jet->isPFJet() || 
            fabs(i_ak7jet->y()) > mMaxY || 
            (i_ak7jet->pt()) < mMinPFPt) {
            continue;
        }

        // Variables of the tuple
        jet_area_ak7[ak7_index] = i_ak7jet->jetArea();
        jet_jes_ak7[ak7_index] = 1/i_ak7jet->jecFactor(0); // JEC factor (pfjet is already corrected !!)

        // p4 is already corrected!
        auto p4 = i_ak7jet->p4();
        jet_pt_ak7[ak7_index]   = p4.Pt();
        jet_eta_ak7[ak7_index]  = p4.Eta();
        jet_phi_ak7[ak7_index]  = p4.Phi();
        jet_E_ak7[ak7_index]    = p4.E(); 
        
        // Matching AK5 jet to this AK7 jet
        // Index of the generated jet matching this PFjet
        ak7_to_ak5[ak7_index] = -1; // -1 if no matching jet

        float r2min(999);
        for (unsigned int ak5_index = 0; ak5_index != njet; ak5_index++) {

            // Compute distance squared
            double deltaR2 = reco::deltaR2( jet_eta_ak7[ak7_index], 
                                            jet_phi_ak7[ak7_index],
                                            jet_eta[ak5_index], 
                                            jet_phi[ak5_index]);
            if (deltaR2 < r2min) {
                r2min = deltaR2;
                ak7_to_ak5[ak7_index] = ak5_index;
            }
        }
        
    ak7_index++;
    }  
    // Number of saved jets in the event
    njet_ak7 = ak7_index;    


    // MET
    Handle< PFMETCollection > met_handle;
    event_obj.getByLabel("pfMet", met_handle);

    met = (*met_handle)[0].et();
    sumet = (*met_handle)[0].sumEt();

    // Finally, fill the tree
    if (njet >= (unsigned)mMinNPFJets && 
        njet_ak7 >= (unsigned)mMinNPFJets ) {            
            mTree->Fill();
    }
}


void OpenDataTreeProducer::endRun(edm::Run const &iRun, edm::EventSetup const &iSetup) {

}

OpenDataTreeProducer::~OpenDataTreeProducer() {
}


DEFINE_FWK_MODULE(OpenDataTreeProducer);
