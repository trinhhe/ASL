#include <stdio.h>
#include <stdlib.h>
#include <dai/alldai.h>
#include <dai/factorgraph.h>
#include <dai/bp.h>
#include <iostream>
#include "../../measurement_utils/tsc_x86.h"
#include "recommend.h"

#define REP 10

using namespace std;
using namespace dai;

const int max_iter = 10;
// First build the library yourself as described in the README in the libdai folder
// Compile with: make
// Examples used: example_sprinkler.cpp and example.cpp
int main(int argc, char *argv[]) {

    const char* fileLocation = argc > 1 ? argv[1] : "../test/data/ratings_small.csv";

    /* READ IN DATA */
    rinput_t ri;
    initialize(&ri,fileLocation,1);

    myInt64 start, stop;
    double total = 0;
    // int cnt = 0;
    FactorGraph fgraphclone;
    BP bpclone;
    //vector<size_t> var_states; //needed to temporarly save states of variables in factorgraph needed for flop counting, for some reason after when factorgraph gets deconstructed, the fclonegraph variable states memory get freed
    double ratedByTargetUser[ri.numMovies] = {-1};
    vector<Factor> factors;
    for (int j = 0; j < REP; j++) {
        start = start_tsc();
        /* DEFINE FACTOR GRAPH */
        // States x_i ∈ {LIKE,DISLIKE}
        // Factors psi_i*phi_ij (https://en.wikipedia.org/wiki/Belief_propagation#Description_of_the_sum-product_algorithm)

        // Calculate edge threshold & collect target user ratings
        int numRatingsPerUser[ri.numUsers] = {0};
        double averageRatingPerUser[ri.numUsers] = {0}; // for edge thresholding
        double ratingsTargetUser[ri.numRatingsTargetUser] = {0};
        // double ratedByTargetUser[numMovies] = {-1};
        int idxRatingsTargetUser = 0;
        for(size_t i=0; i<ri.numRatings; i++){
            numRatingsPerUser[ri.users[i]]++;
            averageRatingPerUser[ri.users[i]]+=ri.ratings[i];
            if(ri.users[i] == ri.targetUser){
                ratingsTargetUser[idxRatingsTargetUser++] = ri.ratings[i];
                ratedByTargetUser[ri.movies[i]] = ri.ratings[i];
            }
        }
        for(size_t i=0; i<ri.numUsers; i++){
            averageRatingPerUser[i] /= numRatingsPerUser[i];
        }

        // Preparation for calculating node potentials
        double sampleVariance = 0; // Not sure
        for(size_t i=0; i<ri.numRatingsTargetUser; i++){
            double diff = ratingsTargetUser[i] - ri.avgMovieRatingTargetUser;
            sampleVariance += diff*diff;
        }
        sampleVariance /= (ri.numRatingsTargetUser); // Not sure   ST: removed -1
        double standardDerivation = sqrt(sampleVariance);

        // Initialize factor variables for term psi_i, phi_ij for all nodes
        // vector<Factor> factors;
        factors.clear();
        for(size_t i=0; i<ri.numUsers; i++){
            Var user(ri.numMovies + i, 2);
            Factor m(user);
            m.set(0, 0.5);
            m.set(1, 0.5);
            factors.push_back(m);
        }
        for(size_t i=0; i<ri.numMovies; i++){
            Var movie(i, 2);
            Factor m(movie);
            double dislikePotential = 0.5;
            double likePotential = 0.5;
            if(ratedByTargetUser[i]!=-1){
                double zscore = (ratedByTargetUser[i] - ri.avgMovieRatingTargetUser)/standardDerivation;
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
        for(size_t i=0; i<ri.numRatings; i++) {
            if(ri.ratings[i]>=averageRatingPerUser[ri.users[i]]) {
                Var user(ri.numMovies + ri.users[i], 2);
                Var movie(ri.movies[i], 2);
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
        opts.set("maxiter", (size_t)max_iter); // Maximum number of iterations
        opts.set("tol",1e-40); // Tolerance for convergence
        opts.set("verbose",(size_t)0); // Verbosity (amount of output generated)
        BP bp(factorGraph, opts("updates",string("SEQFIX"))("logdomain",false)("inference",string("SUMPROD"))); // TODO: SEQFIX?
        bp.init(); // Initialize belief propagation algorithm
        bp.run(); // Run belief propagation algorithm

        for(size_t i = 0;i < factorGraph.nrVars(); i++){ // The first three should correspond to movies, I think
            bp.belief(factorGraph.var(i));
        }
        stop = stop_tsc(start);
        total += (double) stop;
        if (j == REP-1) {
            fgraphclone = factorGraph;
            bpclone = bp;
            size_t n = factorGraph.nrFactors();
            //var_states.reserve(n);
            //for( size_t k = 0; k < n; ++k) //DELETE
            //    var_states[k] = factorGraph.var(k).states();
        }
    }
    total /= REP;

    // count flops
    unsigned int flops = 0;
    unsigned int iterations = bpclone.Iterations();

    flops += 10 * ri.numRatingsTargetUser + 3; //node potentials
    flops += ri.numRatings + ri.numUsers; //edge threshold
    flops += 2; //propagation matrix


    //bp2.run flops
    size_t nrEdges = fgraphclone.nrEdges();
    size_t nrVars = fgraphclone.nrVars();
    size_t nrFactors = fgraphclone.nrFactors();
    std::vector<Edge> _updateSeq;
    _updateSeq.reserve( nrEdges );
    for( size_t I = 0; I < nrFactors; I++)
        bforeach( const Neighbor &i, fgraphclone.nbF(I) ) {
            _updateSeq.push_back( Edge( i, i.dual ) );
        }

    bforeach( const Edge &e, _updateSeq ) {
        //calcNewMessage (bp.cpp:211)
        size_t i = e.first;
        size_t I = fgraphclone.nbV(i,e.second);
        if (fgraphclone.factor(I).vars().size() != 1) {
            // calcIncomingMessageProduct (bp:cpp:168)
            Factor Fprod( fgraphclone.factor(I) );
            Prob &prod = Fprod.p();
            bforeach( const Neighbor &j, fgraphclone.nbF(I) )
                if (j != i) {
                bforeach( const Neighbor &J, fgraphclone.nbV(j) )
                    if( J != I )
                        flops++;
                // multiply prod with prod_j (bp.cpp:200)
                flops += prod.size();      
                }    
            //marginalize in calcNewMessage (bp.cpp:228)
            flops += prod.size(); 
            flops += fgraphclone.var(i).states();//normalize (bp.cpp:247)
        }
    }

    //calculate beliefs (note: they calculate after every message passing iteration the belief states to look
    //up difference between old_belief state and new belief state. If difference < tolerance, algorithm converges)
    
    //believe for Variables (bp.cpp:325)
    for( size_t i = 0; i < nrVars; ++i) {
        bforeach( const Neighbor &I, fgraphclone.nbV(i) )
            flops++;
        flops += 3*fgraphclone.var(i).states(); //normalize + dist that contains dot product between probability vectors
    }
    //believe for Factors (bp.cpp:330) (contains calcIncomingMessageProduct)
    for( size_t k = 0; k < nrFactors; ++k) {        
        //calcIncomingMessageProduct (bp:cpp:168)
        Factor Fprod( fgraphclone.factor(k) );
        Prob &prod = Fprod.p();
        bforeach( const Neighbor &j, fgraphclone.nbF(k) ) {
            bforeach( const Neighbor &J, fgraphclone.nbV(j) )
                if( J != k )
                    flops++;
            // multiply prod with prod_j (bp.cpp:200)
            flops += prod.size();      
        }
        //normalize (bp.cpp:391)
        flops += prod.size();
        flops += 2*prod.size(); //dist (bp.cpp:332)
    }
    flops *= iterations;
    printf("%zu, %f, %u\n", nrVars, total, flops);

    return 0;
}
