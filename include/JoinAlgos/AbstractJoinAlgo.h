/*! \file AbstractJoinAlgo.h*/
//
// Created by tony on 11/03/22.
//

#ifndef _JOINALGO_ABSTRACTJOINALGO_H_
#define _JOINALGO_ABSTRACTJOINALGO_H_
#include <Common/Window.h>
#include <string>
#include <memory>
#include <assert.h>
using namespace INTELLI;
namespace AllianceDB {
/**
* @ingroup ADB_JOINALGOS The specific join algorithms
* @{
* State-of-art joins algorithms. We use a register to table called @ref JoinAlgoTable to manage and access different algos in an unified way, and user-defined
 * new algos should also be registered in that table.
 * */
/**
 * @defgroup ADB_JOINALGOS_ABSTRACT Common Abstraction and Interface
 * @{
 */
/**
 * @ingroup ADB_JOINALGOS_ABSTRACT
 * @class AbstractJoinAlgo JoinAlgos/AbstractJoinAlgo.h
 * @brief The abstraction to describe a join algorithm, providing virtual function of join
 */
class AbstractJoinAlgo {
 protected:
  std::string nameTag;
  struct timeval timeBaseStruct;
 public:
  AbstractJoinAlgo() {
    setAlgoName("NULL");
  }
  ~AbstractJoinAlgo() {}

  /**
  * @brief The function to execute join,
  * @param windS The window of S tuples
   * @param windR The window of R tuples
   * @param threads The threads for executing this join
  * @return The joined tuples
  */
  virtual size_t join(C20Buffer<AllianceDB::TrackTuplePtr> windS,
                      C20Buffer<AllianceDB::TrackTuplePtr> windR,
                      int threads = 1);
  /**
   * @brief set the name of algorithm
   * @param name Algorithm name
   */
  void setAlgoName(std::string name) {
    nameTag = name;
  }
  /**
  * @brief get the name of algorithm
  * @return The name
  */
  std::string getAlgoName() {
    return nameTag;
  }

  /**
   * @brief Synchronize the time structure with outside setting
   * @param tv The outside time structure
   */
  void syncTimeStruct(struct timeval tv) {
    timeBaseStruct = tv;
  }
};
typedef std::shared_ptr<AbstractJoinAlgo> AbstractJoinAlgoPtr;
#define  newAbstractJoinAlgo() make_shared<AbstractJoinAlgo>()
/**
 * @}
 * @}
 */
}

#endif //ALIANCEDB_INCLUDE_JOINALGO_ABSTRACTJOINALGO_H_
