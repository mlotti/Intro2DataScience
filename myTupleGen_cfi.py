## import skeleton process
from PhysicsTools.PatAlgos.patTemplate_cfg import *

from PhysicsTools.PatAlgos.tools.coreTools import *


## keep only jets
removeAllPATObjectsBut(process, ['Jets'])

## make sure to keep the created objects
process.out.outputCommands += ['keep *_offlinePrimaryVertices_*_*']
process.out.outputCommands += ['keep *_pat*_*_*',]

## let it run
process.p = cms.Path(
   process.patDefaultSequence
   )

## globaltag in case of using VM
#process.GlobalTag.connect = cms.string('sqlite_file:/cvmfs/cms-opendata-conddb.cern.ch/FT_53_LV5_AN1_RUNA.db')
#process.GlobalTag.globaltag = 'FT_53_LV5_AN1::All'

## luminosity
import FWCore.ParameterSet.Config as cms
import FWCore.PythonUtilities.LumiList as LumiList
myLumis = LumiList.LumiList(filename='Cert_160404-180252_7TeV_ReRecoNov08_Collisions11_JSON.txt').getCMSSWString().split(',')
process.source.lumisToProcess = cms.untracked.VLuminosityBlockRange()
process.source.lumisToProcess.extend(myLumis)

## input files
import FWCore.Utilities.FileUtils as FileUtils
files = FileUtils.loadListFromFile ('CMS_MonteCarlo2011_Summer11LegDR_QCD_Pt-300to470_TuneZ2_7TeV_pythia6_AODSIM_PU_S13_START53_LV6-v1_00000_file_index.txt')
readFiles = cms.untracked.vstring( *files )
process.source.fileNames = readFiles


process.maxEvents.input = 5
process.MessageLogger.cerr.FwkReport.reportEvery = 1
process.out.fileName = 'myTuple.root'
