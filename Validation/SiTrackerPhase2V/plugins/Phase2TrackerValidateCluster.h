#ifndef Phase2TrackerValidateCluster_h
#define Phase2TrackerValidateCluster_h

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

#include "SimDataFormats/TrackerDigiSimLink/interface/PixelDigiSimLink.h"
#include "SimDataFormats/Track/interface/SimTrackContainer.h"
#include "SimDataFormats/Vertex/interface/SimVertexContainer.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"

// DQM Histograms

class Phase2TrackerValidateCluster : public DQMEDAnalyzer {
public:
  typedef std::map<unsigned int, std::vector<PSimHit> > SimHitsMap;
  typedef std::map<unsigned int, SimTrack> SimTracksMap;

  explicit Phase2TrackerValidateCluster(const edm::ParameterSet&);
  ~Phase2TrackerValidateCluster() override;
  void bookHistograms(DQMStore::IBooker& ibooker, edm::Run const& iRun, edm::EventSetup const& iSetup) override;
  void analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) override;
  //uint32_t getHistoId(uint32_t det_id, const TrackerTopology* tTopo, bool flag); 
  std::string getHistoId(uint32_t det_id, const TrackerTopology* tTopo, bool flag); 
  std::vector<unsigned int> getSimTrackId(const edm::Handle<edm::DetSetVector<PixelDigiSimLink> >& pixelSimLinks, const DetId& detId, unsigned int channel);

  struct ClusterMEs {
    //uint32_t nCluster;
    MonitorElement* ClusterSize;
    //MonitorElement* ClusterCharge;
    MonitorElement* deltaXStrip;
    MonitorElement* deltaXPixel;
    MonitorElement* deltaYStrip;
    MonitorElement* deltaYPixel;
    MonitorElement* deltaXStripP;
    MonitorElement* deltaXPixelP;
    MonitorElement* deltaYStripP;
    MonitorElement* deltaYPixelP;
    MonitorElement* allDigisStrip;
    MonitorElement* allDigisPixel;
    MonitorElement* primaryDigisStrip;
    MonitorElement* primaryDigisPixel;
    MonitorElement* otherDigisStrip;
    MonitorElement* otherDigisPixel;
    MonitorElement* XYGlobalPositionMapPixel;
    MonitorElement* XYGlobalPositionMapStrip;
    MonitorElement* XYLocalPositionMapPixel;
    MonitorElement* XYLocalPositionMapStrip;
  };

  enum HISTOID {
    Layer1, Layer2, Layer3, Layer4, DISCplusR12, DISCplusR345, DISCminusR12, DISCminusR345
  };
private:
  MonitorElement* SimulatedZRPositionMap;
  MonitorElement* SimulatedXYPositionMap;
  MonitorElement* SimulatedXYBarrelPositionMap;
  MonitorElement* SimulatedXYEndCapPositionMap;


  void bookLayerHistos(DQMStore::IBooker& ibooker, uint32_t det_it, const TrackerTopology* tTopo, bool flag);

  //std::map<uint32_t, ClusterMEs> layerMEs;
  std::map<std::string, ClusterMEs> layerMEs;

  edm::ParameterSet config_;
  bool pixelFlag_;
  bool catECasRings_;
  double simtrackminpt_;
  std::string geomType_;

  edm::EDGetTokenT<edm::DetSetVector<PixelDigiSimLink>> simLinksToken_;
  edm::EDGetTokenT<edm::SimTrackContainer> simTracksToken_;
  edm::EDGetTokenT<Phase2TrackerCluster1DCollectionNew> clustersToken_;
  edm::ESHandle<TrackerTopology> tTopoHandle_;
  edm::EDGetTokenT<edm::PSimHitContainer> simHitsBToken_;
  edm::EDGetTokenT<edm::PSimHitContainer> simHitsEToken_;

};
#endif
