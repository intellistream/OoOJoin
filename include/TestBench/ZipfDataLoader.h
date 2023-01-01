/*! \file ZipfDataLoader.h*/


#ifndef _INCLUDE_TESTBENCH_ZIPFDATALOADER_H_
#define _INCLUDE_TESTBENCH_ZIPFDATALOADER_H_
#include <Utils/ConfigMap.hpp>
#include <Common/Tuples.h>
#include <assert.h>
#include <Utils/IntelliLog.h>
#include <TestBench/AbstractDataLoader.h>
#include <vector>
#include <Utils/MicroDataSet.hpp>
using namespace INTELLI;
namespace OoOJoin {

/**
* @ingroup ADB_TESTBENCH The test bench to feed data into operators
* @{
 *
 */
/**
 * @defgroup ADB_TESTBENCH_DATALOADERS The classes of dataloader
 * @{
 */
/**
 * @class ZipfDataLoader TestBench/ZipfDataLoader.h
 * @brief The dataloader which allows zipf distribution of key, value, event time and arrival skewness
 * @ingroup ADB_TESTBENCH_DATALOADERS
 * @note:
 * - Must have a global config by @ref setConfig
 * - Can also have a modification config by
 *@note  Default behavior
* - create
* - setConfig and setModConfig (optional), generate R and S internally
* - call getTupleVectorS/R
 * @note require configs:
 * - zipfDataLoader_zipfKey U64, in key, 1 for zipf, 0 for random
 * - zipfDataLoader_zipfValue U64, in value, 1 for zipf, 0 for random
 * - zipfDataLoader_zipfEvent U64, in event time,  1 for zipf, 0 for random
 * - zipfDataLoader_zipfSkew U64, in arrival skewness,  1 for zipf, 0 for random
 *  - zipfDataLoader_zipfKeyFactor Double, in key,0~1
 * - zipfDataLoader_zipfValueFactor Double, in value, 0~1
 * - zipfDataLoader_zipfEventFactor Double, in event time, 0~1
 *  - zipfDataLoader_zipfSkewFactor Double, in event time, 0~1
 * - "windowLenMs" U64 The real world window length in ms
 * - "timeStepUs" U64 The simulation step in us
 *  - "maxArrivalSkewMs" U64 The maximum real-world arrival skewness in ms
 * - "eventRateKTps" U64 The real-world rate of spawn event, in KTuples/s
 * - "keyRange" U64 The range of Key
 * - "valueRange" U64 The range of value
 */
class ZipfDataLoader : public AbstractDataLoader {
 protected:
  tsType windowLenMs, timeStepUs, watermarkTimeMs, maxArrivalSkewMs, eventRateKTps;
  uint64_t zipfDataLoader_zipfKey, zipfDataLoader_zipfValue, zipfDataLoader_zipfEvent, zipfDataLoader_zipfSkew;
  vector<keyType> keyS, keyR;
  vector<valueType> valueS, valueR;
  vector<tsType> eventS, eventR;
  vector<tsType> arrivalS, arrivalR;
  size_t testSize, keyRange, valueRange;
  double zipfDataLoader_zipfKeyFactor, zipfDataLoader_zipfValueFactor, zipfDataLoader_zipfEventFactor,
      zipfDataLoader_zipfSkewFactor;
  MicroDataSet md;
  vector<tsType> genArrivalTime(vector<tsType> eventTime, vector<tsType> arrivalSkew) {
    vector<tsType> ru = vector<tsType>(eventTime.size());
    size_t len = (eventTime.size() > arrivalSkew.size()) ? arrivalSkew.size() : eventTime.size();
    for (size_t i = 0; i < len; i++) {
      ru[i] = eventTime[i] + arrivalSkew[i];
    }
    return ru;
  }
  vector<OoOJoin::TrackTuplePtr> genTuples(vector<keyType> keyS,
                                           vector<valueType> valueS,
                                           vector<tsType> eventS,
                                           vector<tsType> arrivalS) {
    size_t len = keyS.size();
    vector<OoOJoin::TrackTuplePtr> ru = vector<OoOJoin::TrackTuplePtr>(len);
    for (size_t i = 0; i < len; i++) {
      ru[i] = newTrackTuple(keyS[i], valueS[i], eventS[i], arrivalS[i]);
    }
    return ru;
  }
  /**
   * @brief generate the vector of key
   */
  void genKey();
  /**
   * @brief enerate the vector of key
   */
  void genValue();
  /**
   *
   *  @brief generate the vector of event
   */
  void genEvent();
  /**
   * @brief  generate the vector of arrival
   */
  void genArrival();
  /**
   * @brief generate the final result of s and r
   */
  void genFinal();
 public:
  ConfigMapPtr cfgGlobal;
  vector<TrackTuplePtr> sTuple, rTuple;
  ZipfDataLoader() {}
  ~ZipfDataLoader() {}
  /**
 * @brief Set the GLOBAL config map related to this loader
 * @param cfg The config map
  * @return bool whether the config is successfully set
 */
  virtual bool setConfig(ConfigMapPtr cfg);
  /**
* @brief Set the modification config map related to this loader
* @param cfg The config map
* @return bool whether the config is successfully set
*/
  virtual bool setModConfig(ConfigMapPtr cfg) {
    return AbstractDataLoader::setModConfig(cfg);
  }
  /**
   * @brief get the vector of s tuple
   * @return the vector
   */
  virtual vector<TrackTuplePtr> getTupleVectorS();
  /**
  * @brief get the vector of R tuple
  * @return the vector
  */
  virtual vector<TrackTuplePtr> getTupleVectorR();
};

/**
 * @ingroup ADB_TESTBENCH_DATALOADERS
 * @typedef ZipfDataLoaderPtr
 * @brief The class to describe a shared pointer to @ref ZipfDataLoader

 */
typedef std::shared_ptr<class ZipfDataLoader> ZipfDataLoaderPtr;
/**
 * @ingroup ADB_TESTBENCH_DATALOADERS
 * @def newZipfDataLoader
 * @brief (Macro) To creat a new @ref  ZipfDataLoader under shared pointer.
 */
#define newZipfDataLoader std::make_shared<OoOJoin::ZipfDataLoader>
/**
 * @}
 */
/**
 *
 */
}
#endif //INTELLISTREAM_INCLUDE_TESTBENCH_ZipfDataLoader_H_
