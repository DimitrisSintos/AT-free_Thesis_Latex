from utilities import *
from graph import Graph
from tarjans_biconnectivity import TarjansBiconnectivity
from three_colouring import ThreeColouring
from itertools import combinations

class PolynomialTimeAlgorithm:
    def __init__(self,graph: Graph):
        self.graph = graph
        self.is_recursive_call = False
        self.init_graph = graph.copy()
        self.contracted_vertex = None

    def three_colouring(self):
        try:
            line_1_result = self.line_1_check()
            if line_1_result:
                print("Line 1 check failed. There is a K4 in the graph so it is not 3-colourable.")
                return False

            line_3_condition, vertices_to_contract = self.line_3_check()
            if line_3_condition:
                self.perform_contraction(vertices_to_contract)
                return self.three_colouring()

            line_5_condition,minimal_stable_separator = self.line_5_check()
            if line_5_condition:
                self.perform_contraction(minimal_stable_separator)
                return self.three_colouring()

            colouring = ThreeColouring(self.graph,self.init_graph)
            
            return colouring.construct_three_colouring()
        except Exception as e:
            print(f"An exception occurred: {e}")
            raise

    def line_1_check(self):
        if self.is_recursive_call:
            return self.graph.find_triangle_in_neighborhood(self.contracted_vertex)
        else:
            return self.graph.find_K4()

    def line_3_check(self):
        result = self.graph.find_diamond()
        if result is None:
            return False , None
        else:
            return True , result


    def line_5_check(self):
        """
        this should run in O(n*m) time
        if G contains a minimal stable separator S with |S| ≥ 2 then
            Recursively find a 3-colouring of G/S
        """
        # Find the biconnected components of the graph
        biconnectivity_algorithm = TarjansBiconnectivity(self.graph)
        blocks, self.graph.cutpoints = biconnectivity_algorithm.find_biconnected_components()

        print("cutpoints:",self.graph.cutpoints)

        # delete old blocks from graph and add the new blocks
        self.graph.blocks = {}

        for i, block in enumerate(blocks):# i represents the block id
            self.graph.blocks[i] = block


        for vertex in self.graph.vertices:
            for block_id in self.graph.blocks:
               
                block = self.graph.blocks[block_id]
                if vertex in block.vertices and len(block.vertices) >= 3:
                    cond , minimal_stable_separator = block.find_minimal_stable_separator(vertex)
                    if cond:
                        return True,  minimal_stable_separator

        return False, None

    def perform_contraction(self, vertices_to_contract):
        self.graph = self.graph.contract(vertices_to_contract)
        self.contracted_vertex = rename_vertices_to_contract(vertices_to_contract)
        self.is_recursive_call = True
        self.graph.show("contracted-graph")
    
    
    def run(self):
        return self.three_colouring()
