#ifndef Phase2ITValidateCluster_h
#define Phase2ITValidateCluster_h

#include "DQMServices/Core/interface/DQMEDAnalyzer.h"
#include "DQMServices/Core/interface/DQMStore.h"

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/Event.h"

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/DetId/interface/DetId.h"


#include "DataFormats/Phase2TrackerCluster/interface/Phase2TrackerCluster1D.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"
#include "DataFormats/Common/interface/DetSetVector.h"
#include "DataFormats/SiPixelCluster/interface/SiPixelCluster.h"

#include "SimDataFormats/TrackerDigiSimLink/interface/PixelDigiSimLink.h"
#include "SimDataFormats/Track/interface/SimTrackContainer.h"
#include "SimDataFormats/Vertex/interface/SimVertexContainer.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"

// DQM Histograms

class Phase2ITValidateCluster : public DQMEDAnalyzer {
public:
  typedef std::map<unsigned int, std::vector<PSimHit> > SimHitsMap;
  typedef std::map<unsigned int, SimTrack> SimTracksMap;

  explicit Phase2ITValidateCluster(const edm::ParameterSet&);
  ~Phase2ITValidateCluster() override;
  void bookHistograms(DQMStore::IBooker& ibooker, edm::Run const& iRun, edm::EventSetup const& iSetup) override;
  void analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) override;
  bool isPrimary(const SimTrack& simTrk, const PSimHit* simHit);
  std::string getHistoId(uint32_t det_id, const TrackerTopology* tTopo); 
  std::vector<unsigned int> getSimTrackId(const edm::Handle<edm::DetSetVector<PixelDigiSimLink> >& pixelSimLinks, const DetId& detId, unsigned int channel);

  struct ClusterMEs {
    MonitorElement* ClusterSize=nullptr;
    MonitorElement* deltaXStrip=nullptr;
    MonitorElement* deltaXPixel=nullptr;
    MonitorElement* deltaYStrip=nullptr;
    MonitorElement* deltaYPixel=nullptr;
    MonitorElement* deltaXStripP=nullptr;
    MonitorElement* deltaXPixelP=nullptr;
    MonitorElement* deltaYStripP=nullptr;
    MonitorElement* deltaYPixelP=nullptr;
    MonitorElement* allDigisStrip=nullptr;
    MonitorElement* allDigisPixel=nullptr;
    MonitorElement* primaryDigisStrip=nullptr;
    MonitorElement* primaryDigisPixel=nullptr;
    MonitorElement* otherDigisStrip=nullptr;
    MonitorElement* otherDigisPixel=nullptr;
    MonitorElement* XYGlobalPositionMapPixel=nullptr;
    MonitorElement* XYGlobalPositionMapStrip=nullptr;
    MonitorElement* XYLocalPositionMapPixel=nullptr;
    MonitorElement* XYLocalPositionMapStrip=nullptr;
  };

  enum HISTOID {
    Layer1, Layer2, Layer3, Layer4, DISCplusR12, DISCplusR345, DISCminusR12, DISCminusR345
  };
private:
  MonitorElement* SimulatedZRPositionMap;
  MonitorElement* SimulatedXYPositionMap;
  MonitorElement* SimulatedXYBarrelPositionMap;
  MonitorElement* SimulatedXYEndCapPositionMap;


  void bookLayerHistos(DQMStore::IBooker& ibooker, uint32_t det_it, const TrackerTopology* tTopo);

  std::map<std::string, ClusterMEs> layerMEs;

  edm::ParameterSet config_;
  bool catECasRings_;
  double simtrackminpt_;
  std::string geomType_;
  std::vector<edm::EDGetTokenT<edm::PSimHitContainer> > simHitTokens_;

  edm::EDGetTokenT<edm::DetSetVector<PixelDigiSimLink>> simITLinksToken_;
  edm::EDGetTokenT<edm::SimTrackContainer> simTracksToken_;
  edm::EDGetTokenT<edmNew::DetSetVector<SiPixelCluster>> itPixelClusterToken_;
  std::vector<edm::InputTag> pSimHitSrc_;
  edm::ESHandle<TrackerTopology> tTopoHandle_;
};
#endif
