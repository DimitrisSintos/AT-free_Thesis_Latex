from itertools import combinations
from pyvis.network import Network


class Graph:
    show_count = 0  # Class-level variable to keep track of show calls

    def __init__(self, num_of_vertices, num_of_edges, edges, vertices=None):
        self.num_of_vertices = num_of_vertices
        self.num_of_edges = num_of_edges
        self.vertices = set(str(i) for i in range(num_of_vertices)) if vertices is None else vertices
        self.edges = set((str(u), str(v)) for u, v in edges)
        self.adjacency_list = {str(vertex): set() for vertex in self.vertices}
        for edge in self.edges:
            u, v = edge
            self.adjacency_list[u].add(v)
            self.adjacency_list[v].add(u)
            
        domination_set = set() 
        
        
            
    def bfs_levels(self, source):
        source = str(source)

        visited = {vertex: False for vertex in self.vertices}
        level = {vertex: None for vertex in self.vertices}
        queue = []

        visited[source] = True
        level[source] = 0
        queue.append(source)

        levels = {}

        while queue:
            vertex = queue.pop(0)
            current_level = level[vertex]

            if current_level not in levels:
                levels[current_level] = set()
            levels[current_level].add(vertex)

            for neighbor in self.adjacency_list[vertex]:
                if not visited[neighbor]:
                    queue.append(neighbor)
                    visited[neighbor] = True
                    level[neighbor] = current_level + 1

        return levels
    
    def closed_neighborhood(self, vertex):
        return {vertex}.union(self.adjacency_list[vertex])
    
    def closed_neighborhood_of_set(self, vertex_set):
        closed_neighborhood = set()
        for vertex in vertex_set:
            vertex_str = str(vertex)
            closed_neighborhood = closed_neighborhood.union(self.closed_neighborhood(vertex_str))
        return closed_neighborhood
    

    def show(self, graph_name='graph'):
        Graph.show_count += 1
        print("Showing graph:", Graph.show_count)
        net = Network(height="500px", width="100%", bgcolor="#222222", font_color="white")

        for vertex in self.vertices:
            if vertex in self.domination_set:
                net.add_node(vertex, color="red")
            else:
                net.add_node(vertex)
            

        for edge in self.edges:
            u, v = edge
            net.add_edge(u, v, color="white")

        file_name = f"../output-graphs/{graph_name}-{Graph.show_count}.html"
        net.show(file_name)
