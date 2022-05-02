#include <stdio.h>
#include "recommender.h"

// First build the library yourself as described in the README in the libdai folder
// Compile with: g++ src/main.c -o build/main -I libdai/include -L libdai/lib -ldai -lgmpxx -lgmp
int main(int argc, char *argv[]) {

    const char* fileLocation = argc > 1 ? argv[1] : "../test/data/ratings_small.csv";
    const int targetUser = argc > 2 ? atoi(argv[2]) : 1; // TODO: check inputs
    const size_t k = argc > 3 ? atoi(argv[3]) : 0;

    rinput_t ri;
    initialize(&ri, fileLocation,targetUser);
    int* recommendedK = recommendK(&ri, k);

    printf("Recommended movies: \n");
    for(size_t i; i<k; i++){
        printf("%i ", recommendedK[i]);
    }
    printf("\n");

    return 0;
}
