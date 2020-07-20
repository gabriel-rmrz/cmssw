// -*- C++ -*-
//bookLayer
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
      simtrackminpt_(config_.getParameter<double>("SimTrackMinPt")),
      geomType_(config_.getParameter<std::string>("GeometryType")),
      simLinksToken_(consumes<edm::DetSetVector<PixelDigiSimLink>>(config_.getParameter<edm::InputTag>("simLinks"))),
      simTracksToken_(consumes<edm::SimTrackContainer>(config_.getParameter<edm::InputTag>("simtracks"))),
      clustersToken_(consumes<Phase2TrackerCluster1DCollectionNew>(config_.getParameter<edm::InputTag>("ClusterSource"))),
      simHitsBToken_(consumes<edm::PSimHitContainer>(config_.getParameter<edm::InputTag>("simhitsbarrel"))),
      simHitsEToken_(consumes<edm::PSimHitContainer>(config_.getParameter<edm::InputTag>("simhitsendcap")))

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

  // Getting PixelDigiSimLinks
  edm::Handle<edm::DetSetVector<PixelDigiSimLink> > pixelSimLinksHandle;
  iEvent.getByToken(simLinksToken_, pixelSimLinksHandle);

  // Get the SimHits
  edm::Handle<edm::PSimHitContainer> simHitsRaw[2];
  iEvent.getByToken(simHitsBToken_, simHitsRaw[0]);
  iEvent.getByToken(simHitsEToken_, simHitsRaw[1]);

  // Get the SimTracks
  edm::Handle<edm::SimTrackContainer> simTracksRaw;
  iEvent.getByToken(simTracksToken_, simTracksRaw);

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

  // Rearrange the simTracks for ease of use <simTrackID, simTrack>
  SimTracksMap simTracks;
  for (edm::SimTrackContainer::const_iterator simTrackIt(simTracksRaw->begin()); simTrackIt != simTracksRaw->end();
       ++simTrackIt) {
    if (simTrackIt->momentum().pt() > simtrackminpt_) {
      simTracks.emplace(simTrackIt->trackId(), *simTrackIt);
    }
  }
  
  // Number of clusters
  std::map<std::string, unsigned int> nClusters[3];
  std::map<std::string, unsigned int> nPrimarySimHits[3];
  std::map<std::string, unsigned int> nOtherSimHits[3];

  for(Phase2TrackerCluster1DCollectionNew::const_iterator DSVItr = clusterHandle->begin(); DSVItr != clusterHandle->end(); ++DSVItr){
    // Getting the id of detector unit
    uint32_t rawid(DSVItr->detId());
    DetId detId(rawid);


    ///////////////////////////
    // Getting the geometry of the detector unit
    ///////////////////////////
    const GeomDetUnit* geomDetUnit(tkGeom->idToDetUnit(detId));
    if(!geomDetUnit) continue;


    TrackerGeometry::ModuleType mType = tkGeom->getDetectorType(detId);
    unsigned int det = 0;
    if (mType == TrackerGeometry::ModuleType::Ph2PSP) {
      det = 1;
    } else if (mType == TrackerGeometry::ModuleType::Ph2PSS || mType == TrackerGeometry::ModuleType::Ph2SS) {
      det = 2;
    } else {
      std::cout << "UNKNOWN DETECTOR TYPE!" << std::endl;
    }

    std::string folderkey = getHistoId(detId, tTopo, pixelFlag_);

    // initialize the nhit counters if they don't exist for this layer
    auto nhitit(nClusters[det].find(folderkey));
    if (nhitit == nClusters[det].end()) {
      nClusters[det].emplace(folderkey, 0);
      nPrimarySimHits[det].emplace(folderkey, 0);
      nOtherSimHits[det].emplace(folderkey, 0);
    }


    for(edmNew::DetSet<Phase2TrackerCluster1D>::const_iterator clusterItr = DSVItr->begin(); clusterItr != DSVItr->end(); ++clusterItr){

      MeasurementPoint mpCluster(clusterItr->center(), clusterItr->column() + 0.5);
      Local3DPoint localPosCluster = geomDetUnit->topology().localPosition(mpCluster);
      Global3DPoint globalPosCluster = geomDetUnit->surface().toGlobal(localPosCluster);
      unsigned int layer = tTopo->getOTLayerNumber(detId);

      /////////////////////////
      // Find the closest simhit
      /////////////////////////

      // Get simTracks from the cluster
      std::vector<unsigned int> clusterSimTrackIds;
      for(unsigned int i(0); i < clusterItr->size(); ++i){
        unsigned int channel(Phase2TrackerDigi::pixelToChannel(clusterItr->firstRow() + i, clusterItr->column()));
        std::vector<unsigned int> simTrackIds(getSimTrackId(pixelSimLinksHandle, detId, channel));
        for (auto it : simTrackIds) {
          bool add = true;
          for (unsigned int j = 0; j < clusterSimTrackIds.size(); ++j) {
            // only save simtrackids that are not present yet
            if (it == clusterSimTrackIds.at(j))
              add = false;
          }
          if (add)
            clusterSimTrackIds.push_back(it);
        }
      }
      std::sort(clusterSimTrackIds.begin(), clusterSimTrackIds.end());
      const PSimHit* closestSimHit = 0;
      float minx = 10000;
      for (unsigned int simhitidx = 0; simhitidx < 2; ++simhitidx) {  // loop over both barrel and endcap hits
        for (auto simhitIt : *simHitsRaw[simhitidx]) {
          if (rawid == simhitIt.detUnitId()) {
            auto it = std::lower_bound(clusterSimTrackIds.begin(), clusterSimTrackIds.end(), simhitIt.trackId());
            if (it != clusterSimTrackIds.end() && *it == simhitIt.trackId()) {
              if (!closestSimHit || fabs(simhitIt.localPosition().x() - localPosCluster.x()) < minx) {
                minx = fabs(simhitIt.localPosition().x() - localPosCluster.x());
                closestSimHit = &simhitIt;
              }
            }
          }
        }
      }
      if (!closestSimHit)
        continue;
       // only look at simhits from highpT tracks
      auto simTrackIt(simTracks.find(closestSimHit->trackId()));
      if (simTrackIt == simTracks.end())
         continue;
      Local3DPoint localPosHit(closestSimHit->localPosition());

      // cluster size
      ++(nClusters[det].at(folderkey));
      ++(nOtherSimHits[det].at(folderkey));


      /////////////////////////
      // Filling histograms
      /////////////////////////

      if(SimulatedZRPositionMap)
        SimulatedZRPositionMap->Fill(globalPosCluster.z(), globalPosCluster.perp());

      if(SimulatedXYPositionMap)
        SimulatedXYPositionMap->Fill(globalPosCluster.x(), globalPosCluster.y());

      if(geomDetUnit->type().isBarrel()){
        if(SimulatedXYBarrelPositionMap) SimulatedXYBarrelPositionMap->Fill(globalPosCluster.x(), globalPosCluster.y());
      } 
      else {
        if(SimulatedXYEndCapPositionMap) SimulatedXYEndCapPositionMap->Fill(globalPosCluster.x(), globalPosCluster.y());
      }


      auto pos = layerMEs.find(folderkey);
      if (pos == layerMEs.end())
        continue;
      ClusterMEs& local_mes = pos->second;
      if(layer < 100){
        if(det == 1){
          local_mes.XYGlobalPositionMapPixel->Fill(globalPosCluster.z(), globalPosCluster.perp());
          local_mes.XYLocalPositionMapPixel->Fill(localPosCluster.x(), localPosCluster.y());
        }else if(det == 2){
          local_mes.XYGlobalPositionMapStrip->Fill(globalPosCluster.z(), globalPosCluster.perp());
          local_mes.XYLocalPositionMapStrip->Fill(localPosCluster.x(), localPosCluster.y());
        }
      }
      if(local_mes.ClusterSize)
        local_mes.ClusterSize->Fill(clusterItr->size());
      if(det == 1){
        local_mes.deltaXPixel->Fill(localPosCluster.x() - localPosHit.x());
        local_mes.deltaYPixel->Fill(localPosCluster.y() - localPosHit.y());
      }
      else if(det == 2){
        local_mes.deltaXStrip->Fill(localPosCluster.x() - localPosHit.x());
        local_mes.deltaYStrip->Fill(localPosCluster.y() - localPosHit.y());
      }
         // Primary particles only
      unsigned int procT(closestSimHit->processType());
      if (simTrackIt->second.vertIndex() == 0 and
          (procT == 2 || procT == 7 || procT == 9 || procT == 11 || procT == 13 || procT == 15)) {
        ++(nPrimarySimHits[det].at(folderkey));
        --(nOtherSimHits[det].at(folderkey));  // avoid double counting
        if(det == 1){
          local_mes.deltaXPixelP->Fill(localPosCluster.x() - localPosHit.x());
          local_mes.deltaYPixelP->Fill(localPosCluster.y() - localPosHit.y());
        }else if(det == 2){
          local_mes.deltaXStripP->Fill(localPosCluster.x() - localPosHit.x());
          local_mes.deltaYStripP->Fill(localPosCluster.y() - localPosHit.y());
        }
      }

    }

  }

  for (unsigned int det = 1; det < 3; ++det) {
    for (auto it : nClusters[det]) {
      auto pos = layerMEs.find(it.first);
      std::cout << "Det: " << det << std::endl;
      std::cout << "nClusters[det]: " << nClusters[det].at(it.first) << std::endl;
      if (pos == layerMEs.end())
      {
        std::cout << "*** SL *** No histogram for an existing counter! This should not happen!" << std::endl;
        continue;
      }
      ClusterMEs& local_mes = pos->second;
      if(det == 1){
        local_mes.allDigisPixel->Fill(it.second);
      }else if(det == 2){
        local_mes.allDigisStrip->Fill(it.second);
      }
    }
    for (auto it : nPrimarySimHits[det]) {
      auto pos = layerMEs.find(it.first);
      if (pos == layerMEs.end())
      {
        std::cout << "*** SL *** No histogram for an existing counter! This should not happen!" << std::endl;
        continue;
      }
      ClusterMEs& local_mes = pos->second;
      if(det == 1){
        local_mes.primaryDigisPixel->Fill(it.second);
      }else if(det == 2){
        local_mes.primaryDigisStrip->Fill(it.second);
      }
    }
    for (auto it : nOtherSimHits[det]) {
      auto pos = layerMEs.find(it.first);
      if (pos == layerMEs.end())
      {
        std::cout << "*** SL *** No histogram for an existing counter! This should not happen!" << std::endl;
        continue;
      }
      ClusterMEs& local_mes = pos->second;
      if(det == 1){
        local_mes.otherDigisPixel->Fill(it.second);
      }else if(det == 2){
        local_mes.otherDigisStrip->Fill(it.second);
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
      uint32_t detId_raw = det_u->geographicalId().rawId();
      bookLayerHistos(ibooker, detId_raw, tTopo, pixelFlag_);
    }    
  }
  //return;
  for(auto& f : layerMEs) 
    std::cout << f.first << std::endl;
}

