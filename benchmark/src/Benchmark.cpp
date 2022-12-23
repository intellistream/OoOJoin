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
//#include <torch/torch.h>
#include <chrono>
#include <iostream>
#include <source_location>
#include <ctime>
#include <torch/torch.h>
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
    // INTELLI_INFO(key + " = " + to_string(ru));
  } else {
    //  WM_WARNNING("Leaving " + key + " as blank, will use " + to_string(defaultValue) + " instead");
  }
  return ru;
}

/**
   * @brief Try to get an String from config map, if not exist, use default value instead
   * @param cfg The config map
   * @param key The key
   * @param defaultValue The default
   * @return The returned value
   */
string tryString(ConfigMapPtr config, string key, string defaultValue = "") {
  string ru = defaultValue;
  if (config->existString(key)) {
    ru = config->getString(key);
    //INTELLI_INFO(key + " = " + (ru));
  } else {
    // WM_WARNNING("Leaving " + key + " as blank, will use " + (defaultValue) + " instead");
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
  tsType windowLenMs, timeStepUs, watermarkPeriodMs, maxArrivalSkewMs, eventRateKTps;
  string operatorTag = "IAWJ";
  uint64_t keyRange;
  windowLenMs = tryU64(cfg, "windowLenMs", 10);
  timeStepUs = tryU64(cfg, "timeStepUs", 40);
  watermarkPeriodMs = tryU64(cfg, "watermarkPeriodMs", 10);
  maxArrivalSkewMs = tryU64(cfg, "maxArrivalSkewMs", 10 / 2);
  eventRateKTps = tryU64(cfg, "eventRateKTps", 10);
  keyRange = tryU64(cfg, "keyRange", 10);
  operatorTag = tryString(cfg, "operator", "IAWJ");
  testSize = windowLenMs * eventRateKTps;
  AbstractOperatorPtr iawj = opTable->findOperator(operatorTag);
  if (iawj == nullptr) {
    iawj = newIAWJOperator();
    // WM_WARNNING("No " + operatorTag + " operator, will use IAWJ instead");
  }
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
  INTELLI_INFO("/****run OoO test of " + to_string(testSize) + " tuples***/");
  tbOoO.setOperator(iawj, cfg);
  tbOoO.setDataSet(rTuple, sTuple);
  OoORu = tbOoO.OoOTest(true);
  INTELLI_DEBUG("OoO Confirmed joined " + to_string(OoORu));
  INTELLI_DEBUG("OoO AQP joined " +to_string(tbOoO.AQPResult));
  ConfigMap generalStatistics;
  generalStatistics.edit("AvgLatency", (double) tbOoO.getAvgLatency());
  generalStatistics.edit("95%Latency", (double) tbOoO.getLatencyPercentage(0.95));
  generalStatistics.edit("Throughput", (double) tbOoO.getThroughput());
  // tbOoO.logRTuples();
  // INTELLI_DEBUG("Average latency (us)=" << tbOoO.getAvgLatency());
  INTELLI_DEBUG("95% latency (us)=" +to_string(tbOoO.getLatencyPercentage(0.95)));
  INTELLI_DEBUG("Throughput (TPs/s)=" + to_string(tbOoO.getThroughput()));
  tbOoO.saveRTuplesToFile(outPrefix + "_tuples.csv", true);
  tbOoO.saveRTuplesToFile(outPrefix + "_arrived_tuples.csv", false);
  ConfigMapPtr resultBreakDown = tbOoO.getTimeBreakDown();
  if (resultBreakDown != nullptr) {
    resultBreakDown->toFile(outPrefix + "_breakdown.csv");
  }
  cfg->edit("watermarkPeriod", (uint64_t) (windowLenMs + maxArrivalSkewMs) * 1000);
  tb.setOperator(iawj, cfg);
  tb.setDataSet(rTuple, sTuple);
  realRu = tb.inOrderTest(true);
  INTELLI_DEBUG("Expect " + to_string(realRu));
  double err = OoORu;
  err = (err - realRu) / realRu;
  generalStatistics.edit("Error", (double) err);
  INTELLI_DEBUG("OoO AQP joined " +to_string(tbOoO.AQPResult));
  err = tbOoO.AQPResult;
  err = (err - realRu) / realRu;
  generalStatistics.edit("AQPError", (double) err);
  INTELLI_DEBUG("Error = " +to_string(err));
  generalStatistics.toFile(outPrefix + "_general.csv");
  //windowLenMs= tryU64(cfg,"windowLenMs",1000);
}
enum class log_level : char
{
  Info = 'I',
  Warning = 'W',
  Error = 'E'
};

int testPytorch() {
  torch::Tensor tensor = torch::rand({1,6});

  std::vector<float> v(tensor.data_ptr<float>(), tensor.data_ptr<float>() + tensor.numel());
  cout<<"raw tensor"<<endl;
  std::cout << tensor << std::endl;
  cout<<"vector"<<endl;
  for(int i=0;i<v.size();i++)
  {
    cout<<v[i]<<",";
  }
  auto opts = torch::TensorOptions().dtype(torch::kFloat32);
  auto tensor2 = torch::from_blob(v.data(), {int64_t(v.size())}, opts).clone();
  int tx=2,ty=3;
  tensor2=tensor2.reshape({tx,ty});
  //cout<<v.size()<<endl;
  cout<<"converted tensor"<<endl;
  std::cout << tensor2 << std::endl;
  return 0;
}
int main(int argc, char **argv) {

  ThreadPerf pef(-1);

  //Setup Logs.
  // setLogLevel(getStringAsDebugLevel("LOG_TRACE"));

  // setupLogging("benchmark.log", LOG_DEBUG);

 // INTELLI::IntelliLog::setupLoggingFile("benchmark.log");
  //Run the test here.
  //INTELLI_INFO("Nothing to run." << argc << argv);
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

  //tempTest();
  pef.setPerfList();
  pef.start();
  runTestBenchAdj(configName, outPrefix);
  pef.end();
  pef.resultToConfigMap()->toFile("perfRu.csv");
  testPytorch();
 // torch::Tensor tensor = torch::rand({2, 3});
  //std::cout << tensor << std::endl;
  //minist_test();
  // testConfig();
  //TuplePtrQueue tpq=
}

