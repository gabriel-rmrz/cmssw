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

// DQM Histograms

class Phase2TrackerValidateCluster : public DQMEDAnalyzer {
public:
  explicit Phase2TrackerValidateCluster(const edm::ParameterSet&);
  ~Phase2TrackerValidateCluster() override;
  void bookHistograms(DQMStore::IBooker& ibooker, edm::Run const& iRun, edm::EventSetup const& iSetup) override;
  void analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) override;

  struct ClusterMEs {
    uint32_t nCluster;
    MonitorElement* ZRPositionMap;
    MonitorElement* ClusterSize;
    MonitorElement* ClusterCharge;
  };

private:
  MonitorElement* SimulatedZRPositionMap;
  MonitorElement* SimulatedXYPositionMap;
  MonitorElement* SimulatedXYBarrelPositionMap;
  MonitorElement* SimulatedXYEndCapPositionMap;


  void bookLayerHistos(DQMStore::IBooker& ibooker, uint32_t det_it, const TrackerTopology* tTopo, bool flag);
  uint32_t getHistoId(uint32_t det_id, const TrackerTopology* tTopo, bool flag); 

  std::map<uint32_t, ClusterMEs> layerMEs;

  edm::ParameterSet config_;
  bool pixelFlag_;
  bool catECasRings_;
  std::string geomType_;

  edm::EDGetTokenT<Phase2TrackerCluster1DCollectionNew> clustersToken_;
  edm::ESHandle<TrackerTopology> tTopoHandle_;

};
#endif
