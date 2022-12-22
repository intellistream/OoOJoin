/*! \file   MeanAQPIAWJOperator.h*/
//
// Created by tony on 03/12/22.
//

#ifndef INTELLISTREAM_INCLUDE_OPERATOR_MEANAQPMeanAQPIAWJOPERATOR_H_
#define INTELLISTREAM_INCLUDE_OPERATOR_MEANAQPMeanAQPIAWJOPERATOR_H_
#include <Operator/AbstractOperator.h>
#include <Common/Window.h>
#include <atomic>
#include <WaterMarker/PeriodicalWM.h>
#include <Common/StateOfKey.h>
namespace OoOJoin {

/**
 * @ingroup ADB_OPERATORS
 * @class MeanAQPIAWJOperator Operator/MeanAQPIAWJOperator.h
 * \image html MeanAQP.png
 * @brief The intra window join (MeanAQPIAWJ) operator under the simplest AQP strategy, only considers a single window and uses
 * exponential weighted moving average for prediction
 * @note require configurations:
 * - "windowLen" U64: The length of window
 * - "slideLen" U64: The length of slide
 * - "sLen" U64: The length of S buffer
 * - "rLen" U64: The length of R buffer
 * @warning This implementation is putting rotten, just to explore a basic idea of AQP by using historical mean to predict future
 * @warning The predictor and watermarker are currently NOT seperated in this operator, split them in the future!
 * @note In current version, the computation will block feeding
 * @note follows the assumption of linear independent arrival and skewness
 * @note operator tag = "MeanAQP"
 */
class MeanAQPIAWJOperator : public AbstractOperator {
 protected:
  Window myWindow;
  size_t intermediateResult = 0;
  size_t confirmedResult = 0;
  uint64_t windowBound = 0;
  // double alphaArrivalRate=0.125;
  double alphaArrivalSkew = 0.125;
  double betaArrivalSkew = 0.25;
//  tsType lastTimeS = 0, lastTimeR = 0;
  double aqpScale = 0.1;
  void conductComputation();
  atomic_bool lockedByWaterMark = false;
  AbstractWaterMarkerPtr wmGen = nullptr;
  StateOfKeyHashTablePtr stateOfKeyTableR, stateOfKeyTableS;
  tsType lastTimeOfR = 0;
  /**
   * @brief for time breakdown of searching index
   */
  tsType timeBreakDown_index;
  /**
   * @brief for time breakdown of prediction
   */
  tsType timeBreakDown_prediction;
  /**
   * @brief for time breakdown of join
   */
  tsType timeBreakDown_join;
  class MeanStateOfKey : public AbstractStateOfKey {
   public:
    size_t arrivedTupleCnt = 0;
    double arrivalSkew = 0, sigmaArrivalSkew = 0;
    TrackTuplePtr lastEventTuple = nullptr, lastArrivalTuple = nullptr;
    // tsType  lastSeenTime=0;
    MeanStateOfKey() {}
    ~MeanStateOfKey() {}
  };
  typedef std::shared_ptr<MeanStateOfKey> MeanStateOfKeyPtr;
#define newMeanStateOfKey std::make_shared<MeanStateOfKey>
  void updateStateOfKey(MeanStateOfKeyPtr sk, TrackTuplePtr tp);
  // void updateStateOfKeyR(MeanStateOfKeyPtr sk,TrackTuplePtr tp);
  void lazyComputeOfAQP();
  double predictUnarrivedTuples(MeanStateOfKeyPtr px);
 public:
  MeanAQPIAWJOperator() {}
  ~MeanAQPIAWJOperator() {}
  /**
   * @todo Where this operator is conducting join is still putting rotten, try to place it at feedTupleS/R
  * @brief Set the config map related to this operator
  * @param cfg The config map
   * @return bool whether the config is successfully set
  */
  virtual bool setConfig(ConfigMapPtr cfg);
  /**
 * @brief feed a tuple s into the Operator
 * @param ts The tuple
  * @warning The current version is simplified and assuming only used in SINGLE THREAD!
  * @return bool, whether tuple is fed.
 */
  virtual bool feedTupleS(TrackTuplePtr ts);

  /**
    * @brief feed a tuple R into the Operator
    * @param tr The tuple
    * @warning The current version is simplified and assuming only used in SINGLE THREAD!
    *  @return bool, whether tuple is fed.
    */
  virtual bool feedTupleR(TrackTuplePtr tr);

  /**
   * @brief start this operator
   * @return bool, whether start successfully
   */
  virtual bool start();

  /**
   * @brief stop this operator
   * @return bool, whether start successfully
   */
  virtual bool stop();

  /**
   * @brief get the joined sum result
   * @return The result
   */
  virtual size_t getResult();

  /**
   * @brief get the joined sum result under AQP
   * @return The result
   */
  virtual size_t getAQPResult();
  /**
 * @brief get the break down information of processing time
 * @warning should check the nullptr of output
 * @return The ConfigMapPtr which contains breakdown information, null if no breakdown supported
  * @note provided breakdown:
  * - "index" U64, processing time on prediction for indexing the tuple state in hhash table
  * - "prediction" U64, processing time on prediction
  * - "join" U64, processing time on prediction
  * The total processing time=index+prediction+join
 */
  virtual ConfigMapPtr getTimeBreakDown();
};
/**
 * @ingroup ADB_OPERATORS
 * @typedef MeanAQPIAWJOperatorPtr
 * @brief The class to describe a shared pointer to @ref MeanAQPIAWJOperator
 */
typedef std::shared_ptr<class MeanAQPIAWJOperator> MeanAQPIAWJOperatorPtr;
/**
 * @ingroup ADB_OPERATORS
 * @def newMeanAQPIAWJOperator
 * @brief (Macro) To creat a new @ref MeanAQPIAWJOperator under shared pointer.
 */
#define newMeanAQPIAWJOperator std::make_shared<OoOJoin::MeanAQPIAWJOperator>
}
#endif //INTELLISTREAM_INCLUDE_OPERATOR_MEANAQPMeanAQPIAWJOPERATOR_H_
