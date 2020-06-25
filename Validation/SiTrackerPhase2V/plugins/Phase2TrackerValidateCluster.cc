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
//#define DEBUG 

#ifdef DEBUG
#include <bitset>
#endif

#include <memory>
#include "Validation/SiTrackerPhase2V/plugins/Phase2TrackerValidateCluster.h"

#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESWatcher.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
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


// DQM Histograming
#include "DQMServices/Core/interface/MonitorElement.h"

//
// constructors
//

Phase2TrackerValidateCluster::Phase2TrackerValidateCluster(const edm::ParameterSet& iConfig)
    : config_(iConfig),
      pixelFlag_(config_.getParameter<bool>("PixelPlotFillingFlag")),
      catECasRings_(config_.getParameter<bool>("ECasRings")),
      geomType_(config_.getParameter<std::string>("GeometryType")),
      clustersToken_(consumes<Phase2TrackerCluster1DCollectionNew>(config_.getParameter<edm::InputTag>("ClusterSource")))

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

  edm::ESHandle<TrackerTopology> tTopoHandle;
  iSetup.get<TrackerTopologyRcd>().get(tTopoHandle);
  const TrackerTopology* tTopo = tTopoHandle.product();

  
  for(Phase2TrackerCluster1DCollectionNew::const_iterator DSVItr = clusterHandle->begin(); DSVItr != clusterHandle->end(); ++DSVItr){
    // Getting the id of detector unit
    unsigned int rawid(DSVItr->detId());
    DetId detId(rawid);

    // Getting the layer
    unsigned int layer = (tTopo->side(detId) != 0) * 1000; 
#ifdef DEBUG
    std::cout << "Side: " << (tTopo->side(detId) != 0) <<std::endl;
    //std::bitset<25> x(detId);
    //std::cout << "DetId: " << x <<std::endl;
#endif
    if (!layer) {
      layer += tTopo->layer(detId);
    }
    else {
      layer += (catECasRings_ ? tTopo->tidRing(detId) * 10 : tTopo->layer(detId));
    }
#ifdef DEBUG
    std::cout << "Layer modified: " << layer << std::endl;
    std::cout << "tTopo->tidRing(detId)*10: " << tTopo->tidRing(detId) * 10 << std::endl;
    std::cout << "tTopo->layer(detId): " << tTopo->layer(detId) << std::endl;
#endif

    // Getting the geometry of the detector unit
    const GeomDetUnit* geomDetUnit(tkGeom->idToDetUnit(detId));
    if(!geomDetUnit) continue;
    for(edmNew::DetSet<Phase2TrackerCluster1D>::const_iterator clusterItr = DSVItr->begin(); clusterItr != DSVItr->end(); ++clusterItr){
      MeasurementPoint mpCluster(clusterItr->center(), clusterItr->column() + 0.5);
      Local3DPoint localPosCluster = geomDetUnit->topology().localPosition(mpCluster);
      Global3DPoint globalPosCluster = geomDetUnit->surface().toGlobal(localPosCluster);

      if(SimulatedZRPositionMap)
        SimulatedZRPositionMap->Fill(globalPosCluster.z(), globalPosCluster.perp());

      if(SimulatedXYPositionMap)
        SimulatedXYPositionMap->Fill(globalPosCluster.x(), globalPosCluster.y());

      if(fabs(layer) < 1000){
        if(SimulatedXYBarrelPositionMap) SimulatedXYBarrelPositionMap->Fill(globalPosCluster.x(), globalPosCluster.y());
      } 
      else {
        if(SimulatedXYEndCapPositionMap) SimulatedXYEndCapPositionMap->Fill(globalPosCluster.x(), globalPosCluster.y());
      }
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

  edm::ParameterSet Parameters = config_.getParameter<edm::ParameterSet>("ZRPositionMapH");
  HistoName.str("");
  HistoName << "SimulatedZPosVsRPos";
  if(Parameters.getParameter<bool>("switch"))
    SimulatedZRPositionMap = ibooker.book2D(HistoName.str(),
                                            HistoName.str(),
                                            Parameters.getParameter<int32_t>("NxBins"),
                                            Parameters.getParameter<double>("xmin"),
                                            Parameters.getParameter<double>("xmax"),
                                            Parameters.getParameter<int32_t>("NyBins"),
                                            Parameters.getParameter<double>("ymin"),
                                            Parameters.getParameter<double>("ymax"));
  else
    SimulatedZRPositionMap = nullptr;

  Parameters = config_.getParameter<edm::ParameterSet>("XYPositionMapH");
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

  Parameters = config_.getParameter<edm::ParameterSet>("XYBarrelPositionMapH");
  HistoName.str("");
  HistoName << "SimulatedXPosVsYPosBarrel";
  if(Parameters.getParameter<bool>("switch"))
    SimulatedXYBarrelPositionMap = ibooker.book2D(HistoName.str(),
                                            HistoName.str(),
                                            Parameters.getParameter<int32_t>("NxBins"),
                                            Parameters.getParameter<double>("xmin"),
                                            Parameters.getParameter<double>("xmax"),
                                            Parameters.getParameter<int32_t>("NyBins"),
                                            Parameters.getParameter<double>("ymin"),
                                            Parameters.getParameter<double>("ymax"));
  else
    SimulatedXYBarrelPositionMap = nullptr;

  Parameters = config_.getParameter<edm::ParameterSet>("XYEndCapPositionMapH");
  HistoName.str("");
  HistoName << "SimulatedXPosVsYPosEndCap";
  if(Parameters.getParameter<bool>("switch"))
    SimulatedXYEndCapPositionMap = ibooker.book2D(HistoName.str(),
                                            HistoName.str(),
                                            Parameters.getParameter<int32_t>("NxBins"),
                                            Parameters.getParameter<double>("xmin"),
                                            Parameters.getParameter<double>("xmax"),
                                            Parameters.getParameter<int32_t>("NyBins"),
                                            Parameters.getParameter<double>("ymin"),
                                            Parameters.getParameter<double>("ymax"));
  else
    SimulatedXYEndCapPositionMap = nullptr;

  edm::ESWatcher<TrackerDigiGeometryRecord> theTkDigiGeomWatcher;

  iSetup.get<TrackerTopologyRcd>().get(tTopoHandle_);
  const TrackerTopology* const tTopo = tTopoHandle_.product();
  if (theTkDigiGeomWatcher.check(iSetup)) {
    edm::ESHandle<TrackerGeometry> geom_handle;
    iSetup.get<TrackerDigiGeometryRecord>().get(geomType_, geom_handle);
    const TrackerGeometry* tGeom = geom_handle.product();

    for (auto const& det_u : tGeom->detUnits()) {
      unsigned int detId_raw = det_u->geographicalId().rawId();
      bookLayerHistos(ibooker, detId_raw, tTopo, pixelFlag_);
    }    
  }
  return;
}
void Phase2TrackerValidateCluster::bookLayerHistos(DQMStore::IBooker& ibooker,
                                                   unsigned int det_id,
                                                   const TrackerTopology* tTopo,
                                                   bool flag) {
  int layer;
  int ring = 0;
  if(flag){
    layer = tTopo->getITPixelLayerNumber(det_id);
  }
  else {
    layer = tTopo->getOTLayerNumber(det_id);
    ring = tTopo->tidRing(det_id);
  }

  if(layer < 0)
    return;
  // This is to include disks 1 and 2; and 3, 4 and 5 disk in only one histogram
  int layers = layer;
  std::string idiscs;

  int side = layer / 100;
  if(layer>100){
    if((layer%100) < 3){
      layers = 12000 + 10*ring + side; // meaning that the disk 1 and 2 will be filled togehter
      idiscs = "1_2";
    } else if((layer%100) < 6){
      layers = 345000 + 10*ring + side; // disks 3, 4 and 5 will be filled together
      idiscs = "3_4_5";
    }
  }


  std::map<uint32_t, ClusterMEs>::iterator pos = layerMEs.find(layers);

  if(pos == layerMEs.end()){
    std::string top_folder = config_.getParameter<std::string>("TopFolderName");
    std::stringstream folder_name;

    std::ostringstream fname1, fname2, tag;
    if (layer < 100) {
      fname1 << "Barrel";
      fname2 << "Layer_" << layer;
    } else {
      //int side = layer / 100;
      //int idisc = layer - side * 100;
      fname1 << "EndCap_Side_" << side;
      fname2 << "Discs_" << idiscs << "_ring_" << ring;
#ifdef DEBUG
      std::cout << "---- ring: " << ring << std::endl;
      std::cout << "---- fname2: " << fname2.str() << std::endl;
      std::cout << "---- disk: " << idisc << std::endl;
#endif
    }
    ibooker.cd();
    folder_name << top_folder << "/"
                << "ClusterMonitor"
                << "/" << fname1.str()
                << "/" << fname2.str();

    edm::LogInfo("Phase2TrackerValidateDigi") << " Booking Histograms in: " << folder_name.str();

    ibooker.setCurrentFolder(folder_name.str());

    std::ostringstream HistoName;

    ClusterMEs local_mes;
    edm::ParameterSet Parameters = config_.getParameter<edm::ParameterSet>("ZRPositionMapH");
    HistoName.str("");
    HistoName << "ZPosVsRPos";
    if(Parameters.getParameter<bool>("switch"))
      local_mes.ZRPositionMap = ibooker.book2D(HistoName.str(),
                                              HistoName.str(),
                                              Parameters.getParameter<int32_t>("NxBins"),
                                              Parameters.getParameter<double>("xmin"),
                                              Parameters.getParameter<double>("xmax"),
                                              Parameters.getParameter<int32_t>("NyBins"),
                                              Parameters.getParameter<double>("ymin"),
                                              Parameters.getParameter<double>("ymax"));
    else
      local_mes.ZRPositionMap = nullptr;
    local_mes.nCluster = 1;
    layerMEs.insert(std::make_pair(layers, local_mes));

  }

}



DEFINE_FWK_MODULE(Phase2TrackerValidateCluster);
