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

//using namespace MSMJ;
//
//std::list<Stream *> generate_stream() {
//    std::queue<Tuple> stream_1_list;
//
//    Tuple e1_1(1, 1, 1);
//    Tuple e1_2(1, 2, 4);
//    Tuple e1_3(1, 3, 3);
//    Tuple e1_4(1, 4, 5);
//    Tuple e1_5(1, 5, 7);
//    Tuple e1_6(1, 6, 8);
//    Tuple e1_7(1, 7, 6);
//    Tuple e1_8(1, 8, 9);
//
//    stream_1_list.push(e1_1);
//    stream_1_list.push(e1_2);
//    stream_1_list.push(e1_3);
//    stream_1_list.push(e1_4);
//    stream_1_list.push(e1_5);
//    stream_1_list.push(e1_6);
//    stream_1_list.push(e1_7);
//    stream_1_list.push(e1_8);
//
//    auto *stream_1 = new Stream(1, 2, stream_1_list);
//
//    std::queue<Tuple> stream_2_list;
//
//    Tuple e2_1(2, 1, 2);
//    Tuple e2_2(2, 2, 5);
//    Tuple e2_3(2, 3, 4);
//    Tuple e2_4(2, 4, 5);
//    Tuple e2_5(2, 5, 8);
//    Tuple e2_6(2, 6, 9);
//    Tuple e2_7(2, 7, 7);
//    Tuple e2_8(2, 8, 9);
//
//    stream_2_list.push(e2_1);
//    stream_2_list.push(e2_2);
//    stream_2_list.push(e2_3);
//    stream_2_list.push(e2_4);
//    stream_2_list.push(e2_5);
//    stream_2_list.push(e2_6);
//    stream_2_list.push(e2_7);
//    stream_2_list.push(e2_8);
//
//    auto *stream_2 = new Stream(2, 2, stream_2_list);
//
//    std::list<Stream *> list;
//    list.push_back(stream_1);
//    list.push_back(stream_2);
//
//    stream_map[1] = stream_1;
//    stream_map[2] = stream_2;
//
//    return list;
//}

//static void *task(void *p) {
//    reinterpret_cast<KSlack *>(p)->disorder_handling();
//    return nullptr;
//}
//
//
//void test_2_kslack() {
//    //初始化
//    auto *productivity_profiler = new TupleProductivityProfiler();
//    auto *stream_operator = new StreamOperator(productivity_profiler);
//    auto *statistics_manager = new StatisticsManager(productivity_profiler);
//    auto *buffer_size_manager = new BufferSizeManager(statistics_manager, productivity_profiler);
//    auto *synchronizer = new Synchronizer(2, stream_operator);
//
//    std::list<Stream *> stream_list = generate_stream();
//    std::list<KSlack *> kslack_list;
//
//    for (auto it: stream_list) {
//        auto *kslack = new KSlack(it, buffer_size_manager, statistics_manager, synchronizer);
//        kslack_list.push_back(kslack);
//    }
//
//    //生成线程
//    pthread_t t1 = 1;
//    pthread_t t2 = 2;
//    pthread_create(&t1, NULL, &task, kslack_list.front());
//    pthread_create(&t2, NULL, &task, kslack_list.back());
//    //执行线程：
//    pthread_join(t1, NULL);
//    pthread_join(t2, NULL);
//
//
//    //输出kslack后的结果
//    for (auto it: kslack_list) {
//        std::cout << "kslack作用后:" << std::endl;
//        print(it->get_output());
//    }
//
//    //同步后的结果：
//    std::cout << "同步后:" << std::endl;
//    print(synchronizer->get_output());
//
//    //连接后的结果：
//    std::cout << "连接后:" << std::endl;
//    auto res = stream_operator->get_result();
//    while (!res.empty()) {
//        for (auto it: res.front()) {
//            std::cout << "e" << it.streamId << "," << it.id << "时间戳:" << it.ts << ",";
//        }
//        res.pop();
//        std::cout << std::endl;
//    }
//
//    delete productivity_profiler;
//    delete stream_operator;
//    delete statistics_manager;
//    delete buffer_size_manager;
//}


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

