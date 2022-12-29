/*! \file TestBench.h*/
//
// Created by tony on 23/11/22.
//

#ifndef INTELLISTREAM_INCLUDE_TESTBENCH_TESTBENCH_H_
#define INTELLISTREAM_INCLUDE_TESTBENCH_TESTBENCH_H_
#include <Common/Window.h>
#include <Operator/AbstractOperator.h>
#include <Utils/IntelliLog.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace INTELLI;
#define TB_INFO INTELLI_INFO
#define TB_ERROR INTELLI_ERROR
#define TB_WARNNING INTELLI_WARNING
namespace OoOJoin {

/**
* @ingroup ADB_TESTBENCH The test bench to feed data into operators
* @{
 *
 */
/**
 * @class TestBench Common/TestBench
 * @brief The test bench class to feed data
 * @ingroup ADB_TESTBENCH
 * @note Require config if used
 * -"timeStep" U64 The simulation time step in us
 */
class TestBench {
 protected:
  void inOrderSort(std::vector<TrackTuplePtr> &arr);
  void OoOSort(std::vector<TrackTuplePtr> &arr);
  void inlineTest(void);
  void forceInOrder(std::vector<TrackTuplePtr> &arr);
  tsType timeStep = 1;

  void spilt(const std::string s, const std::string &c, vector<std::string> &v) {
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while (std::string::npos != pos2) {
      v.push_back(s.substr(pos1, pos2 - pos1));

      pos1 = pos2 + c.size();
      pos2 = s.find(c, pos1);
    }
    if (pos1 != s.length())
      v.push_back(s.substr(pos1));
  }
 public:
  std::vector<TrackTuplePtr> rTuple;
  std::vector<TrackTuplePtr> sTuple;
  AbstractOperatorPtr testOp = nullptr;
  ConfigMapPtr opConfig;
  size_t AQPResult = 0;
  TestBench() {}
  ~TestBench() {}
  /**
   * @brief load a dataset from csv file
   * @param fname The name of file
   * @param separator The separator in .csv, default is ","
   * @param newLine THe indicator of a new line. default is "\n"
   * @return The vector of TrackTuplePtr
   */
  std::vector<TrackTuplePtr> loadDataFromCsv(std::string fname,std::string separator = ",", std::string newLine = "\n");

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
  /**
   * @brief print the rTuples to logging system
   * @param skipZero Whether skip the tuples whose processed time is zero
   */
  void logRTuples(bool skipZero = false);
  /**
   * @brief save the  rTuples to a file
   * @param fname the name of file
   * @param skipZero Whether skip the tuples whose processed time is zero
   * @return whether the file is written
   */
  bool saveRTuplesToFile(string fname, bool skipZero = false);
  /**
   * @brief to compute average latency after run a test
   * @return the latency in us
   */
  double getAvgLatency();
  /**
   * @brief to compute the throughput after run a test
   * @return the throughput in tuples/s
   */
  double getThroughput();
  /**
   *  @brief to compute the latency t such that fraction of latency is below t
   *  @param fraction The fraction you want to set
   * @return the latency in us
   */
  double getLatencyPercentage(double fraction);
  /**
   * @brief get the break down information of processing time from the join operator tested
   * @warning should check the nullptr of output
   * @return The ConfigMapPtr which contains breakdown information, null if no breakdown supported
   */
  ConfigMapPtr getTimeBreakDown();
};
/**
 * @}
 */
}
#endif //INTELLISTREAM_INCLUDE_TESTBENCH_TESTBENCH_H_
