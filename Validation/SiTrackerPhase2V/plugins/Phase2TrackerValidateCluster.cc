// -*- C++ -*-
//
// Package:    Phase2TrackerValidateCluster
// Class:      Phase2TrackerValidateCluster
//
/**\class Phase2TrackerValidateCluster Phase2TrackerValidateCluster.cc 

 Description: Validation plots tracker clusters. 

*/
//
// Author: Gabriel Ramirez
// Date: May 23, 2020
//
// system include files
#include <memory>
#include "Validation/SiTrackerPhase2V/plugins/Phase2TrackerValidateCluster.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Framework/interface/ESWatcher.h"


#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"

#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"



// DQM Histograming
#include "DQMServices/Core/interface/MonitorElement.h"

//
// constructors
//
Phase2TrackerValidateCluster::Phase2TrackerValidateCluster(const edm::ParameterSet& iConfig)
    : config_(iConfig),
      clustersToken_(consumes<Phase2TrackerCluster1DCollectionNew>(config_.getParameter<edm::InputTag>("src"))),
      geomType_(config_.getParameter<std::string>("GeometryType"))

{
  edm::LogInfo("Phase2TrackerValidateCluster") << ">>> Construct Phase2TrackerValidateCluster ";
}
    
Phase2TrackerValidateCluster::~Phase2TrackerValidateCluster() {
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
  edm::LogInfo("Phase2TrackerValidateCluster") << ">>> Destroy Phase2TrackerValidateCluster ";
}
//
// -- DQM Begin Run
//
// -- Analyze
//
void Phase2TrackerValidateCluster::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  // Getting the clusters
  edm::Handle<Phase2TrackerCluster1DCollectionNew> clusterHandle;
  iEvent.getByToken(clustersToken_, clusterHandle);

  // Getting the geometry
  edm::ESWatcher<TrackerDigiGeometryRecord> theTkDigiGeomWatcher;

  if (theTkDigiGeomWatcher.check(iSetup)) {
    edm::ESHandle<TrackerGeometry> geomHandle;
    iSetup.get<TrackerDigiGeometryRecord>().get(geomType_, geomHandle);
    if(!geomHandle.isValid()) return;
  }
  /*
  for(Phase2TrackerCluster1DCollectionNew::const_iterator DSVItr = clusters->begin(); DSVItr != clusters->end(); ++DSVItr){
    unsigned int rawid(DSVItr->detId());
    DetId detId(rawid);

    const TrackerGeometry* tkrGeom = geomHandle.product();
  }
  */
  return;
}

//
// -- Book Histograms
//
void Phase2TrackerValidateCluster::bookHistograms(DQMStore::IBooker& ibooker, 
    edm::Run const& iRun, 
    edm::EventSetup const& iSetup){


  return;
}

