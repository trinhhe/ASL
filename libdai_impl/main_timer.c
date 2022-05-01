#include <stdio.h>
#include <stdlib.h>
#include <dai/alldai.h>
#include <dai/factorgraph.h>
#include <dai/bp.h>
#include <iostream>
#include "../measurement_utils/tsc_x86.h"

#define REP 10

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
      fscanf(fp, "%i,%i,%lf,%*lu", &(users[i]), &(movies[i]), &(ratings[i])); 
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


    myInt64 start, stop;
    double total = 0;
    int number_vars;
    for (int j = 0; j < REP; j++) {
        start = start_tsc();
        /* DEFINE FACTOR GRAPH */
        // States x_i ∈ {LIKE,DISLIKE}
        // Factors psi_i*phi_ij (https://en.wikipedia.org/wiki/Belief_propagation#Description_of_the_sum-product_algorithm)

        // Calculate edge threshold & collect target user ratings
        int numRatingsPerUser[numUsers] = {0};
        double averageRatingPerUser[numUsers] = {0}; // for edge thresholding
        double ratingsTargetUser[numRatingsTargetUser] = {0};
        double ratedByTargetUser[numMovies] = {-1};
        int idxRatingsTargetUser = 0;
        for(int i=0; i<numRatings; i++){
            numRatingsPerUser[users[i]]++;
            averageRatingPerUser[users[i]]+=ratings[i];
            if(users[i] == targetUser){
                ratingsTargetUser[idxRatingsTargetUser++] = ratings[i];
                ratedByTargetUser[movies[i]] = ratings[i];
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

        /* HENRY'S APPROACH FOR COMPARISON */

        // Initialize factor variables for term psi_i, phi_ij for all nodes
        vector<Factor> factors2;
        for(int i=0; i<numUsers; i++){
            Var user(numMovies + i, 2);
            Factor m(user);
            m.set(0, 0.5);
            m.set(1, 0.5);
            factors2.push_back(m);
        }
        for(int i=0; i<numMovies; i++){
            Var movie(i, 2);
            Factor m(movie);
            double dislikePotential = 0.5;
            double likePotential = 0.5;
            if(ratedByTargetUser[i]!=-1){
                double zscore = (ratedByTargetUser[i] - avgMovieRatingTargetUser)/sqrt(sampleVariance);
                dislikePotential -= zscore/p;
                likePotential += zscore/p;
                dislikePotential = dislikePotential > 0.9 ? 0.9 
                                : (dislikePotential < 0.1 ? 0.1 : dislikePotential);
                likePotential = likePotential > 0.9 ? 0.9 
                                : (likePotential < 0.1 ? 0.1 : likePotential);
            }
            m.set(0, dislikePotential);
            m.set(1, likePotential);
            factors2.push_back(m);
        }
        for(int i=0; i<numRatings; i++) {
            if(ratings[i]>=averageRatingPerUser[users[i]]) {
                Var user(numMovies + users[i], 2);
                Var movie(movies[i], 2);
                Factor m(VarSet (user, movie)); // movie labels are smaller than user labels, so the order in the table (m.set() below) will be switched
                // let's assume DISLIKE=0 and LIKE=1 // TODO: also not sure
                m.set(0, phi_same); // movies[i]: DISLIKE, users[i]: DISLIKE
                m.set(1, phi_diff); // movies[i]: LIKE, users[i]: DISLIKE
                m.set(2, phi_diff); // movies[i]: DISLIKE, users[i]: LIKE
                m.set(3, phi_same); // movies[i]: LIKE, users[i]: LIKE
                factors2.push_back(m);
            }
        }
        FactorGraph factorGraph2(factors2);
        number_vars = factorGraph2.nrVars();
        // Run believe propagation
        PropertySet opts;
        opts.set("maxiter", (size_t)10); // Maximum number of iterations
        opts.set("tol",1e-20); // Tolerance for convergence
        opts.set("verbose",(size_t)0); // Verbosity (amount of output generated)
        BP bp2(factorGraph2, opts("updates",string("SEQFIX"))("logdomain",false)("inference",string("SUMPROD"))); // TODO: SEQFIX?
        bp2.init(); // Initialize belief propagation algorithm
        bp2.run(); // Run belief propagation algorithm

        for(size_t i = 0;i < factorGraph2.nrVars(); i++){ // The first three should correspond to movies, I think
            bp2.belief(factorGraph2.var(i));
        }
        stop = stop_tsc(start);
        total += (double) stop;

    }
    total /= REP;
    // count flops
    int flops = 0;
    for(int i=0; i<numRatings; i++)
        flops++;
    
    for(int i=0; i<numUsers; i++)
        flops++;
    
    for(int i=0; i<numRatingsTargetUser; i++){
        flops +=3;
    }
    flops++;
    cout << number_vars << endl;
    cout << total << endl;
    return 0;
}