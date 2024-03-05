class Component:
    def __init__(self, x, i, vertices):
        self.x = x
        self.i = i
        self.vertices = vertices
        self.alpha = None
        
    def __repr__(self):
        return f"Component(x={self.x}, i={self.i}, vertices={self.vertices}, alpha={self.alpha})"
    
    def __lt__(self, other):
        return len(self.vertices) < len(other.vertices)
    
    def __len__(self):
        return len(self.vertices)