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
#include <Utils/ConfigMap.hpp>

#include <Utils/Logger.hpp>
#define OP_INFO INTELLI_INFO
#define OP_ERROR INTELLI_ERROR
#define OP_WARNNING INTELLI_WARNING

using namespace INTELLI;
namespace OoOJoin {
/**
 * @ingroup ADB_OPERATORS
 * @class AbstractOperator Operator/AbstractOperator.h
 * @brief The abstraction to describe a join operator, providing virtual function of using the operation
 * @note require configurations:
 * - "windowLen" U64: The length of window, real-world time in us
 * - "slideLen" U64: The length of slide, real world time in us
 * - "sLen" U64: The length of S buffer
 * - "rLen" U64: The length of R buffer
 * - "timeStep" U64. Internal simulation step in us
 */
/**
* @todo Finish the watermark generator part
*/
class AbstractOperator {
 protected:
  struct timeval timeBaseStruct;
  size_t windowLen = 0;
  size_t slideLen = 0;
  size_t sLen = 0, rLen = 0;
  int threads = 0;
  tsType timeStep = 0;
 public:
  ConfigMapPtr config = nullptr;
  AbstractOperator() {}
  ~AbstractOperator() {}

  /**
   * @brief Set the window parameters of the whole operator
   * @param _wl window length
   * @param _sli slide length
   */
  void setWindow(size_t _wl, size_t _sli) {
    windowLen = _wl;
    slideLen = _sli;
  }
  /**
   * @brief Set buffer length of S and R buffer
   * @param _sLen the length of S buffer
   * @param _rLen the length of R buffer
   */
  void setBufferLen(size_t _sLen, size_t _rLen) {
    sLen = _sLen;
    rLen = _rLen;
  }

  /**
   * @brief Synchronize the time structure with outside setting
   * @param tv The outside time structure
   */
  void syncTimeStruct(struct timeval tv) {
    timeBaseStruct = tv;
  }
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
 * @ingroup ADB_OPERATORS
 * @typedef AbstractOperatorPtr
 * @brief The class to describe a shared pointer to @ref AbstractOperator
 */
typedef std::shared_ptr<class AbstractOperator> AbstractOperatorPtr;
/**
 * @ingroup ADB_OPERATORS
 * @def newAbstractOperator
 * @brief (Macro) To creat a new @ref AbstractOperator under shared pointer.
 */
#define newAbstractOperator std::make_shared<OoOJoin::AbstractOperator>
}
#endif //INTELLISTREAM_INCLUDE_OPERATOR_ABSTRACTOPERATOR_H_
