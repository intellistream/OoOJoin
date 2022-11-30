//
// Created by tony on 22/11/22.
//

#include <Common/Tuples.h>
using namespace AllianceDB;

AllianceDB::Tuple::Tuple(AllianceDB::keyType k) { key = k; }
AllianceDB::Tuple::Tuple(AllianceDB::keyType k, AllianceDB::valueType v) {
  key = k;
  payload = v;
  eventTime = 0;
}
AllianceDB::Tuple::Tuple(AllianceDB::keyType k, AllianceDB::valueType v, tsType et) {
  key = k;
  payload = v;
  eventTime = et;
}
std::string AllianceDB::Tuple::toString() {
  std::string tmp;
  tmp.append("\t\tkey:" + std::to_string(key));
  tmp.append("\t\tvalue:" + std::to_string(payload));
  tmp.append("\t\tevent time:" + std::to_string(eventTime));
  return tmp;
}
std::string AllianceDB::OoOTuple::toString() {
  std::string tmp;
  tmp.append(Tuple::toString());
  tmp.append("\t\tarrival time:" + std::to_string(arrivalTime));
  return tmp;
}

std::string AllianceDB::TrackTuple::toString() {
  std::string tmp;
  tmp.append(OoOTuple::toString());
  tmp.append("\t\tprocessed time:" + std::to_string(processedTime));
  return tmp;
}