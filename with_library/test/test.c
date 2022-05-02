#include <stdio.h>
#include "../src/recommend.h"
#include <assert.h>
#include <stdbool.h>
#include <dirent.h> 

int exitCode = EXIT_SUCCESS;

#define ASSERT_EQUAL( expected, actual, length )                                             \
{                                                                                            \
  for(size_t i=0; i<length; i++)                                                             \
  {                                                                                          \
    if(expected[i] != actual[i])                                                             \
    {                                                                                        \
        fprintf(stderr, "expected: %s, actual: %s\n", "x", "y");                             \
        fprintf(stderr, "Error occurred at %s/%s:%i\n", __func__, __FILE__, __LINE__);       \
        exitCode = EXIT_FAILURE;                                                             \
    }                                                                                        \
  }                                                                                          \
}
// TODO: replace x and y with stringified array

typedef struct {
    const int targetUser;
    const char* dataFile;
    const int expectedRecommendations[3];
} test_case_t;

test_case_t testCases[] = {
    {1, "../test/data/ratings_small.csv", {2,2,1}}
};

void test(test_case_t *tcase){
    const size_t k = sizeof(tcase->expectedRecommendations)/size_t(tcase->expectedRecommendations[0]);
    rinput_t ri;
    initialize(&ri,tcase->dataFile,tcase->targetUser);
    int* actualRecommendations = recommendK(&ri, k);
    ASSERT_EQUAL(tcase->expectedRecommendations, actualRecommendations, k);
}

int main(int argc, char *argv[]) {

    size_t numTestCases = sizeof(testCases)/sizeof(testCases[0]);
    for(size_t i=0; i<numTestCases; i++){
        test(&testCases[i]);
    }

    return exitCode;
}
