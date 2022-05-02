#include <stdio.h>
#include "../src/recommend.h"
#include <assert.h>
#include <stdbool.h>
#include <dirent.h> 

// Compile with: g++ test/test.c -o build/test -I libdai/include -L libdai/lib -ldai -lgmpxx -lgmp
// For executing: cd to /build and run ./test

int exitCode = EXIT_SUCCESS;

char* asString(const int * arr){
    size_t arrLength = sizeof(arr)/sizeof(int);

    char* c_arr = (char*)malloc((arrLength*(sizeof(int)+1))+4*sizeof(char));
    c_arr[0] = '[';
    c_arr[1] = ' ';
    c_arr[2*(arrLength+1)] = ']';
    c_arr[2*(arrLength+1)+1] = '\0';
    if(arrLength < 1){
        return c_arr;
    }

    for (int i=0; i < arrLength; i++)
    {   
        snprintf(&c_arr[2*(i+1)], sizeof(int), "%i", arr[i]);
        c_arr[2*(i+1)+1] = ' ';
    }
    return c_arr;  
}

#define TESTFILEDIR "../test/data/"
#define LOCATION(file) TESTFILEDIR file

#define ASSERT_EQUAL( expected, actual, length )                                             \
{                                                                                            \
  for(size_t i=0; i<length; i++)                                                             \
  {                                                                                          \
    if(expected[i] != actual[i])                                                             \
    {                                                                                        \
        fprintf(stderr, "expected: %s, actual: %s\n", asString(expected), asString(actual)); \
        fprintf(stderr, "Error occurred at %s/%s:%i\n", __func__, __FILE__, __LINE__);       \
        exitCode = EXIT_FAILURE;                                                             \
    }                                                                                        \
  }                                                                                          \
}

typedef struct {
    const int targetUser;
    const char* dataFile;
    const int *expectedRecommendations;
} test_case_t;

test_case_t testCases[] = {
    {1, LOCATION("ratings_small.csv"), (const int []){3,2,1}},
    {2, LOCATION("input1.csv"), (const int []){3,2}} // TODO: Find more test cases
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
