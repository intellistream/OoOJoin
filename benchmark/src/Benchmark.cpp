// Copyright (C) 2021 by the IntelliStream team (https://github.com/intellistream)

/**
 * @brief This is the main entry point of the entire program.
 * We use this as the entry point for benchmarking.
 */
#include <Utils/Logger.hpp>
#include <vector>
#include <OoOJoin.h>
using namespace std;
using namespace AllianceDB;
void initVec(vector<int> &a)
{
  size_t i=0;
  for (i=0;i<a.size();i++)
  {
    a[i]=i;
  }
}
void printVec(vector<int> &a)
{
  size_t i=0;
  for (i=0;i<a.size();i++)
  {
    printf("%d,",a[i]);
  }
}
class haha{
 public:
  vector <int> a;
  haha(){}
  ~haha(){}
};
int main(int argc, char **argv) {
  //Setup Logs.
  setLogLevel(getStringAsDebugLevel("LOG_DEBUG"));

  setupLogging("benchmark.log", LOG_DEBUG);
  OoOTuple ta(10, 0, 1, 2);
  //Run the test here.
  INTELLI_INFO("Nothing to run." << argc << argv);
  INTELLI_INFO(ta.toString());
  haha h1;
  h1.a=vector<int>(5);
  initVec(h1.a);
  printVec(h1.a);
}

