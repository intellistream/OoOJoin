/*! \file IntelliLog.hpp*/
#ifndef _UTILS_IntelliLog_H_
#define _UTILS_IntelliLog_H_
#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iostream>
#include <source_location>
#include <ctime>
/**
 *  @defgroup INTELLI_UTIL
 *  @{
* @defgroup INTELLI_UTIL_INTELLILOG Log utils
* @{
 * This package is used for logging
*/
using namespace std;
namespace INTELLI {
/**
 * @ingroup INTELLI_UTIL_INTELLILOG
 * @class ConfigMap Utils/IntelliLog.hpp
 * @brief The unified map structure to store configurations in a key-value style
 */
class IntelliLog{
 public:
  /**
   * @brief Produce a log
   * @param level The log level you want to indicate
   * @param message The log message you want to indicate
   * @param source reserved
   */
 static void log(std::string level,std::string message,std::source_location const source = std::source_location::current());

};

}
#endif