#ifndef LIBIMPL_RECOMMENDER_H
#define LIBIMPL_RECOMMENDER_H

#include <stdlib.h>
#include <dai/alldai.h>
#include <dai/factorgraph.h>
#include <dai/bp.h>
#include <iostream>
#include <assert.h>

using namespace std;
using namespace dai;

/* CONSTANTS */

const double alpha = 0.001; // TODO: What value is suitable and small enough?
const double phi_same = 0.5+alpha; // edge potential between LIKE/LIKE & DISLIKE/DISLIKE
const double phi_diff = 0.5-alpha; // edge potiential between LIKE/DISLIKE & DISLIKE/LIKE
const int p = 3; // Normalizing factor

/* INPUT DATA STRUCTURE */

typedef struct {
    size_t numRatings;
    size_t numUsers;
    size_t numMovies;
    int targetUser;
    size_t numRatingsTargetUser;
    float avgMovieRatingTargetUser;
    int *users;
    int *movies;
    float *ratings;
} rinput_t;

void initialize(rinput_t *input, const char* inputFile, const int targetUser){
    /* READ IN DATA */

    FILE *fp = fopen(inputFile, "r");

    if(fp == NULL){
        printf("File %s cannot be opened.", inputFile);
    }

    char c;
    input->numRatings = 0;
    while(!feof(fp)) // feof
    {
        c = fgetc(fp);
        if(c == '\n')
        {
            input->numRatings++;
        }
    }

    input->numRatings--; // Always assume the file is \n-terminated

    rewind(fp);
    
    input->users = (int*) malloc(input->numRatings * sizeof(int));
    input->movies = (int*) malloc(input->numRatings * sizeof(int));
    input->ratings = (float*) malloc(input->numRatings * sizeof(float));
    input->targetUser = targetUser - 1;
    input->numRatingsTargetUser = 0;
    input->avgMovieRatingTargetUser = 0; // the mean for calculating the z-score of node potentials

    input->numUsers = 0;
    input->numMovies = 0;

    fscanf(fp, "%*s,%*s,%*s,%*s"); // skip line
    for (size_t i=0; i < input->numRatings; i++) {
      fscanf(fp, "%i,%i,%f,%*lu", &(input->users[i]), &(input->movies[i]), &(input->ratings[i])); 
      input->numUsers = input->users[i] > input->numUsers ? input->users[i] : input->numUsers; // user ids start from 1
      input->numMovies = input->movies[i] > input->numMovies ? input->movies[i] : input->numMovies; // movie ids start from 1
      input->users[i]-=1; // -1 to use as index directly
      input->movies[i]-=1; // -1 to use as index directly
      if(input->users[i]==input->targetUser){
          input->numRatingsTargetUser++;
          input->avgMovieRatingTargetUser += input->ratings[i];
      }
    }
    input->avgMovieRatingTargetUser /= input->numRatingsTargetUser;

    fclose(fp);
}

