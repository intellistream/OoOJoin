/*! \file TestBench.h*/
//
// Created by tony on 23/11/22.
//

#ifndef INTELLISTREAM_INCLUDE_TESTBENCH_TESTBENCH_H_
#define INTELLISTREAM_INCLUDE_TESTBENCH_TESTBENCH_H_
#include <Common/Window.h>
#include <Operator/AbstractOperator.h>
#include <Utils/Logger.hpp>
using namespace INTELLI;
#define TB_INFO INTELLI_INFO
#define TB_ERROR INTELLI_ERROR
#define TB_WARNNING INTELLI_WARNING
namespace AllianceDB {

/**
* @ingroup ADB_TESTBENCH The test bench to feed data into operators
* @{
 *
 */
/**
 * @class TestBench Common/TestBench
 * @brief The test bench class to feed data
 * @ingroup ADB_TESTBENCH
 */
class TestBench {
 protected:
  void inOrderSort(std::vector<TrackTuplePtr> &arr);
  void OoOSort(std::vector<TrackTuplePtr> &arr);
  void inlineTest(void);
  void forceInOrder(std::vector<TrackTuplePtr> &arr);
  tsType timeStep=1;
 public:
  std::vector<TrackTuplePtr> rTuple;
  std::vector<TrackTuplePtr> sTuple;
  AbstractOperatorPtr testOp = nullptr;
  ConfigMapPtr opConfig;
  TestBench() {}
  ~TestBench() {}

  /**
   * @brief set the dataset to feed
   * @param _r The r tuples
   * @param _s The s tuples
   */
  void setDataSet(std::vector<TrackTuplePtr> _r, std::vector<TrackTuplePtr> _s);
  /**
  * @brief set the operator to test
  * @param op shared pointer to operator
   * @param cfg the config file to operator
   * @return whether the setting is successful
  */
  bool setOperator(AbstractOperatorPtr op, ConfigMapPtr cfg);
  /**
  * @brief test the operator under in order arrival
  * @param additionalSort whether or not additionally sort the input
  * @return the joined result
  */
  size_t inOrderTest(bool additionalSort);
  /**
  * @brief test the operator under OoO arrival
  * @param additionalSort whether or not additionally sort the input
  * @return the joined result
  */
  size_t OoOTest(bool additionalSort);
};
/**
 * @}
 */
}
#endif //INTELLISTREAM_INCLUDE_TESTBENCH_TESTBENCH_H_
