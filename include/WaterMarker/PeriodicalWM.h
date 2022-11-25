/*! \file PeriodicalWM.h*/
//
// Created by tony on 25/11/22.
//

#ifndef _INCLUDE_WATERMARKER_PERIODICALWM_H_
#define _INCLUDE_WATERMARKER_PERIODICALWM_H_
#include <WaterMarker/AbstractWaterMarker.h>
namespace AllianceDB {
/**
 * @ingroup ADB_WATERMARKER The watermark generator
 *@class PeriodicalWM WaterMarker/PeriodicalWM.h
 * @brief The class which generates watermark periodically
 * @todo multi window support is not done yet, left for future, but we do preserve the interfaces
 * @note require configurations:
 * - "errorBound" Double The assigned error bound
 * - "timeStep" U64 The simulation time step in us
 * - "watermarkPeriod" U64 The watermark period, if not set will equal to window length
 */
class PeriodicalWM : public AbstractWaterMarker{
 protected:
  /**
  * @brief The period to generate watermark, set by outside configuration
  */
  tsType watermarkPeriod=0;
  /**
  * @brief The Delta value for next watermark
  */
  tsType nextWMDelta=0;
  /**
   * @note currently only support single window
   */
  tsType windowLen=0;
  tsType nextWMPoint=0;
  bool isReachWMPoint(TrackTuplePtr tp);
 public:
  PeriodicalWM(){}
  ~PeriodicalWM(){}
  /**
 * @brief Set the config map related to this operator
 * @param cfg The config map
  * @return bool whether the config is successfully set
 */
  virtual bool setConfig(ConfigMapPtr cfg);
  /**
  * @brief creat a window
   * @param tBegin The begin event time of the window
   * @param tEnd The end event time of the window
   * @return the id of created window
  */
  virtual size_t creatWindow(tsType tBegin, tsType tEnd);
  /**
 * @brief report a tuple s into the watermark generator
 * @param ts The tuple
  * @param wid The id of window
  * @return bool, whether generate watermark after receiving this Tuple
 */
  virtual bool reportTupleS(TrackTuplePtr ts, size_t wid = 1);

  /**
    * @brief Report a tuple R into the watermark generator
    * @param tr The tuple
    * @param wid The id of window
    *  @return bool, bool, whether generate watermark after receiving this Tuple
    */
  virtual bool reportTupleR(TrackTuplePtr tr, size_t wid = 1);
};
/**
 * @cite PeriodicalWMPtr
 * @brief The class to describe a shared pointer to @ref  PeriodicalWM
 */
typedef std::shared_ptr<class PeriodicalWM> PeriodicalWMPtr;
/**
 * @cite newPeriodicalWM
 * @brief (Macro) To creat a new @ref PeriodicalWM under shared pointer.
 */
#define newPeriodicalWM std::make_shared<AllianceDB::PeriodicalWM>
}
#endif //INTELLISTREAM_INCLUDE_WATERMARKER_PERIODICALWM_H_
