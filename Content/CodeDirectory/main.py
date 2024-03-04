from graph import Graph
from polynomial_time_algorithm import PolynomialTimeAlgorithm
from graph_parser import GraphParser
from brute_force import BruteForce
import sys


def main(argv):
    file_path = argv[0]
    at_free_graph = GraphParser.parse_graph_from_file(file_path)
        
    algorithm = PolynomialTimeAlgorithm(at_free_graph)
    algorithm.run()
    at_free_graph.show()
    
    brute_force = BruteForce(at_free_graph)
    print("Brute force: " + str(brute_force.run()))


if __name__ == "__main__":
    main(argv=sys.argv[1:])
