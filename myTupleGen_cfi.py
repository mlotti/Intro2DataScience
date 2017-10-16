from PhysicsTools.PatAlgos.patTemplate_cfg import *
from PhysicsTools.PatAlgos.tools.coreTools import *
import FWCore.ParameterSet.Config as cms
import FWCore.PythonUtilities.LumiList as LumiList
import FWCore.Utilities.FileUtils as FileUtils

## in case of data sample:
## removeMCMatching(process, ['All'])


## lets keep only jets
removeAllPATObjectsBut(process, ['Jets'])


process.p = cms.Path(
   process.patDefaultSequence
)


## soft links are needed to the database 
## ln -sf /cvmfs/cms-opendata-conddb.cern.ch/START53_LV6A1 START53_LV6A1
## ln -sf /cvmfs/cms-opendata-conddb.cern.ch/START53_LV6A1.db START53_LV6A1.db
process.GlobalTag.connect = cms.string('sqlite_file:/cvmfs/cms-opendata-conddb.cern.ch/START53_LV6A1.db')
process.GlobalTag.globaltag = cms.string('START53_LV6A1::All')


## we want only validated runs: (wget the .txt file)
myLumis = LumiList.LumiList(filename='Cert_160404-180252_7TeV_ReRecoNov08_Collisions11_JSON.txt').getCMSSWString().split(',')
process.source.lumisToProcess = cms.untracked.VLuminosityBlockRange()
process.source.lumisToProcess.extend(myLumis)


files = FileUtils.loadListFromFile ('CMS_MonteCarlo2011_Summer11LegDR_QCD_Pt-300to470_TuneZ2_7TeV_pythia6_AODSIM_PU_S13_START53_LV6-v1_00000_file_index.txt ') 
process.source.fileNames = cms.untracked.vstring( *files )


process.maxEvents.input = 5
process.MessageLogger.cerr.FwkReport.reportEvery = 1
process.out.fileName = 'myTuple.root'
