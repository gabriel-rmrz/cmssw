import FWCore.ParameterSet.Config as cms

from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer

clusterValid = DQMEDAnalyzer('Phase2TrackerValidateCluster',
    Verbosity = cms.bool(False),
    TopFolderName = cms.string("Ph2TkPixerDigi"),
    PixelPlotFillingFlag =cms.bool(False),
    ### TODO: Look for the ClusterSources
    GeometryType = cms.string('idealForDigi'),
    XYPositionMapH = cms.PSet(
      Nxbins = cms.int32(1250),
      xmin = cms.double(-1250.),
      xmax = cms.double(1250.),
      Nybins = cms.int32(1250),
      ymin = cms.double(-1250.),
      ymax = cms.double(1250.),
      switch = cms.bool(True)
      )
    )
