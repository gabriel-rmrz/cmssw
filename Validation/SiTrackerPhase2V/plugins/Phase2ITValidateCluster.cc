// -*- C++ -*-
//bookLayer
// Package:    Phase2ITValidateCluster
// Class:      Phase2ITValidateCluster
//
/**\class Phase2ITValidateCluster Phase2ITValidateCluster.cc 

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
#include "Validation/SiTrackerPhase2V/plugins/Phase2ITValidateCluster.h"

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

#include "DataFormats/Common/interface/DetSetVectorNew.h"
#include "DataFormats/Common/interface/DetSetVector.h"


// DQM Histograming
#include "DQMServices/Core/interface/MonitorElement.h"

//
// constructors
//

Phase2ITValidateCluster::Phase2ITValidateCluster(const edm::ParameterSet& iConfig)
    : config_(iConfig),
      catECasRings_(config_.getParameter<bool>("ECasRings")),
      simtrackminpt_(config_.getParameter<double>("SimTrackMinPt")),
      geomType_(config_.getParameter<std::string>("GeometryType")),
      //itPixelClusterSrc_(config_.getParameter<edm::InputTag>("InnerPixelClusterSource")),
      simITLinksToken_(consumes<edm::DetSetVector<PixelDigiSimLink>>(config_.getParameter<edm::InputTag>("InnerPixelDigiSimLinkSource"))),
      simTracksToken_(consumes<edm::SimTrackContainer>(config_.getParameter<edm::InputTag>("simtracks"))),
      itPixelClusterToken_(consumes<edmNew::DetSetVector<SiPixelCluster>>(config_.getParameter<edm::InputTag>("InnerPixelClusterSource"))),
      pSimHitSrc_(config_.getParameter<std::vector<edm::InputTag> >("PSimHitSource"))
{
  edm::LogInfo("Phase2ITValidateCluster") << ">>> Construct Phase2ITValidateCluster ";
  for (const auto& itag : pSimHitSrc_)
        simHitTokens_.push_back(consumes<edm::PSimHitContainer>(itag));
}
    
Phase2ITValidateCluster::~Phase2ITValidateCluster() {
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
  edm::LogInfo("Phase2ITValidateCluster") << ">>> Destroy Phase2ITValidateCluster ";
}
//
// -- DQM Begin Run
//
// -- Analyze
//
void Phase2ITValidateCluster::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  // Getting the clusters
  edm::Handle<edmNew::DetSetVector<SiPixelCluster>> itPixelClusterHandle;
  iEvent.getByToken(itPixelClusterToken_, itPixelClusterHandle);

  // Getting PixelDigiSimLinks
  edm::Handle<edm::DetSetVector<PixelDigiSimLink> > pixelSimLinksHandle;

  iEvent.getByToken(simITLinksToken_, pixelSimLinksHandle);


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
  const TrackerGeometry* tkGeom = geomHandle.product();//&(*geomHandle);

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
  std::map<std::string, unsigned int> nClusters;
  std::map<std::string, unsigned int> nPrimarySimHits;
  std::map<std::string, unsigned int> nOtherSimHits;
  
  for(edmNew::DetSetVector<SiPixelCluster>::const_iterator DSVItr = itPixelClusterHandle->begin(); DSVItr != itPixelClusterHandle->end(); ++DSVItr){
    // Getting the id of detector unit
    uint32_t rawid(DSVItr->detId());
    //uint32_t rawid(DSVItr->id());
    DetId detId(rawid);


    ///////////////////////////
    // Getting the geometry of the detector unit
    ///////////////////////////

    const GeomDet* geomDet = tkGeom->idToDet(detId);
    if(!geomDet)
      continue;
    const GeomDetUnit* geomDetUnit(tkGeom->idToDetUnit(detId));
    if(!geomDetUnit) continue;


    TrackerGeometry::ModuleType mType = tkGeom->getDetectorType(detId);

    if (!(mType == TrackerGeometry::ModuleType::Ph2PXB || mType == TrackerGeometry::ModuleType::Ph2PXF)) {
      std::cout << "UNKNOWN DETECTOR TYPE!" << std::endl;
    }

    std::string folderkey = getHistoId(detId, tTopo);

    // initialize the nhit counters if they don't exist for this layer
    auto nhitit(nClusters.find(folderkey));
    if (nhitit == nClusters.end()) {
      nClusters.emplace(folderkey, 0);
      nPrimarySimHits.emplace(folderkey, 0);
      nOtherSimHits.emplace(folderkey, 0);
    }


    for(edmNew::DetSet<SiPixelCluster>::const_iterator clusterItr = DSVItr->begin(); clusterItr != DSVItr->end(); ++clusterItr){

      MeasurementPoint mpCluster(clusterItr->x(), clusterItr->y());
      Local3DPoint localPosCluster = geomDetUnit->topology().localPosition(mpCluster);
      Global3DPoint globalPosCluster = geomDetUnit->surface().toGlobal(localPosCluster);

      /////////////////////////
      // Find the closest simhit
      /////////////////////////

      // Get simTracks from the cluster
      std::vector<unsigned int> clusterSimTrackIds;
      for(int i(0); i < clusterItr->size(); ++i){
        SiPixelCluster::Pixel ipix = clusterItr->pixel(i);

        unsigned int channel(Phase2TrackerDigi::pixelToChannel(ipix.x, ipix.y));
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

  // Get the SimHits
     for (const auto& itoken : simHitTokens_) {
        edm::Handle<edm::PSimHitContainer> simHitHandle;
        iEvent.getByToken(itoken, simHitHandle);
        if (!simHitHandle.isValid())
          continue;
        const edm::PSimHitContainer& simHits = (*simHitHandle.product());


        for (unsigned int simhitidx = 0; simhitidx < 2; ++simhitidx) {  // loop over both barrel and endcap hits
          for (edm::PSimHitContainer::const_iterator isim = simHits.begin(); isim != simHits.end(); ++isim) {
            const PSimHit& simhitIt = (*isim);
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
      }
      if (!closestSimHit)
        continue;
       // only look at simhits from highpT tracks
      auto simTrackIt(simTracks.find(closestSimHit->trackId()));
      if (simTrackIt == simTracks.end())
         continue;
      Local3DPoint localPosHit(closestSimHit->localPosition());

      // cluster size
      ++(nClusters.at(folderkey));
      ++(nOtherSimHits.at(folderkey));


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
      local_mes.XYGlobalPositionMapPixel->Fill(globalPosCluster.z(), globalPosCluster.perp());
      local_mes.XYLocalPositionMapPixel->Fill(localPosCluster.x(), localPosCluster.y());
      if(local_mes.ClusterSize)
        local_mes.ClusterSize->Fill(clusterItr->size());

      local_mes.deltaXPixel->Fill(localPosCluster.x() - localPosHit.x());
      local_mes.deltaYPixel->Fill(localPosCluster.y() - localPosHit.y());
         // Primary particles only
      if(isPrimary(simTrackIt->second, closestSimHit)) {
        ++(nPrimarySimHits.at(folderkey));
        --(nOtherSimHits.at(folderkey));  // avoid double counting
        local_mes.deltaXPixelP->Fill(localPosCluster.x() - localPosHit.x());
        local_mes.deltaYPixelP->Fill(localPosCluster.y() - localPosHit.y());
      }
    }
  }
  for (auto it : nClusters) {
    auto pos = layerMEs.find(it.first);
    
    if (pos == layerMEs.end())
    {
      std::cout << "*** SL *** No histogram for an existing counter! This should not happen!" << std::endl;
      continue;
    }
    ClusterMEs& local_mes = pos->second;
    local_mes.allDigisPixel->Fill(it.second);
  }
  for (auto it : nPrimarySimHits) {
    auto pos = layerMEs.find(it.first);
    if (pos == layerMEs.end())
    {
      std::cout << "*** SL *** No histogram for an existing counter! This should not happen!" << std::endl;
      continue;
    }
    ClusterMEs& local_mes = pos->second;
    local_mes.primaryDigisPixel->Fill(it.second);
  }
  for (auto it : nOtherSimHits) {
    auto pos = layerMEs.find(it.first);
    if (pos == layerMEs.end())
    {
      std::cout << "*** SL *** No histogram for an existing counter! This should not happen!" << std::endl;
      continue;
    }
    ClusterMEs& local_mes = pos->second;
    local_mes.otherDigisPixel->Fill(it.second);
  }
  

  return;
}

//
// -- Book Histograms
//
void Phase2ITValidateCluster::bookHistograms(DQMStore::IBooker& ibooker, 
                                                  edm::Run const& iRun, 
                                                  edm::EventSetup const& iSetup){
  std::string top_folder = config_.getParameter<std::string>("TopFolderName");
  std::stringstream folder_name;

  ibooker.cd();
  folder_name << top_folder << "/"
    << "SimClusterInfo";
  ibooker.setCurrentFolder(folder_name.str());

  edm::LogInfo("Phase2ITValidateCluster") << " Booking Histograms in: " << folder_name.str();
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
      bookLayerHistos(ibooker, detId_raw, tTopo);
    }    
  }
}

//////////////////Layer Histo/////////////////////////////////
void Phase2ITValidateCluster::bookLayerHistos(DQMStore::IBooker& ibooker,
                                                   uint32_t det_id,
                                                   const TrackerTopology* tTopo) {
  std::string folderName = getHistoId(det_id, tTopo);
  if(folderName == ""){
    edm::LogInfo("Phase2ITValidateCluster") << ">>>> Invalid histo_id ";
    return;
  }
  
  std::map<std::string, ClusterMEs>::iterator pos = layerMEs.find(folderName);

  if(pos == layerMEs.end()){
    std::string top_folder = config_.getParameter<std::string>("TopFolderName");

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
    

    //local_mes.nCluster = 1;

    layerMEs.insert(std::make_pair(folderName, local_mes));

  }

}


bool Phase2ITValidateCluster::isPrimary(const SimTrack& simTrk, const PSimHit* simHit){
  bool retval = false;
  unsigned int trkId = simTrk.trackId();
  if (trkId != simHit->trackId())
    return retval;
  int vtxIndex = simTrk.vertIndex();
  int ptype = simHit->processType();
  if ((vtxIndex == 0) && (ptype == 0))
    retval = true;
  return retval;
}

std::string  Phase2ITValidateCluster::getHistoId(uint32_t det_id, const TrackerTopology* tTopo) {
  int layer;
  std::ostringstream fname1, fname2, fname3;
  layer = tTopo->getITPixelLayerNumber(det_id);

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
    fname2 << "Disc" << disc;
  }
  fname1 << fname2.str() << fname3.str();
  return fname1.str();
}  
std::vector<unsigned int> Phase2ITValidateCluster::getSimTrackId(
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
DEFINE_FWK_MODULE(Phase2ITValidateCluster);
