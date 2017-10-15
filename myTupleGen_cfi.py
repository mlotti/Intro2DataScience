from PhysicsTools.PatAlgos.patTemplate_cfg import *
from PhysicsTools.PatAlgos.tools.coreTools import *
import FWCore.ParameterSet.Config as cms
import FWCore.PythonUtilities.LumiList as LumiList
import FWCore.Utilities.FileUtils as FileUtils

## not sure what this does:
removeMCMatching(process, ['All'])


## lets keep only jets
removeAllPATObjectsBut(process, ['Jets'])


process.p = cms.Path(
   process.patDefaultSequence
)


## not sure how these should be set (the name for the connect is not correct!): 
process.GlobalTag.connect = cms.string('sqlite_file:/cvmfs/cms-opendata-conddb.cern.ch/START53_LV6.db')
process.GlobalTag.globaltag = cms.string('START53_LV6::All')


## we want only validated runs: ### NEED TO FIGURE OUT WHERE TO FIND THE .JSON FOR THIS!!!  
myLumis = LumiList.LumiList(filename='Cert_160404-180252_7TeV_ReRecoNov08_Collisions11_JSON.txt').getCMSSWString().split(',')
process.source.lumisToProcess = cms.untracked.VLuminosityBlockRange()
process.source.lumisToProcess.extend(myLumis)


files = FileUtils.loadListFromFile ('CMS_MonteCarlo2011_Summer11LegDR_QCD_Pt-300to470_TuneZ2_7TeV_pythia6_AODSIM_PU_S13_START53_LV6-v1_00000_file_index.txt ') 
process.source.fileNames = cms.untracked.vstring( *files )


process.maxEvents.input = 5
process.MessageLogger.cerr.FwkReport.reportEvery = 1
process.out.fileName = 'file://myTuple.root'
