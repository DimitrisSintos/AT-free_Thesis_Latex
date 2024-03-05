class Interval:
    def __init__(self, x, y, vertices):
        self.x = x
        self.y = y
        self.vertices = vertices
        self.alpha = None
        
    def __repr__(self):
        return f"Interval(x={self.x}, y={self.y}, vertices={self.vertices}, alpha={self.alpha})"
    
    def __lt__(self, other):
        return len(self.vertices) < len(other.vertices)
    
    def __len__(self):
        return len(self.vertices)
    
    