// Examples used: example_sprinkler.cpp and example.cpp
int* recommendK(rinput_t *input, const size_t k){

    /* DEFINE FACTOR GRAPH */
    // States x_i ∈ {LIKE,DISLIKE}
    // Factors psi_i*phi_ij (https://en.wikipedia.org/wiki/Belief_propagation#Description_of_the_sum-product_algorithm)

    // Calculate edge threshold & collect target user ratings
    size_t numRatingsPerUser[input->numUsers] = {0};
    float averageRatingPerUser[input->numUsers] = {0}; // for edge thresholding
    float ratingsTargetUser[input->numRatingsTargetUser] = {0};
    float ratedByTargetUser[input->numMovies] = {-1};
    size_t idxRatingsTargetUser = 0;
    for(size_t i=0; i<input->numRatings; i++){
        numRatingsPerUser[input->users[i]]++;
        averageRatingPerUser[input->users[i]]+=input->ratings[i];
        if(input->users[i] == input->targetUser){
            ratingsTargetUser[idxRatingsTargetUser++] = input->ratings[i];
            ratedByTargetUser[input->movies[i]] = input->ratings[i];
        }
    }
    for(size_t i=0; i<input->numUsers; i++){
        averageRatingPerUser[i] /= numRatingsPerUser[i];
    }

    // Preparation for calculating node potentials
    float sampleVariance = 0; // Not sure
    for(size_t i=0; i<input->numRatingsTargetUser; i++){
        float diff = ratingsTargetUser[i] - input->avgMovieRatingTargetUser;
        sampleVariance += diff*diff;
    }
    assert(input->numRatingsTargetUser > 1);
    sampleVariance /= (input->numRatingsTargetUser-1); // Not sure

     // Initialize factor variables for term psi_i, phi_ij for all nodes
    vector<Factor> factors;
    for(size_t i=0; i<input->numUsers; i++){
        Var user(input->numMovies + i, 2);
        Factor m(user);
        m.set(0, 0.5);
        m.set(1, 0.5);
        factors.push_back(m);
    }
    for(size_t i=0; i<input->numMovies; i++){
        Var movie(i, 2);
        Factor m(movie);
        float dislikePotential = 0.5;
        float likePotential = 0.5;
        if(ratedByTargetUser[i]!=-1){
            float zscore = (ratedByTargetUser[i] - input->avgMovieRatingTargetUser)/sqrt(sampleVariance);
            dislikePotential -= zscore/p;
            likePotential += zscore/p;
            dislikePotential = dislikePotential > 0.9 ? 0.9 
                            : (dislikePotential < 0.1 ? 0.1 : dislikePotential);
            likePotential = likePotential > 0.9 ? 0.9 
                            : (likePotential < 0.1 ? 0.1 : likePotential);
        }
        m.set(0, dislikePotential);
        m.set(1, likePotential);
        factors.push_back(m);
    }
    for(size_t i=0; i<input->numRatings; i++) {
        if(input->ratings[i]>=averageRatingPerUser[input->users[i]]) {
            Var user(input->numMovies + input->users[i], 2);
            Var movie(input->movies[i], 2);
            Factor m(VarSet (user, movie)); // movie labels are smaller than user labels, so the order in the table (m.set() below) will be switched
            // let's assume DISLIKE=0 and LIKE=1 // TODO: also not sure
            m.set(0, phi_same); // movies[i]: DISLIKE, users[i]: DISLIKE
            m.set(1, phi_diff); // movies[i]: LIKE, users[i]: DISLIKE
            m.set(2, phi_diff); // movies[i]: DISLIKE, users[i]: LIKE
            m.set(3, phi_same); // movies[i]: LIKE, users[i]: LIKE
            factors.push_back(m);
        }
    }
    FactorGraph factorGraph(factors);

    // Run believe propagation
    PropertySet opts;
    opts.set("maxiter", (size_t)10000); // Maximum number of iterations
    opts.set("tol",1e-9); // Tolerance for convergence
#ifdef DEBUG
    opts.set("verbose",(size_t)1); // Verbosity (amount of output generated)
#else
    opts.set("verbose", (size_t)0);
#endif
    BP bp(factorGraph, opts("updates",string("SEQFIX"))("logdomain",false)("inference",string("SUMPROD"))); // TODO: SEQFIX?
    bp.init(); // Initialize belief propagation algorithm
    bp.run(); // Run belief propagation algorithm
#ifdef DEBUG
    for(size_t i = 0;i < factorGraph.nrVars(); i++){ // The first three should correspond to movies, I think
        std::cout << bp.belief(factorGraph.var(i)) << std::endl;
    }
#endif

    /* MAKE RECOMMENDATION */
    float beliefs[input->numMovies];
    int orderedMovies[input->numMovies];
    for(int i=0; i<input->numMovies; i++){
        beliefs[i] = bp.belief(factorGraph.var(i))[1];
        orderedMovies[i] = i;
    }
    // Bubble sort
    for (size_t i=0; i<input->numMovies-1; i++){
        for (size_t j=0; j<input->numMovies-i-1; j++){
            if (beliefs[j] < beliefs[j + 1]){
                int temp = orderedMovies[j];
                orderedMovies[j] = orderedMovies[j+1];
                orderedMovies[j+1] = temp;
            }
        }
    }
    int* recommendations = (int*)malloc(k * sizeof(int));
    for(size_t i=0; i<input->numMovies; i++){
        recommendations[i] = orderedMovies[i]+1;
    }
    return recommendations;
}

#endif