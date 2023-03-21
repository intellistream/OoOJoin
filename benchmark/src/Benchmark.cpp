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

constexpr static int SECONDS = 1000000;
constexpr static int MICRO_SECONDS = 1;
constexpr static int MILLION_SECONDS = 1000;


MSWJOperatorPtr mswjConfiguration(ConfigMapPtr cfg) {
    MSWJOperatorPtr mswj;
    cfg->edit("g", (uint64_t) 10 * MILLION_SECONDS);
    cfg->edit("L", (uint64_t) 50 * MILLION_SECONDS);
    cfg->edit("userRecall", 0.4);
    cfg->edit("b", (uint64_t) 10 * MILLION_SECONDS);
    cfg->edit("confidenceValue", 0.5);
    cfg->edit("P", (uint64_t) 10 * SECONDS);
    cfg->edit("maxDelay", (uint64_t) INT16_MAX);
    cfg->edit("StreamCount", (uint64_t) 2);
    cfg->edit("Stream_1", (uint64_t) 0);
    cfg->edit("Stream_2", (uint64_t) 0);

    auto tupleProductivityProfiler = std::make_shared<MSWJ::TupleProductivityProfiler>(cfg);
    auto statisticsManager = std::make_shared<MSWJ::StatisticsManager>(tupleProductivityProfiler.get(), cfg);
    auto bufferSizeManager = std::make_shared<MSWJ::BufferSizeManager>(statisticsManager.get(),
                                                                       tupleProductivityProfiler.get());
    auto streamOperator = std::make_shared<MSWJ::StreamOperator>(tupleProductivityProfiler.get(), cfg);
    auto synchronizer = std::make_shared<MSWJ::Synchronizer>(2, streamOperator.get(), cfg);

    bufferSizeManager->setConfig(cfg);

    auto kSlackS = std::make_shared<MSWJ::KSlack>(1, bufferSizeManager.get(), statisticsManager.get(),
                                                  synchronizer.get());
    auto kSlackR = std::make_shared<MSWJ::KSlack>(2, bufferSizeManager.get(), statisticsManager.get(),
                                                  synchronizer.get());


    mswj = std::make_shared<MSWJOperator>(bufferSizeManager, tupleProductivityProfiler,
                                          synchronizer,
                                          streamOperator, statisticsManager, kSlackR, kSlackS);
    mswj->setConfig(cfg);

    return mswj;
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
void runTestBenchAdj(const string &configName = "config.csv", const string &outPrefix = "") {
    INTELLI_INFO("Load global config from" + configName + ", output prefix = " + outPrefix);

    OperatorTablePtr opTable = newOperatorTable();
    ConfigMapPtr cfg = newConfigMap();
    cfg->fromFile(configName);

    size_t OoORu = 0, realRu = 0;
    tsType windowLenMs, timeStepUs, maxArrivalSkewMs;
    string operatorTag = "IMA";
    string loaderTag = "file";

    cfg->edit("operator", operatorTag);
    cfg->edit("dataLoader", loaderTag);

    windowLenMs = cfg->tryU64("windowLenMs", 10, true);
    timeStepUs = cfg->tryU64("timeStepUs", 40, true);
    maxArrivalSkewMs = cfg->tryU64("maxArrivalSkewMs", 10 / 2);

    windowLenMs = 2000;
    maxArrivalSkewMs = 50000;
    INTELLI_INFO("window len= " + to_string(windowLenMs) + "ms ");
    INTELLI_INFO("Try use " + operatorTag + " operator");

    AbstractOperatorPtr iawj;
    MSWJOperatorPtr mswj;

    if (operatorTag == "IAWJ") {
        iawj = newIAWJOperator();
    } else if (operatorTag == "MSWJ") {
        mswj = mswjConfiguration(cfg);
    } else {
        iawj = opTable->findOperator(operatorTag);
    }

    if (operatorTag != "MSWJ" && iawj == nullptr) {
        iawj = newIAWJOperator();
        INTELLI_INFO("No " + operatorTag + " operator, will use IAWJ instead");
    }

    //Global configs
    cfg->edit("windowLen", (uint64_t) windowLenMs * 1000);
    cfg->edit("timeStep", (uint64_t) timeStepUs);

    //Dataset files
    cfg->edit("fileDataLoader_rFile", "../../benchmark/datasets/cj_1000ms_1tHighDelayData.csv");
    cfg->edit("fileDataLoader_sFile", "../../benchmark/datasets/sb_1000ms_1tHighDelayData.csv");

    TestBench tb, tbOoO;
    tbOoO.setDataLoader(loaderTag, cfg);

    cfg->edit("rLen", (uint64_t) tbOoO.sizeOfS());
    cfg->edit("sLen", (uint64_t) tbOoO.sizeOfR());
    cfg->edit("watermarkTimeMs", (uint64_t) (windowLenMs + maxArrivalSkewMs));
    cfg->edit("latenessMs", (uint64_t) 0);
    cfg->edit("earlierEmitMs", (uint64_t) 0);

    if (operatorTag == "MSWJ") {
        tbOoO.setOperator(mswj, cfg);
    } else {
        tbOoO.setOperator(iawj, cfg);
    }

    INTELLI_INFO("/****run OoO test of  tuples***/");
    OoORu = tbOoO.OoOTest(true);

    INTELLI_DEBUG("OoO Confirmed joined " + to_string(OoORu));
    INTELLI_DEBUG("OoO AQP joined " + to_string(tbOoO.AQPResult));

    ConfigMap generalStatistics;
    generalStatistics.edit("AvgLatency", (double) tbOoO.getAvgLatency());
    generalStatistics.edit("95%Latency", (double) tbOoO.getLatencyPercentage(0.95));
    generalStatistics.edit("Throughput", (double) tbOoO.getThroughput());

    INTELLI_DEBUG("95% latency (us)=" + to_string(tbOoO.getLatencyPercentage(0.95)));
    INTELLI_DEBUG("Throughput (TPs/s)=" + to_string(tbOoO.getThroughput()));

    tbOoO.saveRTuplesToFile(outPrefix + "_tuples.csv", true);
    tbOoO.saveRTuplesToFile(outPrefix + "_arrived_tuplesR.csv", false);
    tbOoO.saveSTuplesToFile(outPrefix + "_arrived_tuplesS.csv", false);

    ConfigMapPtr resultBreakDown = tbOoO.getTimeBreakDown();
    if (resultBreakDown != nullptr) {
        resultBreakDown->toFile(outPrefix + "_breakdown.csv");
    }

    tb.setDataLoader(loaderTag, cfg);

    if (operatorTag == "MSWJ") {
        tb.setOperator(mswj, cfg);
    } else {
        tb.setOperator(iawj, cfg);
    }

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
    runTestBenchAdj(configName, outPrefix);
    pef.end();
    pef.resultToConfigMap()->toFile("perfRu.csv");
}

