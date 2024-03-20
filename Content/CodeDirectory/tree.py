from graph import Graph
from pyvis.network import Network

class BlockCutpointTree:
    current_block = None 

    def __init__(self, blocks, cutpoints):
        self.blocks = blocks
        self.cutpoints = cutpoints
        self.root = None
        self.tree = self.create_tree()
        self.visited = set()

    def create_tree(self):
        """
        For every block of the graph, create a node in the tree.
        If two blocks(nodes) contains the same cutpoint, then we connect them with an edge.
       """
        
        vertices = set()
        edges = set()
        for block_id in self.blocks:
            print("type(block_id)",type(block_id))
            vertices.add(block_id)

        for i in range(len(self.blocks)):
            for j in range(i+1,len(self.blocks)):
                for cutpoint in self.cutpoints:
                    if cutpoint in self.blocks[i].vertices and cutpoint in self.blocks[j].vertices:
                        print("(i,j)",(i,j))
                        edges.add((i,j))

        #vertices to string 
        vertices = set(str(v) for v in vertices)
    
        return Graph(len(vertices),len(edges),edges, vertices)

    def get_next_block(self):
        if self.current_block is None:
            # Find the first unvisited node to start
            for node in self.tree.vertices:
                if node not in self.visited:
                    self.current_block = node
                    break
            else:
                # No unvisited nodes left
                return None

        stack = [self.current_block]
        while stack:
            node = stack[-1]
            if node not in self.visited:
                self.visited.add(node)
                self.current_block = node
                return int(node)

            # Explore neighbors
            unvisited_neighbors = [neighbor for neighbor in self.tree.adjacency_list[str(node)] if neighbor not in self.visited]
            if unvisited_neighbors:
                stack.extend(unvisited_neighbors)
            else:
                stack.pop()  # Backtrack

            # If the stack is empty and there are still unvisited nodes, continue from an unvisited node
            if not stack:
                for next_node in self.tree.vertices:
                    if next_node not in self.visited:
                        stack.append(next_node)
                        break

        # If all nodes are visited, return None
        return None
            
        
    def show(self):
        """
        Show the tree.
        """
        self.tree.show("block_cutpoint_tree")
    
        
            
          
