import FWCore.ParameterSet.Config as cms
from Validation.SiTrackerPhase2V.Phase2ITValidateCluster_cfi import *


pixClusterValid = clusterITValid.clone()
pixClusterValid.PixelPlotFillingFlag = cms.bool(True)
pixClusterValid.TopFolderName = cms.string("TrackerPhase2ITCluster")
pixClusterValid.XYPositionMapH = cms.PSet(
    Nxbins = cms.int32(340),
    xmin   = cms.double(-170.),
    xmax   = cms.double(170.),
    Nybins = cms.int32(340),
    ymin   = cms.double(-170.),
    ymax   = cms.double(170.),
    switch = cms.bool(False))

pixClusterValid.ZRPositionMapH = cms.PSet(
    Nxbins = cms.int32(3000),
    xmin   = cms.double(-3000.0),
    xmax   = cms.double(3000.),
    Nybins = cms.int32(280),
    ymin   = cms.double(0.),
    ymax   = cms.double(280.),
    switch = cms.bool(False))
