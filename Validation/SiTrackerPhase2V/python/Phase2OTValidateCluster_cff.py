import FWCore.ParameterSet.Config as cms
from Validation.SiTrackerPhase2V.Phase2OTValidateCluster_cfi import *

otClusterValid = clusterValid.clone()
otClusterValid.PixelPlotFillingFlag = cms.bool(False)
otClusterValid.TopFolderName = cms.string("TrackerPhase2OTCluster")


