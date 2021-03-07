// -*- C++ -*-
//bookLayer
// Package:    Phase2OTValidateCluster
// Class:      Phase2OTValidateCluster
//
/**\class Phase2OTValidateCluster Phase2OTValidateCluster.cc 

 Description: Validation plots tracker clusters. 

*/
//
// Author: Gabriel Ramirez, Suvankar Roy Chowdhury
// Date: May 23, 2020
//
#include <memory>
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESWatcher.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/Phase2TrackerCluster/interface/Phase2TrackerCluster1D.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"
#include "DataFormats/Common/interface/DetSetVector.h"

#include "SimDataFormats/TrackerDigiSimLink/interface/PixelDigiSimLink.h"
#include "SimDataFormats/Track/interface/SimTrackContainer.h"
#include "SimDataFormats/Vertex/interface/SimVertexContainer.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"

#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/CommonDetUnit/interface/PixelGeomDetUnit.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"
#include "DataFormats/Common/interface/DetSetVector.h"
// DQM Histograming
#include "DQMServices/Core/interface/MonitorElement.h"
#include "DQMServices/Core/interface/DQMEDAnalyzer.h"
#include "DQMServices/Core/interface/DQMStore.h"

class Phase2OTValidateCluster : public DQMEDAnalyzer {
public:
  typedef std::map<unsigned int, std::vector<PSimHit>> SimHitsMap;
  typedef std::map<unsigned int, SimTrack> SimTracksMap;

  explicit Phase2OTValidateCluster(const edm::ParameterSet&);
  ~Phase2OTValidateCluster() override;
  void bookHistograms(DQMStore::IBooker& ibooker, edm::Run const& iRun, edm::EventSetup const& iSetup) override;
  void analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) override;
  void dqmBeginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) override;
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  struct ClusterMEs {
    MonitorElement* deltaX_S = nullptr;
    MonitorElement* deltaX_P = nullptr;
    MonitorElement* deltaY_S = nullptr;
    MonitorElement* deltaY_P = nullptr;
    MonitorElement* pullX_S = nullptr;
    MonitorElement* pullX_P = nullptr;
    MonitorElement* pullY_S = nullptr;
    MonitorElement* pullY_P = nullptr;
    MonitorElement* deltaX_eta_S = nullptr;
    MonitorElement* deltaX_eta_P = nullptr;
    MonitorElement* deltaY_eta_S = nullptr;
    MonitorElement* deltaY_eta_P = nullptr;
    MonitorElement* pullX_eta_S = nullptr;
    MonitorElement* pullX_eta_P = nullptr;
    MonitorElement* pullY_eta_S = nullptr;
    MonitorElement* pullY_eta_P = nullptr;
    MonitorElement* deltaX_P_primary = nullptr;
    MonitorElement* deltaY_P_primary = nullptr;
    MonitorElement* deltaX_S_primary = nullptr;
    MonitorElement* deltaY_S_primary = nullptr;
    MonitorElement* pullX_P_primary = nullptr;
    MonitorElement* pullY_P_primary = nullptr;
    MonitorElement* pullX_S_primary = nullptr;
    MonitorElement* pullY_S_primary = nullptr;
  };

  void fillOTHistos(const edm::Event& iEvent,
                    const std::vector<edm::Handle<edm::PSimHitContainer>>& simHits,
                    const std::map<unsigned int, SimTrack>& simTracks);
  void bookLayerHistos(DQMStore::IBooker& ibooker, uint32_t det_it, const std::string& subdir);
  std::vector<unsigned int> getSimTrackId(const edm::Handle<edm::DetSetVector<PixelDigiSimLink>>& pixelSimLinks,
                                          const DetId& detId,
                                          unsigned int channel);

  std::map<std::string, ClusterMEs> layerMEs_;

  edm::ParameterSet config_;
  double simtrackminpt_;
  std::vector<edm::EDGetTokenT<edm::PSimHitContainer>> simHitTokens_;
  edm::EDGetTokenT<edm::DetSetVector<PixelDigiSimLink>> simOTLinksToken_;
  edm::EDGetTokenT<edm::DetSetVector<PixelDigiSimLink>> simITLinksToken_;
  edm::EDGetTokenT<edm::SimTrackContainer> simTracksToken_;
  edm::EDGetTokenT<Phase2TrackerCluster1DCollectionNew> clustersToken_;
  std::vector<edm::InputTag> pSimHitSrc_;
  const edm::ESGetToken<TrackerGeometry, TrackerDigiGeometryRecord> geomToken_;
  const edm::ESGetToken<TrackerTopology, TrackerTopologyRcd> topoToken_;
  const TrackerGeometry* tkGeom_ = nullptr;
  const TrackerTopology* tTopo_ = nullptr;
};
#include "Validation/SiTrackerPhase2V/interface/TrackerPhase2ValidationUtil.h"
#include "DQM/SiTrackerPhase2/interface/TrackerPhase2DQMUtil.h"
//
// constructors
//
Phase2OTValidateCluster::Phase2OTValidateCluster(const edm::ParameterSet& iConfig)
    : config_(iConfig),
      simtrackminpt_(config_.getParameter<double>("SimTrackMinPt")),
      simOTLinksToken_(consumes<edm::DetSetVector<PixelDigiSimLink>>(
          config_.getParameter<edm::InputTag>("OuterTrackerDigiSimLinkSource"))),
      simTracksToken_(consumes<edm::SimTrackContainer>(config_.getParameter<edm::InputTag>("simtracks"))),
      clustersToken_(
          consumes<Phase2TrackerCluster1DCollectionNew>(config_.getParameter<edm::InputTag>("ClusterSource"))),
      pSimHitSrc_(config_.getParameter<std::vector<edm::InputTag>>("PSimHitSource")),
      geomToken_(esConsumes<TrackerGeometry, TrackerDigiGeometryRecord, edm::Transition::BeginRun>()),
      topoToken_(esConsumes<TrackerTopology, TrackerTopologyRcd, edm::Transition::BeginRun>()) {
  edm::LogInfo("Phase2OTValidateCluster") << ">>> Construct Phase2OTValidateCluster ";
  for (const auto& itag : pSimHitSrc_)
    simHitTokens_.push_back(consumes<edm::PSimHitContainer>(itag));
}

