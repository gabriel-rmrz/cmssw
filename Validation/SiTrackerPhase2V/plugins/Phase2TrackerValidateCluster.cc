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
#include "Geometry/CommonDetUnit/interface/PixelGeomDetUnit.h"
//#include "Geometry/CommonDetUnit/interface/GeomDet.h"

#include "DataFormats/Common/interface/DetSetVectorNew.h"
#include "DataFormats/Common/interface/DetSetVector.h"

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

  edm::ESHandle<TrackerGeometry> geomHandle;
  if (theTkDigiGeomWatcher.check(iSetup)) {
    iSetup.get<TrackerDigiGeometryRecord>().get(geomType_, geomHandle);
  }
  if(!geomHandle.isValid()) return;
  const TrackerGeometry* tkGeom = &(*geomHandle);
  
  for(Phase2TrackerCluster1DCollectionNew::const_iterator DSVItr = clusterHandle->begin(); DSVItr != clusterHandle->end(); ++DSVItr){
    // Getting the id of detector unit
    unsigned int rawid(DSVItr->detId());
    DetId detId(rawid);

    // Getting the geometry of the detector unit
    const GeomDetUnit* geomDetUnit(tkGeom->idToDetUnit(detId));
    if(!geomDetUnit) continue;
    for(edmNew::DetSet<Phase2TrackerCluster1D>::const_iterator clusterItr = DSVItr->begin(); clusterItr != DSVItr->end(); ++clusterItr){
      MeasurementPoint mpCluster(clusterItr->center(), clusterItr->column() + 0.5);
      Local3DPoint localPosCluster = geomDetUnit->topology().localPosition(mpCluster);
      Global3DPoint globalPosCluster = geomDetUnit->surface().toGlobal(localPosCluster);

      std::cout << globalPosCluster << std::endl;
      std::cout << "Hi" << std::endl;

      if(SimulatedXYPositionMap)
        SimulatedXYPositionMap->Fill(globalPosCluster.x(), globalPosCluster.y());
    }

  }

  return;
}

//
// -- Book Histograms
//
void Phase2TrackerValidateCluster::bookHistograms(DQMStore::IBooker& ibooker, 
    edm::Run const& iRun, 
    edm::EventSetup const& iSetup){
  std::string top_folder = config_.getParameter<std::string>("TopFolderName");
  std::stringstream folder_name;

  ibooker.cd();
  folder_name << top_folder << "/"
    << "SimClusterInfo";
  ibooker.setCurrentFolder(folder_name.str());

  edm::LogInfo("Phase2TrackerValidateCluster") << " Booking Histograms in: " << folder_name.str();
  std::stringstream HistoName;

  edm::ParameterSet Parameters = config_.getParameter<edm::ParameterSet>("XYPositionMapH");
  HistoName.str("");
  HistoName << "SimulatedXPosVsYPos";
  if(Parameters.getParameter<bool>("switch"))
    SimulatedXYPositionMap = ibooker.book2D(HistoName.str(),
                                            HistoName.str(),
                                            Parameters.getParameter<int32_t>("NxBins"),
                                            Parameters.getParameter<double>("xmin"),
                                            Parameters.getParameter<double>("xmax"),
                                            Parameters.getParameter<int32_t>("NyBins"),
                                            Parameters.getParameter<double>("ymin"),
                                            Parameters.getParameter<double>("ymax"));
  else
    SimulatedXYPositionMap = nullptr;




  return;
}

