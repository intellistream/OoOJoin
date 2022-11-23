//
// Created by tony on 22/11/22.
//

#ifndef INTELLISTREAM_INCLUDE_OPERATOR_IAWJOPERATOR_H_
#define INTELLISTREAM_INCLUDE_OPERATOR_IAWJOPERATOR_H_
#include <Operator/AbstractOperator.h>
#include <Common/Window.h>

namespace AllianceDB {
/**
 * @class IAWJOperator
 * @ingroup ADB_OPERATORS
 * @class IAWJOperator Operator/IAWJOperator.h
 * @brief The intra window join (IAWJ) operator, only considers a single window
 * @note require configurations:
 * "windowLen" U64: The length of window
 * "slideLen" U64: The length of slide
 * "sLen" U64: The length of S buffer
 * "rLen" U64: The length of R buffer
 * "algo" String: The specific join algorithm (optional, default nested loop)
 * "threads" U64: The threads to conduct intra window join (optional, default 1)
 */
class IAWJOperator : public AbstractOperator {
 protected:
  Window myWindow;
  size_t intermediateResult = 0;
  string algoTag="NestedLoopJoin";
  uint64_t joinThreads=1;
 public:
  IAWJOperator() {}
  ~IAWJOperator() {}
  /**
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

};
/**
 * @cite IAWJOperatorPtr
 * @brief The class to describe a shared pointer to @ref IAWJOperator
 */
typedef std::shared_ptr<class IAWJOperator> IAWJOperatorPtr ;
/**
 * @cite newIAWJOperator
 * @brief (Macro) To creat a new @ref IAWJOperator under shared pointer.
 */
#define newIAWJOperator std::make_shared<AllianceDB::IAWJOperator>
}
#endif //INTELLISTREAM_INCLUDE_OPERATOR_IAWJOPERATOR_H_