Phase2OTValidateCluster::~Phase2OTValidateCluster() {
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
  edm::LogInfo("Phase2OTValidateCluster") << ">>> Destroy Phase2OTValidateCluster ";
}
//
// -- DQM Begin Run
//
void Phase2OTValidateCluster::dqmBeginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
  edm::ESHandle<TrackerGeometry> geomHandle = iSetup.getHandle(geomToken_);
  tkGeom_ = &(*geomHandle);
  edm::ESHandle<TrackerTopology> tTopoHandle = iSetup.getHandle(topoToken_);
  tTopo_ = tTopoHandle.product();
}

// -- Analyze
//
void Phase2OTValidateCluster::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  // Getting simHits
  std::vector<edm::Handle<edm::PSimHitContainer>> simHits;
  for (const auto& itoken : simHitTokens_) {
    edm::Handle<edm::PSimHitContainer> simHitHandle;
    iEvent.getByToken(itoken, simHitHandle);
    if (!simHitHandle.isValid())
      continue;
    simHits.emplace_back(simHitHandle);
  }
  // Get the SimTracks
  edm::Handle<edm::SimTrackContainer> simTracksRaw;
  iEvent.getByToken(simTracksToken_, simTracksRaw);

  // Rearrange the simTracks for ease of use <simTrackID, simTrack>
  SimTracksMap simTracks;
  for (edm::SimTrackContainer::const_iterator simTrackIt(simTracksRaw->begin()); simTrackIt != simTracksRaw->end();
       ++simTrackIt) {
    if (simTrackIt->momentum().pt() > simtrackminpt_) {
      simTracks.emplace(simTrackIt->trackId(), *simTrackIt);
    }
  }
  fillOTHistos(iEvent, simHits, simTracks);
}

