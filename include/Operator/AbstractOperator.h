/*! \file AbstractOperator.h*/
//
// Created by tony on 22/11/22.
//

#ifndef ADB_INCLUDE_OPERATOR_ABSTRACTOPERATOR_H_
#define ADB_INCLUDE_OPERATOR_ABSTRACTOPERATOR_H_
#include <Common/Window.h>
#include <Common/Tuples.h>
#include <Utils/UtilityFunctions.hpp>
#include <assert.h>
using namespace INTELLI;
namespace AllianceDB {
/**
 * @ingroup ADB_OPERATORS
 * @class AbstractOperator Operator/AbstractOperator.h
 * @brief The abstraction to describe a join operator, providing virtual function of using the operation
 */
/**
* @todo The current parameter configuration is messy, try to fix it as soon as possible.
*/
class AbstractOperator {
 protected:
  struct timeval timeBaseStruct;
  size_t windowLen=0;
  size_t slideLen=0;
  size_t sLen=0,rLen=0;
 public:
  AbstractOperator(){}
  ~AbstractOperator(){}
  /**
   * @brief Set the window parameters of the whole operator
   * @param _wl window length
   * @param _sli slide length
   */
  void setWindow(size_t _wl,size_t _sli)
  {
    windowLen=_wl;
    slideLen=_sli;
  }
  /**
   * @brief Set buffer length of S and R buffer
   * @param _sLen the length of S buffer
   * @param _rLen the length of R buffer
   */
  void setBufferLen(size_t _sLen,size_t _rLen)
  {
   sLen=_sLen;
   rLen=_rLen;
  }

  /**
   * @brief Synchronize the time structure with outside setting
   * @param tv The outside time structure
   */
  void syncTimeStruct(struct timeval tv)
  {
    timeBaseStruct=tv;
  }
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
#endif //INTELLISTREAM_INCLUDE_OPERATOR_ABSTRACTOPERATOR_H_
