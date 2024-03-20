from graph import Graph
from itertools import combinations



class PolynomialTimeAlgorithm:
    def __init__(self,graph : Graph, weight : int):
        self.graph = graph
        self.weight = weight



    def mcdsw(self):
        """
        Returns a minimum cardinality dominating set of the graph.
        """
        D = self.graph.vertices
        for vertex in self.graph.vertices:
            H = self.graph.bfs_levels(vertex)
            l = len(H) - 1 
            i = 1
            
            print("\nBFS Levels of:",vertex,":\n" ,H)
            
            #Initialize the queue A1 to contain an ordred triple (S, S, val_S) 
            #for all nonempty subets S of N[vertex] satisfying val_S:= |S| ≤ w
            A1 = self.initialize_queue(vertex)
            print("Init queue of:",vertex,":\n" ,A1)
            A = { i : A1}

            while A[i] and i < l:
                i = i + 1
                A[i] = []
                
                for triple in A[i-1]:
                    print(f"Triple in the queue A[{i-1}]:", triple)
                    S = triple[0]
                    S_accent = triple[1]
                    val_S_accent = triple[2]
                    #For all U ⊆ H[i]   such that   |S ∪ U| ≤ w
                    for r in range(1, len(H[i]) + 1):
                        for U in combinations(H[i], r):
                            U = set(U)
                            if len(S | U) <= self.weight:
                                closed_neighborhood_of_S_and_U = self.graph.closed_neighborhood_of_set(S | U)
                                if closed_neighborhood_of_S_and_U.issuperset(H[i-1]):
                                    R = ( S | U ) - H[i-2]
                                    R_accent = S_accent | U
                                    val_R_accent = val_S_accent + len(U)
                                    print("R:", (R, R_accent, val_R_accent))
                                    #IF there is no triple in Ai with first entry R
                                    #THEN inser (R; R_accent ; val_R_accent) into Ai ;
                                    if not any(triple[0] == R for triple in A[i]):
                                        print(f"R is not in A[{i}] so we insert it into A[{i}")
                                        A[i].append((R, R_accent, val_R_accent))
                                    #IF there is a triple (P; P_accent ; val_P_accent) in Ai such that P = R AND val_R_accent < val_P_accent
                                    #THEN replace (P; P_accent ; val_P_accent) by (R; R_accent ; val_R_accent) in Ai ;
                                    for index, triple in enumerate(A[i]):
                                        if triple[0] == R and val_R_accent < triple[2]:
                                            print(f"R is in A[{i}] and val_R_accent < val_P_accent so we replace it in A[{i}]")
                                            A[i][index] = (R, R_accent, val_R_accent)
                                            
                                
            print("\nA[l] after while loop:", A[l])
            # Find the triple with minimum val(S') in A[l] that satisfies H[l] ⊆ N[S]
            optimal_triple = min((triple for triple in A[l] if set(H[l]).issubset(self.graph.closed_neighborhood_of_set(triple[0]))), 
                                key=lambda x: x[2], default=None)
            
            print("\noptimal_triple:", optimal_triple)
            
            if optimal_triple and optimal_triple[2] < len(D):
                D = optimal_triple[1]  # Update D if a better solution is found
                print("########\nD is updated to:", D)
                print("########")
        
        print("THE DOMINATION SET IS:", D)
        self.graph.domination_set = D
        self.graph.show()
                            
        return 
    
    def initialize_queue(self, vertex):
        queue = []
        for subset in self.generate_subsets(vertex):
            queue.append((subset, subset, len(subset)))
        return queue
    
    def generate_subsets(self, vertex):
        closed_nbhd = self.graph.closed_neighborhood(vertex)
        subsets = []
        for r in range(1, self.weight + 1):
            for subset in combinations(closed_nbhd, r):
                subsets.append(set(subset))
        return subsets

    
    
    def run(self):
        return self.mcdsw()