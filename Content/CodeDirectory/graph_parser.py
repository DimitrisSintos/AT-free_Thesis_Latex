import networkx as nx
import json
from graph import Graph
from itertools import combinations


class GraphParser:
    @staticmethod
    def parse_graph_from_file(file_path):
        with open(file_path, 'r') as file:
            data = json.load(file)

        num_of_vertices = data['num_of_vertices']
        num_of_edges = data['num_of_edges']
        edges = [tuple(edge) for edge in data['edges']]  # Convert edge lists to tuples

        # Verify if we got the correct number of edges
        if len(edges) != num_of_edges:
            raise ValueError("The number of edges specified does not match the number of edges parsed.")

        if GraphParser.has_asteroidal_triple(num_of_vertices, edges):
            raise ValueError("The graph is no AT-free.")
        else:
            return Graph(num_of_vertices, num_of_edges, edges)

    @staticmethod
    def has_asteroidal_triple(num_of_vertices, edges):
        # Create a graph
        G = nx.Graph()
        G.add_nodes_from(range(num_of_vertices))
        G.add_edges_from(edges)

        # Generate all possible combinations of 3 vertices
        vertex_combinations = combinations(range(num_of_vertices), 3)

        # Check for each combination
        for v1, v2, v3 in vertex_combinations:
            if GraphParser.has_path_avoiding_neighbors(G, v1, v2, v3) \
                    and GraphParser.has_path_avoiding_neighbors(G, v2,v3,v1) \
                        and GraphParser.has_path_avoiding_neighbors(G, v3, v1, v2):
                print("Found an asteroidal triple: ", v1, v2, v3)
                return True
        return False

    @staticmethod
    def has_path_avoiding_neighbors(G, source, target, avoid):
        """
        Check if there is a path from source to target in graph G
        avoiding neighbors of avoid.
        """
        neighbors_to_avoid = set(G.neighbors(avoid))
        visited = set()
        queue = [source]

        while queue:
            node = queue.pop(0)
            visited.add(node)
            if node == target:
                return True
            for neighbor in G.neighbors(node):
                if neighbor != avoid and neighbor not in visited and neighbor not in queue and neighbor not in neighbors_to_avoid:
                    queue.append(neighbor)

        return False
