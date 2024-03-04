from graph import Graph
from bucket_sort import BucketSort

from itertools import combinations



class PolynomialTimeAlgorithm:
    def __init__(self,graph : Graph):
        self.graph = graph

        self.max_interval_vertices_set = {}
        self.max_component_vertices_set = {}




    def computing_independent_set(self):
        # Step 1. For every x ∈ V compute all components C1x , C2x , . . . , Cr(x)
        # Step 2. For every pair of nonadjacent vertices x and y compute the interval I(x, y).
        # Step 3. Sort all the components and intervals according to nondecreasing number of
        # vertices.
        # Step 4. Compute α(C) and α(I) for each component C and each interval I in the
        # order of Step 3.
        # Step 5. Compute α(G).

        # Step 1
        self.graph.compute_all_components()
        print("\nComponents:")
        for component in self.graph.components.values():
            print(component)
            
        print("self.graph.num_of_components:", self.graph.num_of_components)
            
        
        
        # Step 2
        self.graph.compute_all_intervals()
        print("\nIntervals:")
        for interval in self.graph.intervals.values():
            print(interval)
                
        # Step 3

        # Sort components
        bucket_sort_components = BucketSort(list(self.graph.components.keys()), key_function=lambda x: len(self.graph.components[x]))
        sorted_components = bucket_sort_components.sort()

        # Sort intervals
        bucket_sort_intervals = BucketSort(list(self.graph.intervals.keys()), key_function=lambda x: len(self.graph.intervals[x]))
        sorted_intervals = bucket_sort_intervals.sort()

        print("\nSorted components:", sorted_components, len(sorted_components), type(sorted_components))
        print("\nSorted intervals:", sorted_intervals, len(sorted_intervals), type(sorted_intervals))
        # Step 4
        for key in sorted_components:
            self.alpha_C(key)
            
        # Step 5
        print('\n\n\n')
        print("Graph intependent set number:", self.alpha_G())
            
            
    def alpha_C(self,component_key):
        component = self.graph.components[component_key]
        print(component)
        if component.alpha is not None:
            return component.alpha
        
        x = component.x
        
        component_vertices = component.vertices

        max_alpha = 0
        self.max_component_vertices_set[component_key] = set()
        max_y = None
        max_D_y_components_vertices = set()
        for y in component_vertices: # y E Cx
            alpha_I_xy = self.alpha_I((x, y))

            D_y_components = self.compute_D_iy(y, component_vertices)
            
            alpha_D_sum = sum(self.alpha_C(D_iy) for D_iy in D_y_components)
            if max_alpha <= alpha_I_xy + alpha_D_sum:
                max_alpha = alpha_I_xy + alpha_D_sum
                max_y = y
                max_D_y_components_vertices = set()
                for D_iy in D_y_components:
                    max_D_y_components_vertices = max_D_y_components_vertices.union(self.max_component_vertices_set[D_iy])

        
        alpha_value = 1 + max_alpha
        
        self.max_component_vertices_set[component_key] = max_D_y_components_vertices.union({max_y})
        if (x,max_y) in self.graph.intervals:
            self.max_component_vertices_set[component_key] = self.max_component_vertices_set[component_key].union(self.max_interval_vertices_set[(x,max_y)])
        print(f"self.max_component_vertices_set[{component_key}]:{self.max_component_vertices_set[component_key]}")
        
        component.alpha = alpha_value
        
    
        print(f'Computed Component,{component_key} Alpha: {self.graph.components[component_key]}')
        
        return alpha_value
    
    def alpha_I(self, I):
        
        try:
            interval = self.graph.intervals[I]
            print(interval)
            if interval.alpha is not None:
                print(f"Interval ({I}) is already computed with alpha: {interval.alpha}")
                return interval.alpha
            
            x = interval.x
            y = interval.y
            I_vertices = interval.vertices

            max_alpha = 0
            self.max_interval_vertices_set[I] = set()
            max_s = None
            max_C_s_components_vertices = set()
            for s in I_vertices: 
                alpha_I_xs = self.alpha_I((x, s)) 
                alpha_I_sy = self.alpha_I((s, y))
                C_s_components = self.compute_C_is(s, I_vertices)
                alpha_C_sum = sum(self.alpha_C(C_is) for C_is in C_s_components)
                # max_alpha = max(max_alpha, alpha_I_xs + alpha_I_sy + alpha_C_sum)
                if max_alpha <= alpha_I_xs + alpha_I_sy + alpha_C_sum:
                    max_alpha = alpha_I_xs + alpha_I_sy + alpha_C_sum
                    max_s = s
                    max_C_s_components_vertices = set()
                    for C_is in C_s_components:
                        max_C_s_components_vertices = max_C_s_components_vertices.union(self.max_component_vertices_set[C_is])
                    
           
            alpha_value = 1 + max_alpha
            print("interval alpha:", alpha_value)

            print("max_C_s_components_vertices", max_C_s_components_vertices)
            interval_x_s_vertices = self.graph.intervals[(x, max_s)].vertices if (x, max_s) in self.graph.intervals else set()
            print("interval_x_s_vertices:", interval_x_s_vertices)
            interval_s_y_vertices = self.graph.intervals[(max_s, y)].vertices if (max_s, y) in self.graph.intervals else set()
            print("interval_s_y_vertices:", interval_s_y_vertices)
            self.max_interval_vertices_set[I] = interval_x_s_vertices.union(interval_s_y_vertices).union(max_C_s_components_vertices).union({max_s})
            print(f"self.max_interval_vertices_set[{I}]: {self.max_interval_vertices_set[I]}")
            
            interval.alpha = alpha_value
            
            
            print(f'Computed Interval({I}) alpha:{self.graph.intervals[I]}')
            
            return alpha_value
        except:
            print("Interval", I, "not found")
            return 0

    def compute_components_subset(self, vertex, target_vertices, num_components):
        """
        Computes the components of G - N[vertex] contained in the target set.
        
        :param vertex: The vertex whose neighborhood defines the components.
        :param target_vertices: The set of vertices that the component should be a subset of.
        :param num_components: The number of components to consider.
        :return: A list of component keys whose vertices are a subset of the target set.
        """
        computed_components = []
        
        for i in range(num_components):
            component_key = (vertex, i)
            component = self.graph.components[component_key]
            if component.vertices.issubset(target_vertices):
                computed_components.append(component_key)
                    
        return computed_components

    def compute_D_iy(self, y, Cx_vertices):
        """
        D_iy are the components of G - N[y] contained in Cx.
        """
        return self.compute_components_subset(y, Cx_vertices, self.graph.num_of_components[y])

    def compute_C_is(self, s, I_vertices):
        """
        C_is are the components of G - N[s] contained in I.
        """
        return self.compute_components_subset(s, I_vertices, self.graph.num_of_components[s])
    
    def alpha_G(self):
        """
        G is the graph
        """
        
        max_alpha = 0
        max_alpha_component = None
        for x in self.graph.vertices:
            alpha_C_sum = sum(self.alpha_C((x,i)) for i in range(self.graph.num_of_components[x]))
            if max_alpha < alpha_C_sum:
                max_alpha = alpha_C_sum
                max_alpha_component = x
                    
            
        print("max_alpha_component:", max_alpha_component)
        independent_set = {max_alpha_component}
        for i in range(self.graph.num_of_components[max_alpha_component]):
            component_key = (max_alpha_component, i)
            independent_set = independent_set.union(self.max_component_vertices_set[component_key])
        print("max independent set:", independent_set)
        self.graph.independent_set = independent_set
        # self.graph.show()
        return max_alpha + 1
    
    def run(self):
        return self.computing_independent_set()
