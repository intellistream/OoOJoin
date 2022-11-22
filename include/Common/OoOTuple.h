/*! \file OoOTuple.h*/
//
// Created by tony on 22/11/22.
//

#ifndef INTELLISTREAM_OOOTUPLE_H
#define INTELLISTREAM_OOOTUPLE_H
#include <stdint.h>
#include <string>
#include <memory>
namespace AllianceDB {

/**
* @ingroup INTELLI_COMMON_BASIC Basic Definitions and Data Structures
* @{
* We define the classes of Tuple, window, queue, etc. here
*/
typedef uint64_t keyType;    /*!< Type of the join key, default uint64_t */
typedef uint64_t valueType;  /*!< Type of the payload, default uint64_t */
typedef uint64_t tsType;  /*!< Type of the time stamp, default uint64_t */
/**
* @class Tuple Common/OoOTuple.h
* @brief The class to describe a normal tuple
* @ingroup INTELLI_COMMON_BASIC
*/
class Tuple {
 public:
  keyType key; /*!< The key used for relational join*/
  valueType payload; /*!< The payload, can also be pointer*/
  tsType eventTime = 0;/*!< The time when event happen*/
  /**
   * @brief construct with key
   * @param k the key
   */
  Tuple(keyType k);
  /**
 * @brief construct with key and value
 * @param k the key
   *@param v the value of payload
 */
  Tuple(keyType k, valueType v);
  /**
 * @brief construct with key, value and eventTime
 * @param k the key
   *@param v the value of payload
   * @param et the event time
 */
  Tuple(keyType k, valueType v, tsType et);
  /**
 * @brief construct with key, value ,eventTime and arrivalTime
 * @param k the key
   *@param v the value of payload
   * @param et the event time
   * @param at the arrival time
 */
  Tuple(keyType k, valueType v, tsType et, tsType at);
  /**
   * @brief convert the tuple into std string
   * @return the std string for this tuple
   */
  std::string toString();
  ~Tuple() {}
};
/**
* @cite TuplePtr
* @brief The class to describe a shared pointer to @ref Tuple
*/
typedef std::shared_ptr<class Tuple> TuplePtr;
/**
* @class OoOTuple Common/OoOTuple.h
* @brief The class to describe a tuple, which allows out-of-order
 * @ingroup INTELLI_COMMON_BASIC
 */
class OoOTuple : public Tuple {
 public:
  tsType arrivalTime = 0; /*!< The time when it arrives*/
  /**
   * @brief construct with key
   * @param k the key
   */
  OoOTuple(keyType k) : Tuple(k) {}
  /**
 * @brief construct with key and value
 * @param k the key
   *@param v the value of payload
 */
  OoOTuple(keyType k, valueType v) : Tuple(k, v) { arrivalTime = 0; }
  /**
 * @brief construct with key, value and eventTime
 * @param k the key
   *@param v the value of payload
   * @param et the event time
 */
  OoOTuple(keyType k, valueType v, tsType et) : Tuple(k, v, et) { arrivalTime = et; }
  /**
 * @brief construct with key, value ,eventTime and arrivalTime
 * @param k the key
   *@param v the value of payload
   * @param et the event time
   * @param at the arrival time
 */
  OoOTuple(keyType k, valueType v, tsType et, tsType at) : Tuple(k, v, et) { arrivalTime = at; }
  /**
   * @brief convert the tuple into std string
   * @return the std string for this tuple
   */
  std::string toString();
  ~OoOTuple() {}
};
/**
 * @cite OoOTuplePtr
 * @brief The class to describe a shared pointer to @ref OoOTuple
 */
typedef std::shared_ptr<class OoOTuple> OoOTuplePtr;
/**
* @}
*/
// AlianceDB

}

#endif //INTELLISTREAM_OOOTUPLE_H
