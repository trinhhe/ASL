import networkx as nx
import sys
import os
import numpy as np
from scipy.stats import truncnorm

OUT = "data_compl_bipartite"

OUT += "/"

def main():
    # if len(sys.argv) < 3:
    #     print(f"Usage: n1, n2 (n1 nodes/users in set1, n2 nodes/movies in set2)")
    #     sys.exit(1)
    # n1, n2 = int(sys.argv[1]), int(sys.argv[2])

    lists = [(10,1000), (20,2000), (30,3000), (40,4000), (50,5000), (60,6000), (70,7000),(80,8000), (90,9000),(100,10000)]
    for l in lists:
        n1, n2 = l
        G = nx.bipartite.complete_bipartite_graph(n1, n2)
        scale = 1
        range = 2
        dist = truncnorm(a=-range/scale, b=+range/scale, scale=scale) #get ratings from 1-5 that behave similar to normal distribution
        
        if not os.path.exists(OUT):
            os.mkdir(OUT)
        
        out = []
        rnd = dist.rvs(size=len(G.edges)).round().astype(int) + 3

        fn = f"{OUT}%03d_%05d_norm.csv" % (n1, n2) # I put norm in filename for measure_runtime.sh script
        with open(fn, 'w') as file:
            file.write("userId,movieId,rating,timestamp\n")
            for i, e in enumerate(G.edges):
                rating = rnd[i]
                file.write(f"{e[0] + 1}, {e[1]-n1+1}, {float(rating)}, 123456789\n")
    
if __name__ == "__main__":
    main()
