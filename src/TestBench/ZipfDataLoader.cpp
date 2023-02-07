//
// Created by tony on 29/12/22.
//

#include <TestBench/ZipfDataLoader.h>

using namespace INTELLI;
using namespace OoOJoin;

void ZipfDataLoader::genKey() {
    zipfDataLoader_zipfKey = cfgGlobal->tryU64("zipfDataLoader_zipfKey", 1, true);

    if (zipfDataLoader_zipfKey) {
        zipfDataLoader_zipfKeyFactor = cfgGlobal->tryDouble("zipfDataLoader_zipfKeyFactor", 0.5, true);
        /**
         * @brief use zipf for key
         */
        INTELLI_INFO("Use zipf for key, factor=" + to_string(zipfDataLoader_zipfKeyFactor));
        md.setSeed(1);
        keyS = md.genZipfInt<keyType>(testSize, keyRange, zipfDataLoader_zipfKeyFactor);
        md.setSeed(100);
        keyR = md.genZipfInt<keyType>(testSize, keyRange, zipfDataLoader_zipfKeyFactor);
    } else {
        md.setSeed(1);
        keyS = md.genRandInt<keyType>(testSize, keyRange, 1);
        md.setSeed(100);
        keyR = md.genRandInt<keyType>(testSize, keyRange, 1);
    }
    INTELLI_INFO("Finish the generation of keys");
}

void ZipfDataLoader::genValue() {
    zipfDataLoader_zipfValue = cfgGlobal->tryU64("zipfDataLoader_zipfValue", 1, true);

    if (zipfDataLoader_zipfValue) {
        zipfDataLoader_zipfValueFactor = cfgGlobal->tryDouble("zipfDataLoader_zipfValueFactor", 0.5, true);
        /**
         * @brief use zipf for value
         */
        INTELLI_INFO("Use zipf for value, factor=" + to_string(zipfDataLoader_zipfValueFactor));
        md.setSeed(999);
        valueS = md.genZipfInt<valueType>(testSize, valueRange, zipfDataLoader_zipfValueFactor);
        md.setSeed(114514);
        valueR = md.genZipfInt<valueType>(testSize, valueRange, zipfDataLoader_zipfValueFactor);
    } else {
        md.setSeed(999);
        valueS = md.genRandInt<valueType>(testSize, valueRange, 1);
        md.setSeed(114514);
        valueR = md.genRandInt<valueType>(testSize, valueRange, 1);
    }
    INTELLI_INFO("Finish the generation of value");

}

void ZipfDataLoader::genEvent() {
    zipfDataLoader_zipfEvent = cfgGlobal->tryU64("zipfDataLoader_zipfEvent", 1, true);

    if (zipfDataLoader_zipfEvent) {
        zipfDataLoader_zipfEventFactor = cfgGlobal->tryDouble("zipfDataLoader_zipfEventFactor", 0.5, true);
        /**
         * @brief use zipf for event
         */
        INTELLI_INFO("Use zipf for event time, factor=" + to_string(zipfDataLoader_zipfEventFactor));
        md.setSeed(7758);
        eventS =
                md.genZipfTimeStamp<tsType>(testSize, (windowLenMs + maxArrivalSkewMs) * 1000,
                                            zipfDataLoader_zipfEventFactor);
        md.setSeed(258);
        eventR =
                md.genZipfTimeStamp<tsType>(testSize, (windowLenMs + maxArrivalSkewMs) * 1000,
                                            zipfDataLoader_zipfEventFactor);
    } else {
        uint64_t tsGrow = 1000 * timeStepUs / eventRateKTps;
        md.setSeed(7758);
        eventS = md.genSmoothTimeStamp<tsType>(testSize, timeStepUs, tsGrow);
        md.setSeed(258);
        eventR = md.genSmoothTimeStamp<tsType>(testSize, timeStepUs, tsGrow);
    }
    INTELLI_INFO("Finish the generation of event time");
}

void ZipfDataLoader::genArrival() {
    vector<tsType> skewS, skewR;
    zipfDataLoader_zipfSkew = cfgGlobal->tryU64("zipfDataLoader_zipfSkew", 1, true);

    if (zipfDataLoader_zipfSkew) {
        zipfDataLoader_zipfSkewFactor = cfgGlobal->tryDouble("zipfDataLoader_zipfSkewFactor", 0.5, true);
        /**
         * @brief use zipf for event
         */
        INTELLI_INFO("Use zipf for arrival skew, factor=" + to_string(zipfDataLoader_zipfSkewFactor));
        md.setSeed(1024);
        skewS = md.genZipfInt<tsType>(testSize, maxArrivalSkewMs * 1000, zipfDataLoader_zipfSkewFactor);
        md.setSeed(4096);
        skewR = md.genZipfInt<tsType>(testSize, maxArrivalSkewMs * 1000, zipfDataLoader_zipfSkewFactor);

    } else {
        md.setSeed(1024);
        skewS = md.genRandInt<valueType>(testSize, maxArrivalSkewMs * 1000, 1);
        md.setSeed(4096);
        skewR = md.genRandInt<valueType>(testSize, maxArrivalSkewMs * 1000, 1);
    }
    arrivalS = genArrivalTime(eventS, skewS);
    arrivalR = genArrivalTime(eventR, skewR);
    INTELLI_INFO("Finish the generation of arrival time");
}

void ZipfDataLoader::genFinal() {
    sTuple = genTuples(keyS, valueS, eventS, arrivalS);
    rTuple = genTuples(keyR, valueR, eventR, arrivalR);
}

bool ZipfDataLoader::setConfig(ConfigMapPtr cfg) {

    cfgGlobal = cfg;
    /**
     * @brief load some common settings
     */
    md.setSeed(999);
    windowLenMs = cfg->tryU64("windowLenMs", 10);
    timeStepUs = cfg->tryU64("timeStepUs", 40);
    watermarkTimeMs = cfg->tryU64("watermarkTimeMs", 10);
    maxArrivalSkewMs = cfg->tryU64("maxArrivalSkewMs", 10 / 2);
    eventRateKTps = cfg->tryU64("eventRateKTps", 10);
    keyRange = cfg->tryU64("keyRange", 10, true);
    valueRange = cfg->tryU64("valueRange", 1000, true);
    testSize = (windowLenMs + maxArrivalSkewMs) * eventRateKTps;
    genKey();
    genValue();
    genEvent();
    genArrival();
    genFinal();
    return true;
}

vector<TrackTuplePtr> ZipfDataLoader::getTupleVectorS() {
    return sTuple;
}

vector<TrackTuplePtr> ZipfDataLoader::getTupleVectorR() {
    return rTuple;
}