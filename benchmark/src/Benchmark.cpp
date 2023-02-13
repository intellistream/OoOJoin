/*! \file benchmark.cpp*/
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
 * @defgroup OJ_BENCHMARK The benchmark program
 * @brief run the test bench and allow adjusting
 * @param configName
 * @param outPrefix
 * @note Require configs for this function:
 * - "windowLenMs" U64 The real world window length in ms
 * - "timeStepUs" U64 The simulation step in us
 * - "watermarkTimeMs" U64 The real world watermark generation period in ms
 * - "maxArrivalSkewMs" U64 The maximum real-world arrival skewness in ms
 * - "eventRateKTps" U64 The real-world rate of spawn event, in KTuples/s
 * - "keyRange" U64 The range of Key
 * - "operator" String The operator to be used
 */
void runTestBenchAdj(const string &configName = "config.csv", const string &outPrefix = "") {
    //IntelliLog::log("iNFO","Load global config from " + configName + ", output prefix = " + outPrefix + "\n");
    INTELLI_INFO("Load global config from" + configName + ", output prefix = " + outPrefix);

    OperatorTablePtr opTable = newOperatorTable();
    //IAWJOperatorPtr iawj = newIAWJOperator();
    //get config
    ConfigMapPtr cfg = newConfigMap();
    //read from csv file
    cfg->fromFile(configName);
    //size_t testSize = 0;
    size_t OoORu = 0, realRu = 0;
    //load global configs
    tsType windowLenMs, timeStepUs, maxArrivalSkewMs;
    string operatorTag = "IAWJ";
    string loaderTag = "random";

    //uint64_t keyRange;
    windowLenMs = cfg->tryU64("windowLenMs", 10, true);
    timeStepUs = cfg->tryU64("timeStepUs", 40, true);
    //watermarkTimeMs = cfg->tryU64("watermarkTimeMs", 10,true);
    maxArrivalSkewMs = cfg->tryU64("maxArrivalSkewMs", 10 / 2);

    INTELLI_INFO("window len= " + to_string(windowLenMs) + "ms ");
    // eventRateKTps = tryU64(cfg, "eventRateKTps", 10);
    //keyRange = tryU64(cfg, "keyRange", 10);
    operatorTag = cfg->tryString("operator", "IAWJ");
    loaderTag = cfg->tryString("dataLoader", "random");
    AbstractOperatorPtr iawj = opTable->findOperator(operatorTag);

    INTELLI_INFO("Try use " + operatorTag + " operator");

    if (iawj == nullptr) {
        iawj = newIAWJOperator();
        INTELLI_INFO("No " + operatorTag + " operator, will use IAWJ instead");
    }
    cfg->edit("windowLen", (uint64_t) windowLenMs * 1000);
    //cfg->edit("watermarkTime", (uint64_t) watermarkTimeMs * 1000);
    cfg->edit("timeStep", (uint64_t) timeStepUs);

    TestBench tb, tbOoO;
    //set data
    tbOoO.setDataLoader(loaderTag, cfg);

    cfg->edit("rLen", (uint64_t) tbOoO.sizeOfS());
    cfg->edit("sLen", (uint64_t) tbOoO.sizeOfR());
    //set operator as iawj
    tbOoO.setOperator(iawj, cfg);

    INTELLI_INFO("/****run OoO test of  tuples***/");

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
    tbOoO.saveRTuplesToFile(outPrefix + "_arrived_tuplesR.csv", false);
    tbOoO.saveSTuplesToFile(outPrefix + "_arrived_tuplesS.csv", false);

    ConfigMapPtr resultBreakDown = tbOoO.getTimeBreakDown();
    if (resultBreakDown != nullptr) {
        resultBreakDown->toFile(outPrefix + "_breakdown.csv");
    }
    /**
     * disable all possible OoO related settings, as we will test the expected in-order results
     */
    cfg->edit("watermarkTimeMs", (uint64_t) (windowLenMs + maxArrivalSkewMs));
    cfg->edit("latenessMs", (uint64_t) 0);
    cfg->edit("earlierEmitMs", (uint64_t) 0);
    tb.setDataLoader(loaderTag, cfg);
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

    //windowLenMs= tryU64(cfg,"windowLenMs",1000);
}


/**
 * @defgroup OJ_BENCHMARK The benchmark program
 * @brief run the test bench and allow adjusting
 * @param configName
 * @param outPrefix
 * @note Require configs for this function:
 * - "windowLenMs" U64 The real world window length in ms
 * - "timeStepUs" U64 The simulation step in us
 * - "watermarkTimeMs" U64 The real world watermark generation period in ms
 * - "maxArrivalSkewMs" U64 The maximum real-world arrival skewness in ms
 * - "eventRateKTps" U64 The real-world rate of spawn event, in KTuples/s
 * - "keyRange" U64 The range of Key
 * - "operator" String The operator to be used
 */
