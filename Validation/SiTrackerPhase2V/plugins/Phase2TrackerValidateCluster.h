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

// DQM Histograms

class Phase2TrackerValidateCluster : public DQMEDAnalyzer {
public:
  explicit Phase2TrackerValidateCluster(const edm::ParameterSet&);
  ~Phase2TrackerValidateCluster() override;
  void bookHistograms(DQMStore::IBooker& ibooker, edm::Run const& iRun, edm::EventSetup const& iSetup) override;
  void analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) override;

  struct ClusterMEs {
    int nCluster;
  };

private:
  MonitorElement* SimulatedXYPositionMap;

  edm::ParameterSet config_;
  edm::EDGetTokenT<Phase2TrackerCluster1DCollectionNew> clustersToken_;
  std::string geomType_;

};
#endif
