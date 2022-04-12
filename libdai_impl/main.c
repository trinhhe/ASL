#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <dai/alldai.h>
#include <dai/factorgraph.h>
#include <dai/bp.h>
#include <iostream>

using namespace std;
using namespace dai;

const double alpha = 0.001; // TODO: What value is suitable and small enough?
const double phi_same = 0.5+alpha; // edge potential between LIKE/LIKE & DISLIKE/DISLIKE
const double phi_diff = 0.5-alpha; // edge potiential between LIKE/DISLIKE & DISLIKE/LIKE
const int p = 3; // Normalizing factor

// First build the library yourself as described in the README in the libdai folder
// Switch to folder: cd libdai_impl
// Compile with: g++ main.c -o main -I libdai/include -L libdai/lib -ldai -lgmpxx -lgmp
//               flags needed are listed in e.g. MAKEFILE.LINUX in the libdai folder
// Examples used: example_sprinkler.cpp and example.cpp
int main(int argc, char *argv[]) {

    /* READ IN DATA */

    const char* fileLocation = argc > 1 ? argv[1] : "test_data/ratings_small.csv";

    FILE *fp = fopen(fileLocation, "r");

    if(fp == NULL){
        printf("File %s cannot be opened.", fileLocation);
    }

    char c;
    int numRatings = 0;
    while(!feof(fp)) // feof
    {
        c = fgetc(fp);
        if(c == '\n')
        {
            numRatings++;
        }
    }

	numRatings--; // Always assume the file is \n-terminated

    rewind(fp);
    
    int users[numRatings];
    int movies[numRatings];
    double ratings[numRatings];
    int targetUser = 0; // TODO: Read in (id-1)
    int numRatingsTargetUser = 0;
    double avgMovieRatingTargetUser = 0; // the mean for calculating the z-score of node potentials

    int numUsers = 0;
    int numMovies = 0;

    fscanf(fp, "%*s,%*s,%*s,%*s"); // skip line
    for (int i=0; i < numRatings; i++) {
      fscanf(fp, "%i,%i,%f,%*lu", &(users[i]), &(movies[i]), &(ratings[i])); 
      numUsers = users[i] > numUsers ? users[i] : numUsers; // user ids start from 1
      numMovies = movies[i] > numMovies ? movies[i] : numMovies; // movie ids start from 1
      users[i]-=1; // -1 to use as index directly
      movies[i]-=1; // -1 to use as index directly
      if(users[i]==targetUser){
          numRatingsTargetUser++;
          avgMovieRatingTargetUser += ratings[i];
      }
    }
    avgMovieRatingTargetUser /= numRatingsTargetUser;

    fclose(fp);

    /* DEFINE FACTOR GRAPH */
    // States x_i ∈ {LIKE,DISLIKE}
    // Factors psi_i*phi_ij (https://en.wikipedia.org/wiki/Belief_propagation#Description_of_the_sum-product_algorithm)

    // Calculate edge threshold & collect target user ratings
    int numRatingsPerUser[numUsers];
    double averageRatingPerUser[numUsers] = {0}; // for edge thresholding
    double ratingsTargetUser[numRatingsTargetUser];
    bool ratedByTargetUser[numMovies] = {false};
    for(int i=0; i<numRatings; i++){
        numRatingsPerUser[users[i]]++;
        averageRatingPerUser[users[i]]+=ratings[i];
        if(users[i] == targetUser){
            ratingsTargetUser[i] = ratings[i];
            ratedByTargetUser[movies[i]] = true;
        }
    }
    for(int i=0; i<numUsers; i++){
        averageRatingPerUser[i] /= numRatingsPerUser[i];
    }

    // Preparation for calculating node potentials
    double sampleVariance = 0; // Not sure
    for(int i=0; i<numRatingsTargetUser; i++){
        double diff = ratingsTargetUser[i] - avgMovieRatingTargetUser;
        sampleVariance += diff*diff;
    }
    sampleVariance /= (numRatingsTargetUser-1); // Not sure

    // Initialize factor variables for term psi_i*phi_ij for all nodes
    vector<Factor> factors;
    for(int i=0; i<numRatings; i++) {
        if(ratings[i]>averageRatingPerUser[users[i]]) {
            Var user(numMovies + users[i], 2);
            Var movie(movies[i], 2);
            Factor m(VarSet (user, movie)); // movie labels are smaller than user labels, so the order in the table (m.set() below) will be switched
            double dislikePotential = 0.5;
            double likePotential = 0.5;
            if(ratedByTargetUser[movies[i]]){
                double zscore = (ratings[i] - avgMovieRatingTargetUser)/sampleVariance;
				// RH: ^ should be sample standard mean, i.e., sqrt(sampleVariance), no?
                dislikePotential -= zscore/p;
                likePotential += zscore/p;
                dislikePotential = dislikePotential > 0.9 ? 0.9 
                                : (dislikePotential < 0.1 ? 0.1 : dislikePotential);
                likePotential = likePotential > 0.9 ? 0.9 
                                : (likePotential < 0.1 ? 0.1 : likePotential);
            }
            // let's assume DISLIKE=0 and LIKE=1 // TODO: also not sure
            m.set(0, dislikePotential*phi_same); // movies[i]: DISLIKE, users[i]: DISLIKE
            m.set(1, dislikePotential*phi_diff); // movies[i]: LIKE, users[i]: DISLIKE
            m.set(2, likePotential*phi_diff); // movies[i]: DISLIKE, users[i]: LIKE
            m.set(3, likePotential*phi_same); // movies[i]: LIKE, users[i]: LIKE
            factors.push_back(m);
        }
    }
    // TODO: Does the user for which we search the top-N recommendation need to be connected to all items?
    FactorGraph factorGraph(factors);

    /* TODO: RUN BELIEVE PROPAGATION */

    PropertySet opts;
    opts.set("maxiter", (size_t)10000); // Maximum number of iterations
    opts.set("tol",1e-9); // Tolerance for convergence
    opts.set("verbose",(size_t)1); // Verbosity (amount of output generated)
    BP bp(factorGraph, opts("updates",string("SEQFIX"))("logdomain",false)("inference",string("SUMPROD"))); // TODO: SEQFIX?
    // Initialize belief propagation algorithm
    bp.init();
    // Run belief propagation algorithm
    bp.run();

    for(size_t i = 0;i < factorGraph.nrVars(); i++){ // TODO: running this multiple times the result changes? Is this the wrong algorithm?
        std::cout << bp.belief(factorGraph.var(i)) << std::endl;
    }

    return 0;
}