//////////////////Layer Histo/////////////////////////////////
void Phase2TrackerValidateCluster::bookLayerHistos(DQMStore::IBooker& ibooker,
                                                   uint32_t det_id,
                                                   const TrackerTopology* tTopo,
                                                   bool flag) {
  std::string folderName = getHistoId(det_id, tTopo, flag);
  if(folderName == ""){
    edm::LogInfo("Phase2TrackerValidateCluster") << ">>>> Invalid histo_id ";
    return;
  }
  
  unsigned int layer = tTopo->getOTLayerNumber(det_id);

  std::map<std::string, ClusterMEs>::iterator pos = layerMEs.find(folderName);

  if(pos == layerMEs.end()){
    std::cout << "histo_id: " << folderName << std::endl;

    
    std::string top_folder = config_.getParameter<std::string>("TopFolderName");
    //std::stringstream folder_name;

    ibooker.cd();

    edm::LogInfo("Phase2TrackerValidateDigi") << " Booking Histograms in: " << folderName;

    ibooker.setCurrentFolder(top_folder+'/'+folderName);

    std::ostringstream HistoName;

    ClusterMEs local_mes;
    edm::ParameterSet Parameters = config_.getParameter<edm::ParameterSet>("ClusterSize");
    HistoName.str("");
    HistoName << "ClusterSize";
    if(Parameters.getParameter<bool>("switch"))
      local_mes.ClusterSize = ibooker.book1D(HistoName.str(),
                                              HistoName.str(),
                                              Parameters.getParameter<int32_t>("NxBins"),
                                              Parameters.getParameter<double>("xmin"),
                                              Parameters.getParameter<double>("xmax"));
    else
      local_mes.ClusterSize = nullptr;
    
    /*
    Parameters = config_.getParameter<edm::ParameterSet>("ClusterCharge");
    HistoName.str("");
    HistoName << "ClusterCharge";
    if(Parameters.getParameter<bool>("switch"))
      local_mes.ClusterCharge = ibooker.book1D(HistoName.str(),
                                              HistoName.str(),
                                              Parameters.getParameter<int32_t>("NxBins"),
                                              Parameters.getParameter<double>("xmin"),
                                              Parameters.getParameter<double>("xmax"));
    else
      local_mes.ClusterCharge = nullptr;
      */

    // Delta position with simhits

    Parameters = config_.getParameter<edm::ParameterSet>("Delta_X_Strip");
    HistoName.str("");
    HistoName << "Delta_X_Strip";
    if(Parameters.getParameter<bool>("switch"))
      local_mes.deltaXStrip = ibooker.book1D(HistoName.str(),
                                              HistoName.str(),
                                              Parameters.getParameter<int32_t>("NxBins"),
                                              Parameters.getParameter<double>("xmin"),
                                              Parameters.getParameter<double>("xmax"));
    else
      local_mes.deltaXStrip = nullptr;

    Parameters = config_.getParameter<edm::ParameterSet>("Delta_X_Pixel");
    HistoName.str("");
    HistoName << "Delta_X_Pixel";
    if(Parameters.getParameter<bool>("switch"))
      local_mes.deltaXPixel = ibooker.book1D(HistoName.str(),
                                              HistoName.str(),
                                              Parameters.getParameter<int32_t>("NxBins"),
                                              Parameters.getParameter<double>("xmin"),
                                              Parameters.getParameter<double>("xmax"));
    else
      local_mes.deltaXPixel = nullptr;

    Parameters = config_.getParameter<edm::ParameterSet>("Delta_Y_Strip");
    HistoName.str("");
    HistoName << "Delta_Y_Strip";
    if(Parameters.getParameter<bool>("switch"))
      local_mes.deltaYStrip = ibooker.book1D(HistoName.str(),
                                              HistoName.str(),
                                              Parameters.getParameter<int32_t>("NxBins"),
                                              Parameters.getParameter<double>("xmin"),
                                              Parameters.getParameter<double>("xmax"));
    else
      local_mes.deltaYStrip = nullptr;

    Parameters = config_.getParameter<edm::ParameterSet>("Delta_Y_Pixel");
    HistoName.str("");
    HistoName << "Delta_Y_Pixel";
    if(Parameters.getParameter<bool>("switch"))
      local_mes.deltaYPixel = ibooker.book1D(HistoName.str(),
                                              HistoName.str(),
                                              Parameters.getParameter<int32_t>("NxBins"),
                                              Parameters.getParameter<double>("xmin"),
                                              Parameters.getParameter<double>("xmax"));
    else
      local_mes.deltaYPixel = nullptr;


    // Delta position with simhits for primary tracks only

    Parameters = config_.getParameter<edm::ParameterSet>("Delta_X_Strip_P");
    HistoName.str("");
    HistoName << "Delta_X_Strip_P";
    if(Parameters.getParameter<bool>("switch"))
      local_mes.deltaXStripP = ibooker.book1D(HistoName.str(),
                                              HistoName.str(),
                                              Parameters.getParameter<int32_t>("NxBins"),
                                              Parameters.getParameter<double>("xmin"),
                                              Parameters.getParameter<double>("xmax"));
    else
      local_mes.deltaXStripP = nullptr;

    Parameters = config_.getParameter<edm::ParameterSet>("Delta_X_Pixel_P");
    HistoName.str("");
    HistoName << "Delta_X_Pixel_P";
    if(Parameters.getParameter<bool>("switch"))
      local_mes.deltaXPixelP = ibooker.book1D(HistoName.str(),
                                              HistoName.str(),
                                              Parameters.getParameter<int32_t>("NxBins"),
                                              Parameters.getParameter<double>("xmin"),
                                              Parameters.getParameter<double>("xmax"));
    else
      local_mes.deltaXPixelP = nullptr;

    Parameters = config_.getParameter<edm::ParameterSet>("Delta_Y_Strip_P");
    HistoName.str("");
    HistoName << "Delta_Y_Strip_P";
    if(Parameters.getParameter<bool>("switch"))
      local_mes.deltaYStripP = ibooker.book1D(HistoName.str(),
                                              HistoName.str(),
                                              Parameters.getParameter<int32_t>("NxBins"),
                                              Parameters.getParameter<double>("xmin"),
                                              Parameters.getParameter<double>("xmax"));
    else
      local_mes.deltaYStripP = nullptr;

    Parameters = config_.getParameter<edm::ParameterSet>("Delta_Y_Pixel_P");
    HistoName.str("");
    HistoName << "Delta_Y_Pixel_P";
    if(Parameters.getParameter<bool>("switch"))
      local_mes.deltaYPixelP = ibooker.book1D(HistoName.str(),
                                              HistoName.str(),
                                              Parameters.getParameter<int32_t>("NxBins"),
                                              Parameters.getParameter<double>("xmin"),
                                              Parameters.getParameter<double>("xmax"));
    else
      local_mes.deltaYPixelP = nullptr;
    // Information on the Digis per cluster

    Parameters = config_.getParameter<edm::ParameterSet>("Primary_Digis_Pixel");
    HistoName.str("");
    HistoName << "all_Digis_Pixel";
    if(Parameters.getParameter<bool>("switch"))
      local_mes.allDigisPixel = ibooker.book1D(HistoName.str(),
                                              HistoName.str(),
                                              Parameters.getParameter<int32_t>("NxBins"),
                                              Parameters.getParameter<double>("xmin"),
                                              Parameters.getParameter<double>("xmax"));
    else
      local_mes.allDigisPixel = nullptr;

    Parameters = config_.getParameter<edm::ParameterSet>("Primary_Digis_Strip");
    HistoName.str("");
    HistoName << "all_Digis_Strip";
    if(Parameters.getParameter<bool>("switch"))
      local_mes.allDigisStrip = ibooker.book1D(HistoName.str(),
                                              HistoName.str(),
                                              Parameters.getParameter<int32_t>("NxBins"),
                                              Parameters.getParameter<double>("xmin"),
                                              Parameters.getParameter<double>("xmax"));
    else
      local_mes.allDigisStrip = nullptr;


    Parameters = config_.getParameter<edm::ParameterSet>("Primary_Digis_Pixel");
    HistoName.str("");
    HistoName << "Primary_Digis_Pixel";
    if(Parameters.getParameter<bool>("switch"))
      local_mes.primaryDigisPixel = ibooker.book1D(HistoName.str(),
                                              HistoName.str(),
                                              Parameters.getParameter<int32_t>("NxBins"),
                                              Parameters.getParameter<double>("xmin"),
                                              Parameters.getParameter<double>("xmax"));
    else
      local_mes.primaryDigisPixel = nullptr;

    Parameters = config_.getParameter<edm::ParameterSet>("Primary_Digis_Strip");
    HistoName.str("");
    HistoName << "Primary_Digis_Strip";
    if(Parameters.getParameter<bool>("switch"))
      local_mes.primaryDigisStrip = ibooker.book1D(HistoName.str(),
                                              HistoName.str(),
                                              Parameters.getParameter<int32_t>("NxBins"),
                                              Parameters.getParameter<double>("xmin"),
                                              Parameters.getParameter<double>("xmax"));
    else
      local_mes.primaryDigisStrip = nullptr;

    Parameters = config_.getParameter<edm::ParameterSet>("Other_Digis_Pixel");
    HistoName.str("");
    HistoName << "Other_Digis_Pixel";
    if(Parameters.getParameter<bool>("switch"))
      local_mes.otherDigisPixel = ibooker.book1D(HistoName.str(),
                                              HistoName.str(),
                                              Parameters.getParameter<int32_t>("NxBins"),
                                              Parameters.getParameter<double>("xmin"),
                                              Parameters.getParameter<double>("xmax"));
    else
      local_mes.otherDigisPixel = nullptr;

    Parameters = config_.getParameter<edm::ParameterSet>("Other_Digis_Strip");
    HistoName.str("");
    HistoName << "Other_Digis_Strip";
    if(Parameters.getParameter<bool>("switch"))
      local_mes.otherDigisStrip = ibooker.book1D(HistoName.str(),
                                              HistoName.str(),
                                              Parameters.getParameter<int32_t>("NxBins"),
                                              Parameters.getParameter<double>("xmin"),
                                              Parameters.getParameter<double>("xmax"));
    else
      local_mes.otherDigisStrip = nullptr;
    if (layer < 100)
    {
      Parameters = config_.getParameter<edm::ParameterSet>("XYGlobalPositionMapH");
      HistoName.str("");
      HistoName << "XvsYGlobalPosStrip";
      if(Parameters.getParameter<bool>("switch"))
        local_mes.XYGlobalPositionMapStrip = ibooker.book2D(HistoName.str(),
                                                HistoName.str(),
                                                Parameters.getParameter<int32_t>("NxBins"),
                                                Parameters.getParameter<double>("xmin"),
                                                Parameters.getParameter<double>("xmax"),
                                                Parameters.getParameter<int32_t>("NyBins"),
                                                Parameters.getParameter<double>("ymin"),
                                                Parameters.getParameter<double>("ymax"));
      else
        local_mes.XYGlobalPositionMapStrip = nullptr;
      Parameters = config_.getParameter<edm::ParameterSet>("XYGlobalPositionMapH");
      HistoName.str("");
      HistoName << "XvsYGlobalPosPixel";
      if(Parameters.getParameter<bool>("switch"))
        local_mes.XYGlobalPositionMapPixel = ibooker.book2D(HistoName.str(),
                                                HistoName.str(),
                                                Parameters.getParameter<int32_t>("NxBins"),
                                                Parameters.getParameter<double>("xmin"),
                                                Parameters.getParameter<double>("xmax"),
                                                Parameters.getParameter<int32_t>("NyBins"),
                                                Parameters.getParameter<double>("ymin"),
                                                Parameters.getParameter<double>("ymax"));
      else
        local_mes.XYGlobalPositionMapPixel = nullptr;

      Parameters = config_.getParameter<edm::ParameterSet>("XYLocalPositionMapH");
      HistoName.str("");
      HistoName << "XvsYLocalPosStrip";
      if(Parameters.getParameter<bool>("switch"))
        local_mes.XYLocalPositionMapStrip = ibooker.book2D(HistoName.str(),
                                                HistoName.str(),
                                                Parameters.getParameter<int32_t>("NxBins"),
                                                Parameters.getParameter<double>("xmin"),
                                                Parameters.getParameter<double>("xmax"),
                                                Parameters.getParameter<int32_t>("NyBins"),
                                                Parameters.getParameter<double>("ymin"),
                                                Parameters.getParameter<double>("ymax"));
      else
        local_mes.XYLocalPositionMapStrip = nullptr;
      Parameters = config_.getParameter<edm::ParameterSet>("XYLocalPositionMapH");
      HistoName.str("");
      HistoName << "XvsYLocalPosPixel";
      if(Parameters.getParameter<bool>("switch"))
        local_mes.XYLocalPositionMapPixel = ibooker.book2D(HistoName.str(),
                                                HistoName.str(),
                                                Parameters.getParameter<int32_t>("NxBins"),
                                                Parameters.getParameter<double>("xmin"),
                                                Parameters.getParameter<double>("xmax"),
                                                Parameters.getParameter<int32_t>("NyBins"),
                                                Parameters.getParameter<double>("ymin"),
                                                Parameters.getParameter<double>("ymax"));
      else
        local_mes.XYLocalPositionMapPixel = nullptr;
    }

    //local_mes.nCluster = 1;

    layerMEs.insert(std::make_pair(folderName, local_mes));

  }

}



