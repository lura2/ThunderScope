#ifndef EVTester_hpp
#define EVTester_hpp

#include "common.hpp"

#define TEST_RUN_TIME 20

void TestSincInterpolation();

void TestDataThroughput();

void testTriggerThroughput();

void testBenchmark();

void testCsv(char * filename);

void initializePipeline();

void cleanPipeline();

#endif /* EVTester_hpp */
