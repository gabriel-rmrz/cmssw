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

// DQM Histograms

class Phase2TrackerValidateCluster : public DQMEDAnalyzer {
public:
  explicit Phase2TrackerValidateCluster(const edm::ParameterSet&);
  ~Phase2TrackerValidateCluster() override;
  void bookHistograms(DQMStore::IBooker& ibooker, edm::Run const& iRun, edm::EventSetup const& iSetup) override;
  void anlyze(const edm::Event& iEvent, const edn::EventSetup& iSetup) override;

private:

}