void Phase2OTValidateCluster::fillOTHistos(const edm::Event& iEvent,
                                           const std::vector<edm::Handle<edm::PSimHitContainer>>& simHits,
                                           const std::map<unsigned int, SimTrack>& simTracks) {
  // Getting the clusters
  edm::Handle<Phase2TrackerCluster1DCollectionNew> clusterHandle;
  iEvent.getByToken(clustersToken_, clusterHandle);

  // Getting PixelDigiSimLinks
  edm::Handle<edm::DetSetVector<PixelDigiSimLink>> pixelSimLinksHandle;
  iEvent.getByToken(simOTLinksToken_, pixelSimLinksHandle);

  // Number of clusters
  std::map<std::string, unsigned int> nPrimarySimHits[3];
  std::map<std::string, unsigned int> nOtherSimHits[3];
  for (const auto& DSVItr : *clusterHandle) {
    // Getting the id of detector unit
    uint32_t rawid = DSVItr.detId();
    DetId detId(rawid);
    const GeomDetUnit* geomDetUnit(tkGeom_->idToDetUnit(detId));
    if (!geomDetUnit)
      continue;
    TrackerGeometry::ModuleType mType = tkGeom_->getDetectorType(detId);

    std::string folderkey = phase2tkutil::getOTHistoId(detId, tTopo_);
    for (const auto& clusterItr : DSVItr) {
      MeasurementPoint mpCluster(clusterItr.center(), clusterItr.column() + 0.5);
      Local3DPoint localPosCluster = geomDetUnit->topology().localPosition(mpCluster);

      // Get simTracks from the cluster
      std::vector<unsigned int> clusterSimTrackIds;
      for (unsigned int i(0); i < clusterItr.size(); ++i) {
        unsigned int channel(Phase2TrackerDigi::pixelToChannel(clusterItr.firstRow() + i, clusterItr.column()));
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
      const PSimHit* closestSimHit = nullptr;
      float minx = 10000.;
      // Get the SimHit
      for (const auto& psimhitCont : simHits) {
        for (const auto& simhitIt : *psimhitCont) {
          if (rawid == simhitIt.detUnitId()) {
            auto it = std::lower_bound(clusterSimTrackIds.begin(), clusterSimTrackIds.end(), simhitIt.trackId());
            if (it != clusterSimTrackIds.end() && *it == simhitIt.trackId()) {
              if (!closestSimHit || fabs(simhitIt.localPosition().x() - localPosCluster.x()) < minx) {
                minx = abs(simhitIt.localPosition().x() - localPosCluster.x());
                closestSimHit = &simhitIt;
              }
            }
          }
        }  //end loop over PSimhitcontainers
      }    //end loop over simHits

      if (!closestSimHit)
        continue;
      // only look at simhits from highpT tracks
      auto simTrackIt(simTracks.find(closestSimHit->trackId()));
      if (simTrackIt == simTracks.end())
        continue;

      Local3DPoint localPosSimHit(closestSimHit->localPosition());
      const float deltaX = localPosCluster.x() - localPosSimHit.x();
      const float deltaY = localPosCluster.y() - localPosSimHit.y();
      
      const double pullX = 1;//deltaX / clusterItr.getSplitClusterErrorX();
      const double pullY = 1;//deltaY / clusterItr.getSplitClusterErrorY();

      float eta = geomDetUnit->surface().toGlobal(localPosCluster).eta();

      auto layerMEit = layerMEs_.find(folderkey);
      if (layerMEit == layerMEs_.end())
        continue;

      ClusterMEs& local_mes = layerMEit->second;
      if (mType == TrackerGeometry::ModuleType::Ph2PSP) {
        local_mes.deltaX_P->Fill(deltaX);
        local_mes.deltaY_P->Fill(deltaY);
        local_mes.pullX_P->Fill(pullX);
        local_mes.pullY_P->Fill(pullY);
        local_mes.deltaX_eta_P->Fill(eta, deltaX);
        local_mes.deltaY_eta_P->Fill(eta, deltaY);
        local_mes.pullX_eta_P->Fill(eta, pullX);
        local_mes.pullY_eta_P->Fill(eta, pullY);
      } else if (mType == TrackerGeometry::ModuleType::Ph2PSS || mType == TrackerGeometry::ModuleType::Ph2SS) {
        local_mes.deltaX_S->Fill(deltaX);
        local_mes.deltaY_S->Fill(deltaY);
        local_mes.pullX_S->Fill(pullX);
        local_mes.pullY_S->Fill(pullY);
        local_mes.deltaX_eta_S->Fill(eta, deltaX);
        local_mes.deltaY_eta_S->Fill(eta, deltaY);
        local_mes.pullX_eta_S->Fill(eta, pullX);
        local_mes.pullY_eta_S->Fill(eta, pullY);
      }
      // Primary particles only
      if (phase2tkutil::isPrimary(simTrackIt->second, closestSimHit)) {
        if (mType == TrackerGeometry::ModuleType::Ph2PSP) {
          local_mes.deltaX_P_primary->Fill(deltaX);
          local_mes.deltaY_P_primary->Fill(deltaY);
          local_mes.pullX_P_primary->Fill(pullX);
          local_mes.pullY_P_primary->Fill(pullY);
        } else if (mType == TrackerGeometry::ModuleType::Ph2PSS || mType == TrackerGeometry::ModuleType::Ph2SS) {
          local_mes.deltaX_S_primary->Fill(deltaX);
          local_mes.deltaY_S_primary->Fill(deltaY);
          local_mes.pullX_S_primary->Fill(pullX);
          local_mes.pullY_S_primary->Fill(pullY);
        }
      }
    }
  }
}

//
// -- Book Histograms
//
void Phase2OTValidateCluster::bookHistograms(DQMStore::IBooker& ibooker,
                                             edm::Run const& iRun,
                                             edm::EventSetup const& iSetup) {
  std::string top_folder = config_.getParameter<std::string>("TopFolderName");
  edm::LogInfo("Phase2OTValidateCluster") << " Booking Histograms in: " << top_folder;

  edm::ESWatcher<TrackerDigiGeometryRecord> theTkDigiGeomWatcher;
  if (theTkDigiGeomWatcher.check(iSetup)) {
    for (auto const& det_u : tkGeom_->detUnits()) {
      //Always check TrackerNumberingBuilder before changing this part
      if ((det_u->subDetector() == GeomDetEnumerators::SubDetector::P2PXB ||
           det_u->subDetector() == GeomDetEnumerators::SubDetector::P2PXEC))
        continue;  //continue if Pixel
      uint32_t detId_raw = det_u->geographicalId().rawId();
      bookLayerHistos(ibooker, detId_raw, top_folder);
    }
  }
}

//////////////////Layer Histo/////////////////////////////////
void Phase2OTValidateCluster::bookLayerHistos(DQMStore::IBooker& ibooker, uint32_t det_id, const std::string& subdir) {
  std::string folderName = phase2tkutil::getOTHistoId(det_id, tTopo_);
  if (folderName.empty()) {
    edm::LogWarning("Phase2OTValidateCluster") << ">>>> Invalid histo_id ";
    return;
  }

  if (layerMEs_.find(folderName) == layerMEs_.end()) {
    ibooker.cd();
    edm::LogInfo("Phase2TrackerValidateDigi") << " Booking Histograms in: " << subdir + '/' + folderName;
    ClusterMEs local_mes;
    if (tkGeom_->getDetectorType(det_id) == TrackerGeometry::ModuleType::Ph2PSP) {
      ibooker.setCurrentFolder(subdir + '/' + folderName);

      local_mes.deltaX_P =
          phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("Delta_X_Pixel"), ibooker);

      local_mes.deltaY_P =
          phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("Delta_Y_Pixel"), ibooker);

      local_mes.pullX_P =
          phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("Pull_X_Pixel"), ibooker);

      local_mes.pullY_P =
          phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("Pull_Y_Pixel"), ibooker);

      local_mes.deltaX_eta_P =
          phase2tkutil::book2DFromPSet(config_.getParameter<edm::ParameterSet>("Delta_X_eta_Pixel"), ibooker);

      local_mes.deltaY_eta_P =
          phase2tkutil::book2DFromPSet(config_.getParameter<edm::ParameterSet>("Delta_Y_eta_Pixel"), ibooker);

      local_mes.pullX_eta_P =
          phase2tkutil::book2DFromPSet(config_.getParameter<edm::ParameterSet>("Pull_X_eta_Pixel"), ibooker);

      local_mes.pullY_eta_P =
          phase2tkutil::book2DFromPSet(config_.getParameter<edm::ParameterSet>("Pull_Y_eta_Pixel"), ibooker);

      // Puting primary digis in a subfolder
      ibooker.setCurrentFolder(subdir + '/' + folderName + "/PrimarySimHits");

      local_mes.deltaX_P_primary =
          phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("Delta_X_Pixel_Primary"), ibooker);

      local_mes.deltaY_P_primary =
          phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("Delta_Y_Pixel_Primary"), ibooker);

      local_mes.pullX_P_primary =
          phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("Pull_X_Pixel_Primary"), ibooker);

      local_mes.pullY_P_primary =
          phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("Pull_Y_Pixel_Primary"), ibooker);
    }
    ibooker.setCurrentFolder(subdir + '/' + folderName);

    local_mes.deltaX_S =
        phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("Delta_X_Strip"), ibooker);

    local_mes.deltaY_S =
        phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("Delta_Y_Strip"), ibooker);

    local_mes.pullX_S =
        phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("Pull_X_Strip"), ibooker);

    local_mes.pullY_S =
        phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("Pull_Y_Strip"), ibooker);

    local_mes.deltaX_eta_S =
        phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("Delta_X_eta_Strip"), ibooker);

    local_mes.deltaY_eta_S =
        phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("Delta_Y_eta_Strip"), ibooker);

    local_mes.pullX_eta_S =
        phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("Pull_X_eta_Strip"), ibooker);

    local_mes.pullY_eta_S =
        phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("Pull_Y_eta_Strip"), ibooker);

    // Puting primary digis in a subfolder
    ibooker.setCurrentFolder(subdir + '/' + folderName + "/PrimarySimHits");

    local_mes.deltaX_S_primary =
        phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("Delta_X_Strip_Primary"), ibooker);

    local_mes.deltaY_S_primary =
        phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("Delta_Y_Strip_Primary"), ibooker);

    local_mes.pullX_S_primary =
        phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("Pull_X_Strip_Primary"), ibooker);

    local_mes.pullY_S_primary =
        phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("Pull_Y_Strip_Primary"), ibooker);

    layerMEs_.emplace(folderName, local_mes);
  }
}