std::string  Phase2TrackerValidateCluster::getHistoId(uint32_t det_id, const TrackerTopology* tTopo, bool flag) {
  int layer;
  std::ostringstream fname1, fname2, fname3;
  if(flag){
    layer = tTopo->getITPixelLayerNumber(det_id);
  }
  else {
    layer = tTopo->getOTLayerNumber(det_id);
  }
  if(layer < 0)
    return "";

  if (layer < 100) {
    fname1 << "Barrel/";
    fname2 << "Layer" << layer;
    fname3 << "";
  } else {
    int side = layer / 100;
    fname1 << "EndCap_Side" << side << "/";
    int disc = layer - side * 100;
    disc = (disc < 3) ? 12 : 345;
    fname2 << "Disc" << disc << "/";
    int ring = tTopo->tidRing(det_id);
    fname3 << "Ring" << ring;
  }
  fname1 << fname2.str() << fname3.str();
  //std::cout << fname1.str() << std::endl;
  return fname1.str();
}  
std::vector<unsigned int> Phase2TrackerValidateCluster::getSimTrackId(
    const edm::Handle<edm::DetSetVector<PixelDigiSimLink> >& pixelSimLinks, const DetId& detId, unsigned int channel) {
  std::vector<unsigned int> retvec;
  edm::DetSetVector<PixelDigiSimLink>::const_iterator DSViter(pixelSimLinks->find(detId));
  if (DSViter == pixelSimLinks->end())
    return retvec;
  for (edm::DetSet<PixelDigiSimLink>::const_iterator it = DSViter->data.begin(); it != DSViter->data.end(); ++it) {
    if (channel == it->channel()) {
      retvec.push_back(it->SimTrackId());
    }
  }
  return retvec;
}
DEFINE_FWK_MODULE(Phase2TrackerValidateCluster);
