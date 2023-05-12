#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <Common/LinearVAE.h>
#include <vector>
#include <OoOJoin.h>
#include "SimpleTest.cpp"
TEST_CASE("Test AIOperator on Pretrain mode")
{
  int a = 0;
  string configName, outPrefix = "";
  configName = "config_AI_TRAIN.csv";
  ConfigMapPtr cfg = newConfigMap();
  cfg->fromFile(configName);
  cfg->edit("operator", "AI");
  cfg->edit("aiMode", "pretrain");
  a = runTestBenchAdj(cfg, configName, outPrefix);
  REQUIRE(a == 1);
}

