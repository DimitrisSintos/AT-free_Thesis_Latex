from utilities import *
from itertools import combinations
from pyvis.network import Network


class Graph:
    show_count = 0 # Class-level variable to keep track of show calls

    def __init__(self, num_of_vertices, num_of_edges,edges,vertices = None):
        self.num_of_vertices = num_of_vertices
        self.num_of_edges = num_of_edges
        self.vertices = set(str(i) for i in range(num_of_vertices)) if vertices is None else vertices
        self.edges = set((str(u),str(v)) for u,v in edges)       
        self.adjacency_list = { str(vertex): set() for vertex in self.vertices}
        for edge in self.edges:
            u, v = edge
            self.adjacency_list[u].add(v)
            self.adjacency_list[v].add(u)

        self.blocks = {}
        self.cutpoints = set()
        self.vertices_color = None


    def is_triangle(self):
        if self.num_of_vertices == 3 or self.num_of_edges == 3:
            return True
        else:
            return False

    def find_K4(self):
        """
        The function `find_K4` checks if a graph contains a complete graph K4.
        :return: a boolean value. It returns True if a K4 subgraph is found in the given graph, and False
        otherwise.
        """
        for edge in self.edges:
            u, v = edge
            disjoint_edges = set( (x,y) for (x,y) in self.edges if x not in (u,v) and y not in (u,v))
            for across_edge in disjoint_edges:
                x, y = across_edge
                edges_found = 0 
                if ( ( (x,u) in self.edges or (u,x) in self.edges )
                     and ( (y,v) in self.edges or (v,y) in self.edges ) 
                       and ( (x,v) in self.edges or (v,x) in self.edges ) 
                        and ( (y,u) in self.edges or (u,y) in self.edges ) ):
                    return True
        return False
                


    def find_triangle_in_neighborhood(self, contracted_vertex):
        """
        Test if the neighborhood of contracted_vertex in graph contains a triangle. 
        This function is designed to be called when Line 1 is reached via recursion.
        This function should run in time O(n*m)

        :param graph: The graph instance
        :param contracted_vertex: The contracted vertex in the graph
        :return: True if the neighborhood of s contains a triangle, otherwise False
        """
        neighbors = self.adjacency_list[contracted_vertex]
        if len(neighbors) < 3:
            return False
        else:
            for sub_vertices in combinations(neighbors, 3):
                if all((u, v) in self.edges or (v, u) in self.edges for u, v in combinations(sub_vertices, 2)):
                    return True 

        return False
    
    def find_diamond(self):
        """
        This function should run in time O(n*m)
        The function "find_diamond" searches for a diamond pattern in a graph by finding two nodes that
        have at least two common neighbors.
        :return: a set of common neighbors between two vertices in a graph. If there are at least two
        common neighbors, the function returns the set of common neighbors. If there are no common
        neighbors or less than two common neighbors, the function returns None.
        """
        for edge in self.edges:
            u, v = edge
            adj_u = self.adjacency_list[u]

            adj_v = self.adjacency_list[v]

            common_neighbors = adj_u & adj_v

            if len(common_neighbors) >= 2:
                return common_neighbors
        return None
    
    def contract(self, vertices_to_contract):
        """
        Contract a set of vertices into a single vertex.
        
        :param vertices_to_contract: set of vertices to contract
        :return: A new Graph instance with the vertices contracted.
        """

        #All vertices to contract will be replaced by a new vertex
        new_vertex = rename_vertices_to_contract(vertices_to_contract)
        new_vertices = [v for v in self.vertices if v not in vertices_to_contract] + [new_vertex]

        # All edges incident to a vertex in vertices_to_contract will now be incident to new_vertex
        new_edges = []
        for u, v in self.edges:
            if u in vertices_to_contract and v in vertices_to_contract:
                # Ignore edges within the contracted set
                continue
            elif u in vertices_to_contract:
                new_edges.append((new_vertex, v))
            elif v in vertices_to_contract:
                new_edges.append((u, new_vertex))
            else:
                new_edges.append((u, v))


        #(self, num_of_vertices, num_of_edges,edges,vertices = None)
        return Graph(len(new_vertices), len(new_edges), new_edges, new_vertices)
    
    
    def delete_vertices(self, vertices_to_delete):
        """
        Delete a set of vertices from the graph.
        
        :param vertices_to_delete: set of vertices to delete
        :return: A new Graph instance with the vertices deleted.
        """
        new_vertices = set(self.vertices - vertices_to_delete)
        print('new_vertices:',new_vertices)
        new_edges = set(e for e in self.edges if e[0] not in vertices_to_delete and e[1] not in vertices_to_delete)#TODO
        print("new_edges:",new_edges)
        return Graph(len(new_vertices), len(new_edges), new_edges, new_vertices)

    
    def edge_exists(self, u, v):
        return (u, v) in self.edges or (v, u) in self.edges
    
    
    def copy(self):
        return Graph(self.num_of_vertices, self.num_of_edges, self.edges, self.vertices)
    

    def show(self,graph_name='graph'):
        Graph.show_count += 1
        print("Showing graph:",Graph.show_count)
        net = Network(height="500px", width="100%", bgcolor="#222222", font_color="white")

        for vertex in self.vertices:
            # If vertex colors are provided, use them; otherwise, default to None
            color = self.vertices_color.get(vertex) if isinstance(self.vertices_color, dict) else None
            net.add_node(vertex, color=color)


        for edge in self.edges:
            u, v = edge
            net.add_edge(u, v, color="white")

        file_name = f"../output-graphs/{graph_name}-{Graph.show_count}.html"
        net.show(file_name)








