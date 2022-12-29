/*! \file RandomDataLoader.h*/
//
// Created by tony on 29/12/22.
//

#ifndef _INCLUDE_TESTBENCH_RandomDataLoader_H_
#define _INCLUDE_TESTBENCH_RandomDataLoader_H_
#include <TestBench/AbstractDataLoader.h>
#include <Utils/MicroDataSet.hpp>
using namespace INTELLI;
namespace OoOJoin {

/**
* @ingroup ADB_TESTBENCH The test bench to feed data into operators
* @{
 *
 */
/**
 * @class RandomDataLoader TestBench/RandomDataLoader.h
 * @brief The dataloader which produces random key, random value and random skewness
 * @ingroup ADB_TESTBENCH_DATALOADERS
 * @note:
 * Default behavior
 * - create
 * - setConfig and setModConfig (optional), generate R and S internally
 * - call getTupleVectorS/R and getSizeS/R;
 */
class RandomDataLoader : public  AbstractDataLoader{
 protected:
  tsType windowLenMs, timeStepUs, watermarkPeriodMs, maxArrivalSkewMs, eventRateKTps;
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
    vector<TrackTuplePtr> genTuple = genTuples(keyS, eventS, arrivalS);
    return genTuple;
  }
 public:
  ConfigMapPtr cfgGlobal;
  vector<TrackTuplePtr> sTuple,rTuple;
  RandomDataLoader(){}
  ~RandomDataLoader(){}
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
  virtual bool setModConfig(ConfigMapPtr cfg)
  {
    assert(cfg);
    return true;
  }

  /**
   * @brief get the vector of s tuple
   * @return the vector
   */
  virtual vector<TrackTuplePtr>getTupleVectorS();

  /**
  * @brief get the vector of R tuple
  * @return the vector
  */
  virtual vector<TrackTuplePtr>getTupleVectorR();

};

/**
 * @ingroup ADB_TESTBENCH_DATALOADERS
 * @typedef RandomDataLoaderPtr
 * @brief The class to describe a shared pointer to @ref RandomDataLoader

 */
typedef std::shared_ptr<class RandomDataLoader> RandomDataLoaderPtr;
/**
 * @ingroup ADB_TESTBENCH_DATALOADERS
 * @def newRandomDataLoader
 * @brief (Macro) To creat a new @ref RandomDataLoader under shared pointer.
 */
#define newRandomDataLoader std::make_shared<OoOJoin::RandomDataLoader>

/**
 *
 */
}
#endif //INTELLISTREAM_INCLUDE_TESTBENCH_RandomDataLoader_H_