void runTestBenchOfMSMJ(const string &configName = "config.csv", const string &outPrefix = "") {
    //IntelliLog::log("iNFO","Load global config from " + configName + ", output prefix = " + outPrefix + "\n");
    INTELLI_INFO("Load global config from" + configName + ", output prefix = " + outPrefix);

    //get config
    ConfigMapPtr cfg = newConfigMap();
    //read from csv file
    cfg->fromFile(configName);
    //size_t testSize = 0;
    size_t OoORu = 0, realRu = 0;
    //load global configs
    tsType windowLenMs, timeStepUs, maxArrivalSkewMs;
    string operatorTag = "MSMJ";
    string loaderTag = "random";

    //uint64_t keyRange;
    windowLenMs = cfg->tryU64("windowLenMs", 10, true);
    timeStepUs = cfg->tryU64("timeStepUs", 40, true);
    //watermarkTimeMs = cfg->tryU64("watermarkTimeMs", 10,true);
    maxArrivalSkewMs = cfg->tryU64("maxArrivalSkewMs", 10 / 2);

    INTELLI_INFO("window len= " + to_string(windowLenMs) + "ms ");
    // eventRateKTps = tryU64(cfg, "eventRateKTps", 10);
    //keyRange = tryU64(cfg, "keyRange", 10);
//    operatorTag = cfg->tryString("operator", "MSMJ");
//    loaderTag = cfg->tryString("dataLoader", "random");


    INTELLI_INFO("Try use " + operatorTag + " operator");


    cfg->edit("operator", "MSMJ");
    cfg->edit("dataLoader", "random");
    cfg->edit("windowLen", (uint64_t) windowLenMs * 1000);
    //cfg->edit("watermarkTime", (uint64_t) watermarkTimeMs * 1000);
    cfg->edit("timeStep", (uint64_t) timeStepUs);

    TestBench tb, tbOoO;
    //set data
    tbOoO.setDataLoader(loaderTag, cfg);

    cfg->edit("rLen", (uint64_t) tbOoO.sizeOfS());
    cfg->edit("sLen", (uint64_t) tbOoO.sizeOfR());

    cfg->edit("g", (uint64_t) 1);
    cfg->edit("L", (uint64_t) 3);
    cfg->edit("userRecall", 0.4);
    cfg->edit("b", (uint64_t) 1);
    cfg->edit("confidenceValue", 0.5);
    cfg->edit("P", (uint64_t) 4);
    cfg->edit("maxDelay", (uint64_t) 100);
    cfg->edit("StreamCount", (uint64_t) 2);

    OperatorTablePtr opTable = newOperatorTable();

    AbstractOperatorPtr abstractOperator = opTable->findOperator(operatorTag);

    MSMJOperatorPtr msmj = shared_ptr<MSMJOperator>(reinterpret_cast<MSMJOperator *>(abstractOperator.get()));

    msmj->init(cfg);

    if (msmj == nullptr) {
        msmj = newMSMJOperator();
        INTELLI_INFO("No " + operatorTag + " operator, will use MSMJ instead");
    }

    //set operator as msmj
    tbOoO.setOperator(msmj, cfg);

    INTELLI_INFO("/****run OoO test of  tuples***/");

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
    tbOoO.saveRTuplesToFile(outPrefix + "_arrived_tuplesR.csv", false);
    tbOoO.saveSTuplesToFile(outPrefix + "_arrived_tuplesS.csv", false);

    ConfigMapPtr resultBreakDown = tbOoO.getTimeBreakDown();
    if (resultBreakDown != nullptr) {
        resultBreakDown->toFile(outPrefix + "_breakdown.csv");
    }
    /**
     * disable all possible OoO related settings, as we will test the expected in-order results
     */
    cfg->edit("watermarkTimeMs", (uint64_t) (windowLenMs + maxArrivalSkewMs));
    cfg->edit("latenessMs", (uint64_t) 0);
    cfg->edit("earlierEmitMs", (uint64_t) 0);
    tb.setDataLoader(loaderTag, cfg);
    tb.setOperator(msmj, cfg);

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

    //windowLenMs= tryU64(cfg,"windowLenMs",1000);
}


int main(int argc, char **argv) {

    ThreadPerf pef(-1);

    string configName, outPrefix = "";
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
//    runTestBenchAdj(configName, outPrefix);
    runTestBenchOfMSMJ(configName, outPrefix);
    pef.end();
    pef.resultToConfigMap()->toFile("perfRu.csv");

}

