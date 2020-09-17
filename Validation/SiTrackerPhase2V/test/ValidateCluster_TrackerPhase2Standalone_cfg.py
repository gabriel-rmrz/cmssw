import FWCore.ParameterSet.Config as cms

from Configuration.Eras.Era_Phase2_cff import Phase2
process = cms.Process('clusterValTest', Phase2)
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1)
    )

process.load('Configuration.StandardSequences.Services_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
#process.load('Configuration.Geometry.GeometryExtended2023D17Reco_cff')
process.load('Configuration.Geometry.GeometryExtended2026D56Reco_cff')
#process.load('Configuration.Geometry.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')


###

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase2_realistic', '')

# list of files
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
      #'file:step2.root'
      #'file:051C2F4A-56AC-3943-BA4D-3DF295E026DF.root'
      'file:/afs/cern.ch/work/g/garamire/public/DQMTracker/CMSSW_11_1_0_pre7/src/Validation/SiTrackerPhase2V/test/051C2F4A-56AC-3943-BA4D-3DF295E026DF.root',
      )
    )

# Production Info
process.configurationMetadata = cms.untracked.PSet(
    version = cms.untracked.string('$Revision: 1.19 $'),
    annotation = cms.untracked.string('step1 nevts:1'),
    name = cms.untracked.string('Applications')
    )

# Output definition
process.DQMoutput = cms.OutputModule("PoolOutputModule",
    splitLevel = cms.untracked.int32(0),
    outputCommands = process.DQMEventContent.outputCommands,
    fileName = cms.untracked.string('step1_ClusterValTest.root'),
    dataset = cms.untracked.PSet(
      filterName = cms.untracked.string(''),
      dataTier = cms.untracked.string('')
      )
    )

process.load('Validation.SiTrackerPhase2V.Phase2OTValidateCluster_cff')
#process.clusterana_seq = cms.Sequence(process.otClusterValid * process.pixClusterValid)
process.clusterana_seq = cms.Sequence(process.otClusterValid)

process.load('DQMServices.Components.DQMEventInfo_cfi')
process.dqmEnv.subSystemFolder = cms.untracked.string('Ph2TkCluster')

process.dqm_comm =cms.Sequence(process.dqmEnv)

# Path and EndPath definitions
process.endjob_step = cms.EndPath(process.endOfProcess)
process.DQMoutput_step = cms.EndPath(process.DQMoutput)

#process.p = cms.Path(process.clusterana_seq * process.dqm_comm)
process.p = cms.Path(process.clusterana_seq)
