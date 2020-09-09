import FWCore.ParameterSet.Config as cms

from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer

clusterValid = DQMEDAnalyzer('Phase2TrackerValidateCluster',
    Verbosity = cms.bool(False),
    TopFolderName = cms.string("TrackerPhase2Clusters"),
    PixelPlotFillingFlag =cms.bool(False),
    ### TODO: Look for the ClusterSources
    #ClusterSource = cms.InputTag("TTClustersFromPhase2TrackerDigis"),
    ClusterSource = cms.InputTag("siPhase2Clusters"),
    InnerPixelClusterSource = cms.InputTag("siPixelClusters"),
    #simLinks = cms.InputTag("simSiPixelDigis", "Tracker"),
    OuterTrackerDigiSimLinkSource = cms.InputTag("simSiPixelDigis", "Tracker"),
    InnerPixelDigiSimLinkSource = cms.InputTag("simSiPixelDigis", "Pixel"), 
    PSimHitSource  = cms.VInputTag('g4SimHits:TrackerHitsPixelBarrelLowTof',
                                   'g4SimHits:TrackerHitsPixelBarrelHighTof',
                                   'g4SimHits:TrackerHitsPixelEndcapLowTof',
                                   'g4SimHits:TrackerHitsPixelEndcapHighTof',
                                   'g4SimHits:TrackerHitsTIBLowTof',
                                   'g4SimHits:TrackerHitsTIBHighTof',
                                   'g4SimHits:TrackerHitsTIDLowTof',
                                   'g4SimHits:TrackerHitsTIDHighTof',
                                   'g4SimHits:TrackerHitsTOBLowTof',
                                   'g4SimHits:TrackerHitsTOBHighTof',
                                   'g4SimHits:TrackerHitsTECLowTof',
                                   'g4SimHits:TrackerHitsTECHighTof'),
    #simhitsbarrel = cms.InputTag("g4SimHits", "TrackerHitsPixelBarrelLowTof"),
    #simhitsendcap = cms.InputTag("g4SimHits", "TrackerHitsPixelEndcapLowTof"),
    simtracks = cms.InputTag("g4SimHits"),
    SimTrackMinPt = cms.double(0.),
    ECasRings = cms.bool(True),
    GeometryType = cms.string('idealForDigi'),
    #OuterTrackerDigiSource = cms.InputTag("mix", "Tracker"),
    #InnerPixelDigiSource   = cms.InputTag("simSiPixelDigis","Pixel"),    
    #SimTrackSource = cms.InputTag("g4SimHits"),
    #SimVertexSource = cms.InputTag("g4SimHits"),
    #PtCutOff      = cms.double(9.5),    
    #EtaCutOff      = cms.double(3.5),    
    #TOFLowerCutOff = cms.double(-12.5),
    #TOFUpperCutOff = cms.double(12.5),
    ZRPositionMapH = cms.PSet(
      NxBins = cms.int32(6000),
      xmin = cms.double(-300.),
      xmax = cms.double(300.),
      NyBins = cms.int32(1200),
      ymin = cms.double(0.),
      ymax = cms.double(120.),
      switch = cms.bool(True)
      ),
    SimZRPositionMapH = cms.PSet(
      NxBins = cms.int32(6000),
      xmin = cms.double(-300.),
      xmax = cms.double(300.),
      NyBins = cms.int32(1200),
      ymin = cms.double(0.),
      ymax = cms.double(120.),
      switch = cms.bool(True)
      ),
    XYPositionMapH = cms.PSet(
      NxBins = cms.int32(2400),
      xmin = cms.double(-120.),
      xmax = cms.double(120.),
      NyBins = cms.int32(2400),
      ymin = cms.double(-120.),
      ymax = cms.double(120.),
      switch = cms.bool(True)
      ),
    XYBarrelPositionMapH = cms.PSet(
      NxBins = cms.int32(2400),
      xmin = cms.double(-120.),
      xmax = cms.double(120.),
      NyBins = cms.int32(2400),
      ymin = cms.double(-120.),
      ymax = cms.double(120.),
      switch = cms.bool(True)
      ),
    XYGlobalPositionMapH = cms.PSet(
      NxBins = cms.int32(500),
      xmin = cms.double(-120.),
      xmax = cms.double(120.),
      NyBins = cms.int32(2400),
      ymin = cms.double(-120.),
      ymax = cms.double(120.),
      switch = cms.bool(True)
      ),
    XYLocalPositionMapH = cms.PSet(
      NxBins = cms.int32(50),
      xmin = cms.double(-10.),
      xmax = cms.double(10.),
      NyBins = cms.int32(50),
      ymin = cms.double(-5.),
      ymax = cms.double(5.),
      switch = cms.bool(True)
      ),
    XYEndCapPositionMapH = cms.PSet(
      NxBins = cms.int32(2400),
      xmin = cms.double(-120.),
      xmax = cms.double(120.),
      NyBins = cms.int32(2400),
      ymin = cms.double(-120.),
      ymax = cms.double(120.),
      switch = cms.bool(True)
      ),
    ClusterSize = cms.PSet(
      NxBins = cms.int32(31),
      xmin = cms.double(-0.5),
      xmax = cms.double(30.5),
      switch = cms.bool(True)
      ),
    ClusterCharge = cms.PSet(
      NxBins = cms.int32(31),
      xmin = cms.double(-0.5),
      xmax = cms.double(30.5),
      switch = cms.bool(True)
      ),
    Delta_X_Strip = cms.PSet(
      NxBins = cms.int32(100),
      #xmin = cms.double(-0.02),
      #xmax = cms.double(0.02),
      xmin = cms.double(-3.),
      xmax = cms.double(3.),
      switch = cms.bool(True)
      ),
    Delta_X_Pixel = cms.PSet(
      NxBins = cms.int32(100),
      #xmin = cms.double(-0.02),
      #xmax = cms.double(0.02),
      xmin = cms.double(-3.),
      xmax = cms.double(3.),
      switch = cms.bool(True)
      ),
    Delta_Y_Strip = cms.PSet(
      NxBins = cms.int32(100),
      #xmin = cms.double(-0.2),
      #xmax = cms.double(0.2),
      xmin = cms.double(-3.),
      xmax = cms.double(3.),
      switch = cms.bool(True)
      ),
    Delta_Y_Pixel = cms.PSet(
      NxBins = cms.int32(100),
      xmin = cms.double(-3.),
      xmax = cms.double(3.),
      switch = cms.bool(True)
      ),
    Delta_X_Strip_P = cms.PSet(
      NxBins = cms.int32(100),
      #xmin = cms.double(-0.02),
      #xmax = cms.double(0.02),
      xmin = cms.double(-3.),
      xmax = cms.double(3.),
      switch = cms.bool(True)
      ),
    Delta_X_Pixel_P = cms.PSet(
      NxBins = cms.int32(100),
      #xmin = cms.double(-0.02),
      #xmax = cms.double(0.02),
      xmin = cms.double(-3.),
      xmax = cms.double(3.),
      switch = cms.bool(True)
      ),
    Delta_Y_Strip_P = cms.PSet(
      NxBins = cms.int32(100),
      #xmin = cms.double(-0.2),
      #xmax = cms.double(0.2),
      xmin = cms.double(-3.),
      xmax = cms.double(3.),
      switch = cms.bool(True)
      ),
    Delta_Y_Pixel_P = cms.PSet(
      NxBins = cms.int32(100),
      xmin = cms.double(-3.),
      xmax = cms.double(3.),
      switch = cms.bool(True)
      ),
    Primary_Digis_Strip = cms.PSet(
      NxBins = cms.int32(100),
      xmin = cms.double(0.),
      xmax = cms.double(1000.),
      switch = cms.bool(True)
      ),
    Primary_Digis_Pixel = cms.PSet(
      NxBins = cms.int32(100),
      xmin = cms.double(0.),
      xmax = cms.double(1000.),
      switch = cms.bool(True)
      ),
    Other_Digis_Strip = cms.PSet(
      NxBins = cms.int32(100),
      xmin = cms.double(0.),
      xmax = cms.double(1000.),
      switch = cms.bool(True)
      ),
    Other_Digis_Pixel = cms.PSet(
      NxBins = cms.int32(100),
      xmin = cms.double(0.),
      xmax = cms.double(1000.),
      switch = cms.bool(True)
      )
    )