std::vector<unsigned int> Phase2OTValidateCluster::getSimTrackId(
    const edm::Handle<edm::DetSetVector<PixelDigiSimLink>>& pixelSimLinks, const DetId& detId, unsigned int channel) {
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

void Phase2OTValidateCluster::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  //for macro-pixel sensors
  std::string mptag = "macro-pixel sensor";
  std::string striptag = "strip sensor";
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Delta_X_Pixel");
    psd0.add<std::string>("title", "#Delta X " + mptag + ";Cluster resolution X dimension;");
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmax", 5.0);
    psd0.add<double>("xmin", -5.0);
    psd0.add<int>("NxBins", 100);
    desc.add<edm::ParameterSetDescription>("Delta_X_Pixel", psd0);
  }
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Delta_Y_Pixel");
    psd0.add<std::string>("title", "#Delta Y " + mptag + ";Cluster resolution Y dimension;");
    psd0.add<double>("xmin", -5.0);
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmax", 5.0);
    psd0.add<int>("NxBins", 100);
    desc.add<edm::ParameterSetDescription>("Delta_Y_Pixel", psd0);
  }
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Pull_X_Pixel");
    psd0.add<std::string>("title", "Pull X " + mptag + ";pull x;");
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmax", 5.0);
    psd0.add<double>("xmin", -5.0);
    psd0.add<int>("NxBins", 100);
    desc.add<edm::ParameterSetDescription>("Pull_X_Pixel", psd0);
  }
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Pull_Y_Pixel");
    psd0.add<std::string>("title", "Pull Y " + mptag + ";pull y;");
    psd0.add<double>("xmin", -5.0);
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmax", 5.0);
    psd0.add<int>("NxBins", 100);
    desc.add<edm::ParameterSetDescription>("Pull_Y_Pixel", psd0);
  }
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Delta_X_eta_Pixel");
    psd0.add<std::string>("title", "#Delta X vs Eta" + mptag + ";Cluster resolution X dimension;");
    psd0.add<int>("NxBins", 100);
    psd0.add<int>("NyBins", 100);
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmax", 5.0);
    psd0.add<double>("xmin", -5.0);
    psd0.add<double>("ymin", -4.0);
    psd0.add<double>("ymax", 4.0);
    desc.add<edm::ParameterSetDescription>("Delta_X_eta_Pixel", psd0);
  }
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Delta_Y_eta_Pixel");
    psd0.add<std::string>("title", "#Delta Y vs Eta" + mptag + ";Cluster resolution Y dimension;");
    psd0.add<int>("NxBins", 100);
    psd0.add<int>("NyBins", 100);
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmin", -5.0);
    psd0.add<double>("xmax", 5.0);
    psd0.add<double>("ymin", -4.0);
    psd0.add<double>("ymax", 4.0);
    desc.add<edm::ParameterSetDescription>("Delta_Y_eta_Pixel", psd0);
  }
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Pull_X_eta_Pixel");
    psd0.add<std::string>("title", "Pull X vs Eta" + mptag + ";pull x;");
    psd0.add<int>("NxBins", 100);
    psd0.add<int>("NyBins", 100);
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmax", 5.0);
    psd0.add<double>("xmin", -5.0);
    psd0.add<double>("ymin", -4.0);
    psd0.add<double>("ymax", 4.0);
    desc.add<edm::ParameterSetDescription>("Pull_X_eta_Pixel", psd0);
  }
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Pull_Y_eta_Pixel");
    psd0.add<std::string>("title", "Pull Y vs Eta" + mptag + ";pull y;");
    psd0.add<int>("NxBins", 100);
    psd0.add<int>("NyBins", 100);
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmin", -5.0);
    psd0.add<double>("xmax", 5.0);
    psd0.add<double>("ymin", -4.0);
    psd0.add<double>("ymax", 4.0);
    desc.add<edm::ParameterSetDescription>("Pull_Y_eta_Pixel", psd0);
  }
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Delta_X_Pixel_Primary");
    psd0.add<std::string>("title", "#Delta X " + mptag + ";cluster resolution X dimension");
    psd0.add<double>("xmin", -5.0);
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmax", 5.0);
    psd0.add<int>("NxBins", 100);
    desc.add<edm::ParameterSetDescription>("Delta_X_Pixel_Primary", psd0);
  }
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Delta_Y_Pixel_Primary");
    psd0.add<std::string>("title", "#Delta Y " + mptag + ";cluster resolution Y dimension");
    psd0.add<double>("xmin", -5.0);
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmax", 5.0);
    psd0.add<int>("NxBins", 100);
    desc.add<edm::ParameterSetDescription>("Delta_Y_Pixel_Primary", psd0);
  }
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Pull_X_Pixel_Primary");
    psd0.add<std::string>("title", "Pull X " + mptag + ";pull y");
    psd0.add<double>("xmin", -5.0);
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmax", 5.0);
    psd0.add<int>("NxBins", 100);
    desc.add<edm::ParameterSetDescription>("Pull_X_Pixel_Primary", psd0);
  }
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Pull_Y_Pixel_Primary");
    psd0.add<std::string>("title", "Pull Y " + mptag + ";pull y");
    psd0.add<double>("xmin", -5.0);
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmax", 5.0);
    psd0.add<int>("NxBins", 100);
    desc.add<edm::ParameterSetDescription>("Pull_Y_Pixel_Primary", psd0);
  }

  //strip sensors
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Delta_X_Strip");
    psd0.add<std::string>("title", "#Delta X " + striptag + ";Cluster resolution X dimension");
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmin", -5.0);
    psd0.add<double>("xmax", 5.0);
    psd0.add<int>("NxBins", 100);
    desc.add<edm::ParameterSetDescription>("Delta_X_Strip", psd0);
  }
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Delta_Y_Strip");
    psd0.add<std::string>("title", "#Delta Y " + striptag + ";Cluster resolution Y dimension");
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmin", -5.0);
    psd0.add<double>("xmax", 5.0);
    psd0.add<int>("NxBins", 100);
    desc.add<edm::ParameterSetDescription>("Delta_Y_Strip", psd0);
  }
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Pull_X_Strip");
    psd0.add<std::string>("title", "Pull X " + striptag + ";pull x");
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmin", -5.0);
    psd0.add<double>("xmax", 5.0);
    psd0.add<int>("NxBins", 100);
    desc.add<edm::ParameterSetDescription>("Pull_X_Strip", psd0);
  }
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Pull_Y_Strip");
    psd0.add<std::string>("title", "Pull Y " + striptag + ";pull y");
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmin", -5.0);
    psd0.add<double>("xmax", 5.0);
    psd0.add<int>("NxBins", 100);
    desc.add<edm::ParameterSetDescription>("Pull_Y_Strip", psd0);
  }
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Delta_X_eta_Strip");
    psd0.add<std::string>("title", "#Delta X vs Eta" + striptag + ";Cluster resolution X dimension;");
    psd0.add<int>("NxBins", 100);
    psd0.add<int>("NyBins", 100);
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmin", -5.0);
    psd0.add<double>("xmax", 5.0);
    psd0.add<double>("ymin", -4.0);
    psd0.add<double>("ymax", 4.0);
    desc.add<edm::ParameterSetDescription>("Delta_X_eta_Strip", psd0);
  }
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Delta_Y_eta_Strip");
    psd0.add<std::string>("title", "#Delta Y vs Eta" + striptag + ";Cluster resolution Y dimension;");
    psd0.add<int>("NxBins", 100);
    psd0.add<int>("NyBins", 100);
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmin", -5.0);
    psd0.add<double>("xmax", 5.0);
    psd0.add<double>("ymin", -4.0);
    psd0.add<double>("ymax", 4.0);
    desc.add<edm::ParameterSetDescription>("Delta_Y_eta_Strip", psd0);
  }
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Pull_X_eta_Strip");
    psd0.add<std::string>("title", "Pull X vs Eta" + striptag + ";pull x;");
    psd0.add<int>("NxBins", 100);
    psd0.add<int>("NyBins", 100);
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmin", -5.0);
    psd0.add<double>("xmax", 5.0);
    psd0.add<double>("ymin", -4.0);
    psd0.add<double>("ymax", 4.0);
    desc.add<edm::ParameterSetDescription>("Pull_X_eta_Strip", psd0);
  }
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Pull_Y_eta_Strip");
    psd0.add<std::string>("title", "Pull Y vs Eta" + striptag + ";pull y;");
    psd0.add<int>("NxBins", 100);
    psd0.add<int>("NyBins", 100);
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmin", -5.0);
    psd0.add<double>("xmax", 5.0);
    psd0.add<double>("ymin", -4.0);
    psd0.add<double>("ymax", 4.0);
    desc.add<edm::ParameterSetDescription>("Pull_Y_eta_Strip", psd0);
  }
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Delta_X_Strip_Primary");
    psd0.add<std::string>("title", "#Delta X " + striptag + ";Cluster resolution X dimension");
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmin", -5.0);
    psd0.add<double>("xmax", 5.0);
    psd0.add<int>("NxBins", 100);
    desc.add<edm::ParameterSetDescription>("Delta_X_Strip_Primary", psd0);
  }
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Delta_Y_Strip_Primary");
    psd0.add<std::string>("title", "#Delta Y " + striptag + ";Cluster resolution Y dimension");
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmin", -5.0);
    psd0.add<double>("xmax", 5.0);
    psd0.add<int>("NxBins", 100);
    desc.add<edm::ParameterSetDescription>("Delta_Y_Strip_Primary", psd0);
  }
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Pull_X_Strip_Primary");
    psd0.add<std::string>("title", "Pull X " + striptag + ";pull x");
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmin", -5.0);
    psd0.add<double>("xmax", 5.0);
    psd0.add<int>("NxBins", 100);
    desc.add<edm::ParameterSetDescription>("Pull_X_Strip_Primary", psd0);
  }
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::string>("name", "Pull_Y_Strip_Primary");
    psd0.add<std::string>("title", "Pull Y " + striptag + ";pull y");
    psd0.add<bool>("switch", true);
    psd0.add<double>("xmin", -5.0);
    psd0.add<double>("xmax", 5.0);
    psd0.add<int>("NxBins", 100);
    desc.add<edm::ParameterSetDescription>("Pull_Y_Strip_Primary", psd0);
  }
  desc.add<std::string>("TopFolderName", "TrackerPhase2OTClusterV");
  desc.add<edm::InputTag>("ClusterSource", edm::InputTag("siPhase2Clusters"));
  desc.add<edm::InputTag>("OuterTrackerDigiSimLinkSource", edm::InputTag("simSiPixelDigis", "Tracker"));
  desc.add<edm::InputTag>("simtracks", edm::InputTag("g4SimHits"));
  desc.add<double>("SimTrackMinPt", 0.0);
  desc.add<std::vector<edm::InputTag>>("PSimHitSource",
                                       {
                                           edm::InputTag("g4SimHits:TrackerHitsTIBLowTof"),
                                           edm::InputTag("g4SimHits:TrackerHitsTIBHighTof"),
                                           edm::InputTag("g4SimHits:TrackerHitsTIDLowTof"),
                                           edm::InputTag("g4SimHits:TrackerHitsTIDHighTof"),
                                           edm::InputTag("g4SimHits:TrackerHitsTOBLowTof"),
                                           edm::InputTag("g4SimHits:TrackerHitsTOBHighTof"),
                                           edm::InputTag("g4SimHits:TrackerHitsTECLowTof"),
                                           edm::InputTag("g4SimHits:TrackerHitsTECHighTof"),
                                           edm::InputTag("g4SimHits:TrackerHitsPixelBarrelLowTof"),
                                           edm::InputTag("g4SimHits:TrackerHitsPixelBarrelHighTof"),
                                           edm::InputTag("g4SimHits:TrackerHitsPixelEndcapLowTof"),
                                           edm::InputTag("g4SimHits:TrackerHitsPixelEndcapHighTof"),
                                       });
  descriptions.add("Phase2OTValidateCluster", desc);
}
DEFINE_FWK_MODULE(Phase2OTValidateCluster);
