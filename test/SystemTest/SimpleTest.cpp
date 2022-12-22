// Copyright (C) 2021 by the INTELLI team (https://github.com/intellistream)

#include <filesystem>
#include <gtest/gtest.h>
//#include <Utils/Logger.hpp>
//#include <Utils/Logger.hpp>
#include <vector>
#include <OoOJoin.h>
using namespace std;
using namespace OoOJoin;

vector<tsType> genArrivalTime(vector<tsType> eventTime, vector<tsType> arrivalSkew) {
  vector<tsType> ru = vector<tsType>(eventTime.size());
  size_t len = (eventTime.size() > arrivalSkew.size()) ? arrivalSkew.size() : eventTime.size();
  for (size_t i = 0; i < len; i++) {
    ru[i] = eventTime[i] + arrivalSkew[i];
  }
  return ru;
}

void bubble_sort(vector<OoOJoin::TrackTuplePtr> &arr) {
  size_t i, j;
  TrackTuplePtr temp;
  size_t len = arr.size();
  for (i = 0; i < len - 1; i++)
    for (j = 0; j < len - 1 - i; j++)
      if (arr[j]->arrivalTime > arr[j + 1]->arrivalTime) {
        temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
}
vector<OoOJoin::TrackTuplePtr> genTuples(vector<keyType> keyS, vector<tsType> eventS, vector<tsType> arrivalS) {
  size_t len = keyS.size();
  vector<OoOJoin::TrackTuplePtr> ru = vector<OoOJoin::TrackTuplePtr>(len);
  for (size_t i = 0; i < len; i++) {
    ru[i] = newTrackTuple(keyS[i], 0, eventS[i], arrivalS[i]);
  }
  bubble_sort(ru);
  return ru;
}
vector<TrackTuplePtr> genTuplesSmooth(size_t testSize,
                                      uint64_t keyRange,
                                      uint64_t rateKtps,
                                      uint64_t groupUnit,
                                      uint64_t maxSkewUs,
                                      uint64_t seed = 999) {
  MicroDataSet ms(seed);
  uint64_t tsGrow = 1000 * groupUnit / rateKtps;
  vector<keyType> keyS = ms.genRandInt<keyType>(testSize, keyRange, 1);
  vector<tsType> eventS = ms.genSmoothTimeStamp<tsType>(testSize, groupUnit, tsGrow);
  vector<tsType> arrivalSkew = ms.genRandInt<tsType>(testSize, maxSkewUs, 1);
  vector<tsType> arrivalS = genArrivalTime(eventS, arrivalSkew);
  vector<TrackTuplePtr> sTuple = genTuples(keyS, eventS, arrivalS);
  return sTuple;
}
/**
   * @brief Try to get an U64 from config map, if not exist, use default value instead
   * @param cfg The config map
   * @param key The key
   * @param defaultValue The default
   * @return The returned value
   */
uint64_t tryU64(ConfigMapPtr config, string key, uint64_t defaultValue = 0) {
  uint64_t ru = defaultValue;
  if (config->existU64(key)) {
    ru = config->getU64(key);
    // INTELLI_INFO(key+" = " + to_string(ru));
  } else {
    // WM_WARNNING("Leaving " +key+" as blank, will use "+ to_string(defaultValue)+" instead");
  }
  return ru;
}
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
 */
void runTestBenchAdj(string configName = "config.csv", string outPrefix = "") {
  //INTELLI_INFO("Load global config from" + configName + ", output prefix = " + outPrefix + "\n");
  IAWJOperatorPtr iawj = newIAWJOperator();
  //get config
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  size_t testSize = 0;
  size_t OoORu = 0, realRu = 0;
  //load global configs
  tsType windowLenMs, timeStepUs, watermarkPeriodMs, maxArrivalSkewMs, eventRateKTps;
  uint64_t keyRange;
  windowLenMs = tryU64(cfg, "windowLenMs", 10);
  timeStepUs = tryU64(cfg, "timeStepUs", 40);
  watermarkPeriodMs = tryU64(cfg, "watermarkPeriodMs", 10);
  maxArrivalSkewMs = tryU64(cfg, "maxArrivalSkewMs", 10 / 2);
  eventRateKTps = tryU64(cfg, "eventRateKTps", 10);
  keyRange = tryU64(cfg, "keyRange", 10);
  testSize = windowLenMs * eventRateKTps;
  // generate dataset
  vector<TrackTuplePtr>
      sTuple = genTuplesSmooth(testSize, keyRange, eventRateKTps, timeStepUs, maxArrivalSkewMs * 1000, 7758258);
  vector<TrackTuplePtr>
      rTuple = genTuplesSmooth(testSize, keyRange, eventRateKTps, timeStepUs, maxArrivalSkewMs * 1000, 114514);
  cfg->edit("rLen", (uint64_t) testSize);
  cfg->edit("sLen", (uint64_t) testSize);
  cfg->edit("windowLen", (uint64_t) windowLenMs * 1000);
  cfg->edit("watermarkPeriod", (uint64_t) watermarkPeriodMs * 1000);
  cfg->edit("timeStep", (uint64_t) timeStepUs);
  TestBench tb, tbOoO;
  //cfg->edit("windowLen", (uint64_t) 100);
  // cfg->edit("watermarkPeriod", (uint64_t) 100);
  //INTELLI_INFO("/****run OoO test of "+to_string(testSize)+" tuples***/");
  tbOoO.setOperator(iawj, cfg);
  tbOoO.setDataSet(rTuple, sTuple);
  OoORu = tbOoO.OoOTest(true);
  //INTELLI_DEBUG("OoO joined " << OoORu);
  ConfigMap generalStatistics;
  generalStatistics.edit("AvgLatency", (double) tbOoO.getAvgLatency());
  generalStatistics.edit("95%Latency", (double) tbOoO.getLatencyPercentage(0.95));
  generalStatistics.edit("Throughput", (double) tbOoO.getThroughput());
  // tbOoO.logRTuples();
  //INTELLI_DEBUG("Average latency (us)=" << tbOoO.getAvgLatency());
  //INTELLI_DEBUG("95% latency (us)=" << tbOoO.getLatencyPercentage(0.95));
  //INTELLI_DEBUG("Throughput (TPs/s)=" << tbOoO.getThroughput());
  tbOoO.saveRTuplesToFile(outPrefix + "_tuples.csv", true);

  cfg->edit("watermarkPeriod", (uint64_t) (windowLenMs + maxArrivalSkewMs) * 1000);
  tb.setOperator(iawj, cfg);
  tb.setDataSet(rTuple, sTuple);
  realRu = tb.inOrderTest(true);
  //INTELLI_DEBUG("Expect " << realRu);
  double err = OoORu;
  err = (err - realRu) / realRu;
  generalStatistics.edit("Error", (double) err);
  // INTELLI_DEBUG("Error = " << err);
  generalStatistics.toFile(outPrefix + "_general.csv");
  //windowLenMs= tryU64(cfg,"windowLenMs",1000);
}
TEST(SystemTest, SimpleTest
) {
//Setup Logs.
// setLogLevel(getStringAsDebugLevel("LOG_DEBUG"));

//setupLogging("benchmark.log", LOG_DEBUG);

//Run the test here.
// INTELLI_INFO("Nothing to test.");
string configName = "config.csv", outPrefix = "";

//tempTest();
runTestBenchAdj(configName, outPrefix
);
}