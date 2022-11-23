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
 * @todo The current version of @ref feedTupleS, @ref start, @ref stop is putting rotten, fix it later
 */
class IAWJOperator : public AbstractOperator{
 protected:
  Window myWindow;
  size_t intermediateResult=0;
 public:
  IAWJOperator(){}
  ~IAWJOperator(){}
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
}
#endif //INTELLISTREAM_INCLUDE_OPERATOR_IAWJOPERATOR_H_
