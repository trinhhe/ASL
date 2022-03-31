#include <stdio.h>
#include <stdlib.h>
#include <dai/factorgraph.h>

using namespace std;
using namespace dai;

// First build the library yourself as described in the README in the libdai folder
// g++ main.c -o main -I libdai/include -L libdai/lib -ldai -lgmpxx -lgmp
// flags needed are listed in e.g. MAKEFILE.LINUX in the libdai folder
int main(int argc, char *argv[]) {
    const char* fileLocation = argc > 1 ? argv[1] : "test_data/ratings_small.csv";

    FILE *fp = fopen(fileLocation, "r");

    if(fp == NULL){
        printf("File %s cannot be opened.", fileLocation);
    }

    char c;
    int lines = 0;
    while(!feof(fp)) // feof
    {
        c = fgetc(fp);
        if(c == '\n')
        {
            lines++;
        }
    }

    rewind(fp);
    
    int users[lines];
    int movies[lines];
    float ratings[lines];

    rewind(fp);

    fscanf(fp, "%*s,%*s,%*s,%*s"); // skip line
    for (int i=0; i < lines; i++) {
      fscanf(fp, "%i,%i,%f,%*lu", &(users[i]), &(movies[i]), &(ratings[i]));
    }

    fclose(fp);

    //printf("lines %i\n", lines);
    //printf("ratings: %f %f %f %f %f\n", ratings[0], ratings[1], ratings[2], ratings[3], ratings[4]);

    // TODO: Define a factor graph
    // States x_i ∈ {LIKE,DISLIKE}
    // Factors m_ij

    // This example program illustrates how to construct a factorgraph
    // by means of the sprinkler network example discussed at
    // http://www.cs.ubc.ca/~murphyk/Bayes/bnintro.html
 /*    Var C(0, 2); // Define binary variable Cloudy (with label 0)
    Var S(1, 2); // Define binary variable Sprinkler (with label 1)
    Var R(2, 2); // Define binary variable Rain (with label 2)
    Var W(3, 2); // Define binary variable Wetgrass (with label 3)
    // Define probability distribution for C
    Factor P_C( C );
    P_C.set(0, 0.5); // C = 0
    P_C.set(1, 0.5); // C = 1
    // Define conditional probability of S given C
    Factor P_S_given_C( VarSet( S, C ) );
    P_S_given_C.set(0, 0.5); // C = 0, S = 0
    P_S_given_C.set(1, 0.9); // C = 1, S = 0
    P_S_given_C.set(2, 0.5); // C = 0, S = 1
    P_S_given_C.set(3, 0.1); // C = 1, S = 1
    // Define conditional probability of R given C
    Factor P_R_given_C( VarSet( R, C ) );
    P_R_given_C.set(0, 0.8); // C = 0, R = 0
    P_R_given_C.set(1, 0.2); // C = 1, R = 0
    P_R_given_C.set(2, 0.2); // C = 0, R = 1
    P_R_given_C.set(3, 0.8); // C = 1, R = 1
    // Define conditional probability of W given S and R
    Factor P_W_given_S_R( VarSet( S, R ) | W );
    P_W_given_S_R.set(0, 1.0); // S = 0, R = 0, W = 0
    P_W_given_S_R.set(1, 0.1); // S = 1, R = 0, W = 0
    P_W_given_S_R.set(2, 0.1); // S = 0, R = 1, W = 0
    P_W_given_S_R.set(3, 0.01); // S = 1, R = 1, W = 0
    P_W_given_S_R.set(4, 0.0); // S = 0, R = 0, W = 1
    P_W_given_S_R.set(5, 0.9); // S = 1, R = 0, W = 1
    P_W_given_S_R.set(6, 0.9); // S = 0, R = 1, W = 1
    P_W_given_S_R.set(7, 0.99); // S = 1, R = 1, W = 1
    // Build factor graph consisting of those four factors
    vector<Factor> SprinklerFactors;
    SprinklerFactors.push_back( P_C );
    SprinklerFactors.push_back( P_R_given_C );
    SprinklerFactors.push_back( P_S_given_C );
    SprinklerFactors.push_back( P_W_given_S_R );
    FactorGraph SprinklerNetwork( SprinklerFactors );
*/
    // TODO: Run believe propagation

    /*
    // Construct a BP (belief propagation) object from the FactorGraph fg
    // using the parameters specified by opts and two additional properties,
    // specifying the type of updates the BP algorithm should perform and
    // whether they should be done in the real or in the logdomain
    BP bp(fg, opts("updates",string("SEQRND"))("logdomain",false));
    // Initialize belief propagation algorithm
    bp.init();
    // Run belief propagation algorithm
    bp.run();
   */

    return 0;
}