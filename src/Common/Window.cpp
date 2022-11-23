//
// Created by tony on 22/11/22.
//

#include <Common/Window.h>
using namespace std;
using namespace INTELLI;
using namespace AllianceDB;
void AllianceDB::Window::setRange(AllianceDB::tsType ts, AllianceDB::tsType te) {
  startTime = ts;
  endTime = te;
}
AllianceDB::Window::Window(AllianceDB::tsType ts, AllianceDB::tsType te) {
  setRange(ts, te);
}
void AllianceDB::Window::init(size_t sLen, size_t rLen, size_t _sysId) {
  windowS = INTELLI::C20Buffer<TrackTuplePtr>(sLen);
  windowR = INTELLI::C20Buffer<TrackTuplePtr>(rLen);
  windowID = _sysId;
}
bool AllianceDB::Window::feedTupleS(AllianceDB::TrackTuplePtr ts) {
  /* if (ts->eventTime > endTime || ts->eventTime < startTime) {
     return false;
   }*/
  windowS.append(ts);
  return true;
}

bool AllianceDB::Window::feedTupleR(AllianceDB::TrackTuplePtr tr) {
  if (tr->eventTime > endTime || tr->eventTime < startTime) {
    return false;
  }
  windowR.append(tr);
  return true;
}
bool AllianceDB::Window::reset() {
  windowS.reset();
  windowR.reset();
  return true;
}