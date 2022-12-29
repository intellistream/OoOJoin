// Copyright (C) 2021 by the IntelliStream team (https://github.com/intellistream)

/**
 * @brief This is the main entry point of the entire program.
 * We use this as the entry point for benchmarking.
 *
 */
//#include <Utils/Logger.hpp>
#include <vector>
#include <OoOJoin.h>
#include <cmath>

#include <chrono>
#include <iostream>
#include <source_location>
#include <ctime>

using namespace std;
using namespace OoOJoin;
/**
 * @brief run the test bench and allow adjusting
 * @param configName
 * @param outPrefix
 * @note Require configs for this function:
 * - "windowLenMs" U64 The real world window length in ms
 * - "timeStepUs" U64 The simulation step in us
 * - "watermarkPeriodMs" U64 The real world watermark generation period in ms
 * - "maxArrivalSkewMs" U64 The maximum real-world arrival skewness in ms
 * - "eventRateKTps" U64 The real-world rate of spawn event, in KTuples/s
 * - "keyRange" U64 The range of Key
 * - "operator" String The operator to be used
 */
void runTestBenchAdj(string configName = "config.csv", string outPrefix = "") {
  //IntelliLog::log("iNFO","Load global config from " + configName + ", output prefix = " + outPrefix + "\n");
  INTELLI_INFO("Load global config from" + configName + ", output prefix = " + outPrefix);
  OperatorTablePtr opTable = newOperatorTable();
  //IAWJOperatorPtr iawj = newIAWJOperator();
  //get config
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  size_t testSize = 0;
  size_t OoORu = 0, realRu = 0;
  //load global configs
  tsType windowLenMs, timeStepUs, watermarkPeriodMs,maxArrivalSkewMs;
  string operatorTag = "IAWJ";
  string loaderTag = "random";
  //uint64_t keyRange;

  windowLenMs = cfg->tryU64 ("windowLenMs", 10);
  timeStepUs = cfg->tryU64( "timeStepUs", 40);
  watermarkPeriodMs = cfg->tryU64("watermarkPeriodMs", 10);
  maxArrivalSkewMs = cfg->tryU64("maxArrivalSkewMs", 10 / 2);
 // eventRateKTps = tryU64(cfg, "eventRateKTps", 10);
  //keyRange = tryU64(cfg, "keyRange", 10);
  operatorTag = cfg->tryString( "operator", "IAWJ");
  loaderTag=cfg->tryString("dataLoader","random");
  AbstractOperatorPtr iawj = opTable->findOperator(operatorTag);
  if (iawj == nullptr) {
    iawj = newIAWJOperator();
    // WM_WARNNING("No " + operatorTag + " operator, will use IAWJ instead");
  }
  cfg->edit("windowLen", (uint64_t) windowLenMs * 1000);
  cfg->edit("watermarkPeriod", (uint64_t) watermarkPeriodMs * 1000);
  cfg->edit("timeStep", (uint64_t) timeStepUs);
  TestBench tb, tbOoO;
  INTELLI_INFO("/****run OoO test of " + to_string(testSize) + " tuples***/");

  tbOoO.setDataLoader(loaderTag,cfg);
  cfg->edit("rLen", (uint64_t) tbOoO.sizeOfS());
  cfg->edit("sLen", (uint64_t) tbOoO.sizeOfR());
  tbOoO.setOperator(iawj, cfg);
  OoORu = tbOoO.OoOTest(true);
  INTELLI_DEBUG("OoO Confirmed joined " + to_string(OoORu));
  INTELLI_DEBUG("OoO AQP joined " + to_string(tbOoO.AQPResult));
  ConfigMap generalStatistics;
  generalStatistics.edit("AvgLatency", (double) tbOoO.getAvgLatency());
  generalStatistics.edit("95%Latency", (double) tbOoO.getLatencyPercentage(0.95));
  generalStatistics.edit("Throughput", (double) tbOoO.getThroughput());
  // tbOoO.logRTuples();
  // INTELLI_DEBUG("Average latency (us)=" << tbOoO.getAvgLatency());
  INTELLI_DEBUG("95% latency (us)=" + to_string(tbOoO.getLatencyPercentage(0.95)));
  INTELLI_DEBUG("Throughput (TPs/s)=" + to_string(tbOoO.getThroughput()));
  tbOoO.saveRTuplesToFile(outPrefix + "_tuples.csv", true);
  tbOoO.saveRTuplesToFile(outPrefix + "_arrived_tuples.csv", false);
  ConfigMapPtr resultBreakDown = tbOoO.getTimeBreakDown();
  if (resultBreakDown != nullptr) {
    resultBreakDown->toFile(outPrefix + "_breakdown.csv");
  }
  cfg->edit("watermarkPeriod", (uint64_t) (windowLenMs + maxArrivalSkewMs) * 1000);
  tb.setDataLoader(loaderTag,cfg);
  tb.setOperator(iawj, cfg);

  realRu = tb.inOrderTest(true);
  INTELLI_DEBUG("Expect " + to_string(realRu));
  double err = OoORu;
  err = (err - realRu) / realRu;
  generalStatistics.edit("Error", (double) err);
  INTELLI_DEBUG("OoO AQP joined " + to_string(tbOoO.AQPResult));
  err = tbOoO.AQPResult;
  err = (err - realRu) / realRu;
  generalStatistics.edit("AQPError", (double) err);
  INTELLI_DEBUG("Error = " + to_string(err));
  generalStatistics.toFile(outPrefix + "_general.csv");
  tb.loadDataFromCsv(outPrefix + "_arrived_tuples.csv");
  //windowLenMs= tryU64(cfg,"windowLenMs",1000);
}

int main(int argc, char **argv) {

  ThreadPerf pef(-1);

  string configName = "", outPrefix = "";
  if (argc >= 2) {
    configName += argv[1];
  } else {
    configName = "config.csv";
  }
  if (argc >= 3) {
    outPrefix += argv[2];
  } else {
    outPrefix = "default";
  }

  pef.setPerfList();
  pef.start();
  runTestBenchAdj(configName, outPrefix);
  pef.end();
  pef.resultToConfigMap()->toFile("perfRu.csv");

}

