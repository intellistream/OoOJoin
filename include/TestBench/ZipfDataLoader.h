/*! \file ZipfDataLoader.h*/


#ifndef _INCLUDE_TESTBENCH_ZIPFDATALOADER_H_
#define _INCLUDE_TESTBENCH_ZIPFDATALOADER_H_
#include <Utils/ConfigMap.hpp>
#include <Common/Tuples.h>
#include <assert.h>
#include <Utils/IntelliLog.h>
#include <TestBench/AbstractDataLoader.h>
using namespace INTELLI;
namespace OoOJoin {

/**
* @ingroup ADB_TESTBENCH The test bench to feed data into operators
* @{
 *
 */
/**
 * @defgroup ADB_TESTBENCH_DATALOADERS The classes of dataloader
 * @{
 */
/**
 * @class ZipfDataLoader TestBench/ZipfDataLoader.h
 * @brief The dataloader which allows zipf distribution of key, value, event time and arrival skewness
 * @ingroup ADB_TESTBENCH_DATALOADERS
 * @note:
 * - Must have a global config by @ref setConfig
 * - Can also have a modification config by
 *@note  Default behavior
* - create
* - setConfig and setModConfig (optional), generate R and S internally
* - call getTupleVectorS/R
 */
class ZipfDataLoader: public AbstractDataLoader{
 public:
  ConfigMapPtr cfgGlobal;
  vector<TrackTuplePtr> sTuple, rTuple;
  ZipfDataLoader() {}
  ~ZipfDataLoader() {}
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
  virtual bool setModConfig(ConfigMapPtr cfg) ;
  /**
   * @brief get the vector of s tuple
   * @return the vector
   */
  virtual vector<TrackTuplePtr> getTupleVectorS() ;
  /**
  * @brief get the vector of R tuple
  * @return the vector
  */
  virtual vector<TrackTuplePtr> getTupleVectorR();
};

/**
 * @ingroup ADB_TESTBENCH_DATALOADERS
 * @typedef ZipfDataLoaderPtr
 * @brief The class to describe a shared pointer to @ref ZipfDataLoader

 */
typedef std::shared_ptr<class ZipfDataLoader> ZipfDataLoaderPtr;
/**
 * @ingroup ADB_TESTBENCH_DATALOADERS
 * @def newZipfDataLoader
 * @brief (Macro) To creat a new @ref  ZipfDataLoader under shared pointer.
 */
#define newZipfDataLoader std::make_shared<OoOJoin::ZipfDataLoader>
/**
 * @}
 */
/**
 *
 */
}
#endif //INTELLISTREAM_INCLUDE_TESTBENCH_ZipfDataLoader_H_
