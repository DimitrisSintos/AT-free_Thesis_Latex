from itertools import combinations
from pyvis.network import Network
from component import Component
from interval import Interval

import os

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
            
        self.components = {}
        self.num_of_components = {}
        self.intervals = {}
        self.non_adjacent_vertices_component_pointer = {}
        
        self.independent_set = set()
            
    
    def compute_all_components(self):
        for vertex in self.vertices:
            vertex_components = self.compute_components_of_vertex(vertex)
            self.num_of_components[vertex] = len(vertex_components)
            for i in range(len(vertex_components)):
                self.components[(vertex, i)] = Component(vertex, i, vertex_components[i])
    
    def compute_components_of_vertex(self,vertex):
        components_vertices = self.vertices - self.closed_neighborhood(vertex)
        components = []
        visited = set()
        
        def dfs(node,component):
            visited.add(node)
            component.add(node)
            self.non_adjacent_vertices_component_pointer[(vertex, node)] = (vertex, len(components))
            for neighbor in self.adjacency_list[node]:
                if neighbor not in visited and neighbor in components_vertices:
                    dfs(neighbor, component)
        
        for v in components_vertices:
            if v not in visited:
                component = set()
                dfs(v, component)
                components.append(component)
                      
        return components   
    
    def closed_neighborhood(self, vertex):
        return {vertex}.union(self.adjacency_list[vertex])
    
    def closed_neighborhood_of_set(self, vertex_set):
        closed_neighborhood = set()
        for vertex in vertex_set:
            closed_neighborhood = closed_neighborhood.union(self.closed_neighborhood(vertex))
        return closed_neighborhood
    
    def compute_all_intervals(self):
        for x in self.vertices:
            for y in self.vertices:
                if x != y and y not in self.adjacency_list[x]:
                    interval = self.compute_interval(x, y)
                    if interval:
                        self.intervals[(x, y)] = Interval(x, y, interval)
        return self.intervals
    
    
    def compute_interval(self, x, y):
        if y in self.adjacency_list[x]:
            raise ValueError("Vertices x and y are adjacent. Interval can only be computed for nonadjacent vertices.")

        Cx_y = None
        Cy_x = None
        
        Cx_y_pointer = self.non_adjacent_vertices_component_pointer[(x, y)]
        Cy_x_pointer = self.non_adjacent_vertices_component_pointer[(y, x)]
        
        Cx_y = self.components[Cx_y_pointer].vertices 
        Cy_x = self.components[Cy_x_pointer].vertices 
        
        interval = Cx_y.intersection(Cy_x) if Cx_y is not None and Cy_x is not None else None
        return interval
    
    def show(self, graph_name='graph'):
        Graph.show_count += 1
        print("Showing graph:", Graph.show_count)
        net = Network(height="500px", width="100%", bgcolor="#222222", font_color="white")

        for vertex in self.vertices:
            if vertex in self.independent_set:
                net.add_node(vertex, color="red")
            else:
                net.add_node(vertex)
            

        for edge in self.edges:
            u, v = edge
            net.add_edge(u, v, color="white")

        script_dir = os.path.dirname(os.path.abspath(__file__))
        output_dir = os.path.join(script_dir, '..', 'output-graphs')
        file_name = os.path.join(output_dir, f"{graph_name}-{Graph.show_count}.html")
        net.show(file_name)

