import FWCore.ParameterSet.Config as cms

from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer

clusterValid = DQMEDAnalyzer('Phase2TrackerValidateCluster',
    Verbosity = cms.bool(False),
    TopFolderName = cms.string("TrackerPhase2Clusters"),
    PixelPlotFillingFlag =cms.bool(False),
    ### TODO: Look for the ClusterSources
    #ClusterSource = cms.InputTag("TTClustersFromPhase2TrackerDigis"),
    ClusterSource = cms.InputTag("siPhase2Clusters"),
    #  "ClusterInclusive"   "HLT"
    ECasRings = cms.bool(True),
    GeometryType = cms.string('idealForDigi'),
    ZRPositionMapH = cms.PSet(
      NxBins = cms.int32(2400),
      xmin = cms.double(-120.),
      xmax = cms.double(120.),
      NyBins = cms.int32(6000),
      ymin = cms.double(-300.),
      ymax = cms.double(300.),
      switch = cms.bool(True)
      )
    XYPositionMapH = cms.PSet(
      NxBins = cms.int32(2400),
      xmin = cms.double(-120.),
      xmax = cms.double(120.),
      NyBins = cms.int32(2400),
      ymin = cms.double(-120.),
      ymax = cms.double(120.),
      switch = cms.bool(True)
      )
    XYBarrelPositionMapH = cms.PSet(
      NxBins = cms.int32(2400),
      xmin = cms.double(-120.),
      xmax = cms.double(120.),
      NyBins = cms.int32(2400),
      ymin = cms.double(-120.),
      ymax = cms.double(120.),
      switch = cms.bool(True)
      )
    XYBarrelPositionMapH = cms.PSet(
      NxBins = cms.int32(2400),
      xmin = cms.double(-120.),
      xmax = cms.double(120.),
      NyBins = cms.int32(2400),
      ymin = cms.double(-120.),
      ymax = cms.double(120.),
      switch = cms.bool(True)
      )
    )

