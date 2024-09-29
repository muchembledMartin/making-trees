#ifndef MAKING_TREE_CORE_TREE
#define MAKING_TREE_CORE_TREE

#include "Graph.h"
#include <algorithm>
#include "../inflator/Inflator.h"

class Tree{

   public:
      int type = 0;
      Graph graph;
      std::vector<glm::vec3> structure;
      std::vector<std::vector<int>> branches;
      float maxBranchRadius = 2.f;
      float minBranchRadius = 0.3f;
      std::vector<int> pathNodes;
      std::vector<int> pathNodes2;

      std::vector<int> border;
      std::vector<int> border2;
      std::vector<int> dests;

      int firstLim;

   private:
      // Generation parameters

      int steps;
      std::vector<int> samplesCounts; // The number of endpoints at each step
      std::vector<int> borderSizes; // The number of step to take from the core to find the border to sample elements from
      std::vector<float> borderKeptProportion; // The proportion of points we want to keep when ordered by number of step from origin
      std::vector<bool> reversedProportions;  // If false, take the points with greatest distance from origin, true the closests
      std::vector<std::function<void(Node&, const Node&, const Grid&)>> guidingVectorsComputers; // Guiding vector function to use at each step
      std::vector<int> edgeCostComputers; // Edge cost computer to use at each step, see edgeWeightComputerFromIndex
      std::vector<bool> usePreviousVectors; // Reuse the guiding vector computed in the previous step, or use the default src vector defined below
      std::vector<glm::vec3> defaultSrcVectors;
      std::function<void(int, std::vector<int>&)> firstSampler; // How to sample the points in the first step.
      bool hasTrunck = false; // Does the tree has a trunk



   public:

      void setCustomConfig(
         int steps,
         std::vector<int>& samplesCounts, 
         std::vector<int>& borderSizes,
         std::vector<float>& borderKeptProportion,  
         std::vector<bool>& reversedProportions, 
         std::vector<std::function<void(Node&, const Node&, const Grid&)>> guidingVectorsComputers,
         std::vector<int> edgeCostComputers, 
         std::vector<bool> usePreviousVectors, 
         std::vector<glm::vec3> defaultSrcVectors,
         std::function<void(int, std::vector<int>&)> firstSampler,
         bool hasTrunck
      ){
         this->steps = steps;
         this->samplesCounts = samplesCounts;
         this->borderSizes = borderSizes;
         this->borderKeptProportion = borderKeptProportion;
         this->reversedProportions = reversedProportions;
         this->guidingVectorsComputers = guidingVectorsComputers;
         this->edgeCostComputers = edgeCostComputers;
         this->usePreviousVectors = usePreviousVectors;
         this->defaultSrcVectors = defaultSrcVectors;
         this->firstSampler = firstSampler;
         this->hasTrunck = hasTrunck;
      }

      void setDefaultConfig(){
         hasTrunck = false;
         steps = 5;
         samplesCounts = {6, 100, 200, 400, 800};
         borderSizes = {10, 4, 4, 4};
         borderKeptProportion = {0.5,0.6, 0.6, 0.6};
         guidingVectorsComputers = {
            computeGuidingVectorsDefault(-3*PI/180.f), 
            computeGuidingVectorsDefault(-3*PI/180.f), 
            computeGuidingVectorsDefault(1*PI/180.f), 
            computeGuidingVectorsDefault(1*PI/180.f),
            computeGuidingVectorsDefault(1*PI/180.f)
         };
         edgeCostComputers = {0,0,0,0,0};
         usePreviousVectors = {true, true, true, true};
         reversedProportions = {false, false, false, false};
         defaultSrcVectors = {glm::vec3(0.f,1.f,0.f), glm::vec3(0.f,1.f,0.f), glm::vec3(0.f,1.f,0.f), glm::vec3(0.f,1.f,0.f), glm::vec3(0.f,1.f,0.f)};
         firstSampler = [&](int n, std::vector<int>& destinations)->void {
            sampleFromHalfAnulus(glm::vec3(0,50,0), 25.f, 35.f, n, destinations);
         };
      }

      void setExample2Config(){
         hasTrunck = true;
         steps = 3;
         samplesCounts = {35, 200, 400};
         borderSizes = {10, 6, 5};
         borderKeptProportion = {0.6,0.8, 0.8};
         guidingVectorsComputers = {
            computeGuidingVectorsDefault(3*PI/180.f), 
            computeGuidingVectorsDefault(3*PI/180.f), 
            computeGuidingVectorsDefault(1*PI/180.f), 
            
         };
         edgeCostComputers = {0,0,0};
         usePreviousVectors = {true, true, true};
         defaultSrcVectors = {glm::vec3(0.f,1.f,0.f), glm::vec3(0.f,1.f,0.f), glm::vec3(0.f,1.f,0.f)};
         reversedProportions = {false, false, false};
         firstSampler = [&](int n, std::vector<int>& destinations)->void {
            sampleFromAnulus(glm::vec3(0,80,0), 30.f, 40.f, n, destinations);
         };
      }

      void setExample3Config(){
         steps = 1;
         samplesCounts = {6};
         
         guidingVectorsComputers = {
            computeGuidingVectorsReverseAfterD(-3*PI/180.f, 20), 
         };         
         edgeCostComputers = {0};

         firstSampler = [&](int n, std::vector<int>& destinations)->void {
            sampleFromHalfAnulus(glm::vec3(0,80,0), 30.f, 60.f, n, destinations);
         };

         defaultSrcVectors = {glm::vec3(0.f,1.f,0.f)};
      }

      void setExample4Config(){
         steps = 1;
         samplesCounts = {6};
         
         guidingVectorsComputers = {
            computeGuidingVectorsDefault(-3*PI/180.f), 
         };         
         edgeCostComputers = {0};

         firstSampler = [&](int n, std::vector<int>& destinations)->void {
            sampleFromHalfAnulus(glm::vec3(0,80,0), 30.f, 60.f, n, destinations);
         };

         defaultSrcVectors = {glm::vec3(0.f,1.f,0.f)};
      }

      

      std::function<void(Node&, const Node&, const Grid&)> computeGuidingVectorsDefault(float alpha){
         return [=](Node& node, const Node& parent, const Grid& grid)-> void {
		      float cos(glm::cos(alpha)), sin(glm::sin(alpha));
            glm::vec3 n,p;
            grid.fromIndex(node.id, n);
            grid.fromIndex(parent.id, p);
            glm::mat3 localRotation(glm::mat3(glm::vec3(1, 0, 0), glm::vec3(0,cos,sin), glm::vec3(0, -sin, cos)));
            glm::vec3 rotAxis(glm::normalize(glm::cross(n-p ,glm::vec3(0,1,0))));
            glm::mat3 base(rotAxis, glm::vec3(0,1,0), glm::normalize(glm::cross(rotAxis, glm::vec3(0,1,0))));
            node.guidingVector = parent.guidingVector * glm::inverse(base) * localRotation * base;
	      };
      }

      std::function<void(Node&, const Node&, const Grid&)> computeGuidingVectorsReverseAfterD(float alpha, float d){
         return [=](Node& node, const Node& parent, const Grid& grid)-> void {
            float a = alpha;
            glm::vec3 n,p;
            grid.fromIndex(node.id, n);
            grid.fromIndex(parent.id, p);
            if(n.x*n.x+n.z*n.z > d*d){
               a = -a;
            }
            float cos(glm::cos(a)), sin(glm::sin(a));
            glm::mat3 localRotation(glm::mat3(glm::vec3(1, 0, 0), glm::vec3(0,cos,sin), glm::vec3(0, -sin, cos)));
            glm::vec3 rotAxis(glm::normalize(glm::cross(n-p ,glm::vec3(0,1,0))));
            glm::mat3 base(rotAxis, glm::vec3(0,1,0), glm::normalize(glm::cross(rotAxis, glm::vec3(0,1,0))));
            node.guidingVector = parent.guidingVector * glm::inverse(base) * localRotation * base;
	      };
      }


      std::function<void(Node&, const Node&, const Grid&)> computeGuidingVectorsLinearField(glm::vec3 vector){
         return [=](Node& node, const Node& parent, const Grid& grid)-> void {
		      node.guidingVector = vector;
	      };
      }

      std::function<void(Node&, const Node&, const Grid&)> computeGuidingVectorsCylindricalField =
         [=](Node& node, const Node& parent, const Grid& grid)-> void {
            glm::vec3 point;
            grid.fromIndex(node.id, point);
            point.z = 0;
		      point = glm::normalize(point); 
            node.guidingVector = glm::cross(point, glm::vec3(0.f,1.f,0.f));
	      };
      
      std::function<float(const Node& src, int dest, float squaredLength, const Grid& grid)> computeEdgeWeightDefault = 
         [](const Node& src, int dest, float squaredLength, const Grid& grid) -> float {
            glm::vec3 p1, p2;
            grid.fromIndex(src.id, p1);
            grid.fromIndex(dest, p2);
            p1 = p2-p1;
            return glm::sqrt(squaredLength)*(1-glm::dot(glm::normalize(p1), src.guidingVector));
         };

      std::function<float(const Node& src, int dest, float squaredLength, const Grid& grid)> computeEdgeWeightOnlyDown = 
         [](const Node& src, int dest, float squaredLength, const Grid& grid) -> float {
            glm::vec3 p1, p2;
            grid.fromIndex(src.id, p1);
            grid.fromIndex(dest, p2);
            if(p2.y > p1.y) return squaredLength*5;
            p1 = p2-p1;
            return glm::sqrt(squaredLength)*(1-glm::dot(glm::normalize(p1), src.guidingVector));
         };
      
      std::function<float(const Node& src, int dest, float squaredLength, const Grid& grid)> computeEdgeWeightDistance = 
         [](const Node& src, int dest, float squaredLength, const Grid& grid) -> float {
            
            return glm::sqrt(squaredLength);
         };

      // Initialize the graph : 
      //    - Populate the graph with nodes
      //    - Set the correct weight and guiding vectors function for first step
      void initializeGraph(){
         graph.generateGraph();
      }

      void generateTree(std::vector<std::vector<glm::vec3>>& meshes3D){

         std::vector<int> src({{graph.grid.indexFromCell(0,0,0)}});
         std::vector<int> destinations({});

         if(hasTrunck){

            std::vector<int> trunkNodes;
            std::vector<int> trunkEdges;
            std::vector<glm::vec3> trunkMeshes;
            destinations.push_back(graph.grid.indexFromCell(0,W-1,0));
            graph.setComputeGuidingVector(computeGuidingVectorsLinearField(glm::vec3(0.,1.,0.)));
            graph.setComputeEdgeWeight(computeEdgeWeightDefault);
            graph.shortestPath(src, destinations);
            branches.clear();
            graph.reconstructPath(destinations, src, trunkNodes, trunkEdges, branches);
            generate3DMeshes(trunkMeshes, 0);
            meshes3D.push_back(trunkMeshes);
            graph.resetGraph();

            src = std::vector<int>(trunkNodes);

            trunkNodes.clear();
            destinations.clear();
            graph.resetGuidingVectorMemory();

         }

         for(int s(0); s < steps; s++){

            std::cout << "Starting step generation of step " << s << std::endl;

            std::vector<glm::vec3> stepMeshes;
            std::vector<std::vector<int>> stepBranches;
            std::vector<int> stepPathNodes;
            std::vector<int> stepPathEdges;
            std::vector<int> stepBorder;
            
            if(s == 0){        // Samples from a volume if it is the first step, a sampling from the border is made at the end
               destinations.clear();
               firstSampler(samplesCounts[s], destinations);
            }

            std::cout << "Setting edge weight and guiding vector computers" << std::endl;

            graph.setComputeEdgeWeight(edgeWeightComputerFromIndex(edgeCostComputers.at(s)));
            graph.setComputeGuidingVector(guidingVectorsComputers.at(s));
            graph.setDefaultGuidingVectors(defaultSrcVectors.at(s));
            if(s>0){
               graph.setUsePreviousGuidingVector(usePreviousVectors.at(s-1));
            }

            std::cout << "About to construct shortest path" << std::endl;

            graph.shortestPath(src, destinations);
            std::cout << "About to reconstruct the path" << std::endl;

            branches.clear();
            graph.reconstructPath(destinations, src, stepPathNodes, stepPathEdges, branches);

            std::cout << "About to generate meshes with " << branches.size() << " branches" << std::endl;
            
            generate3DMeshes(stepMeshes, s);

            std::cout << "Generated meshes" << std::endl;
            
            meshes3D.push_back(stepMeshes);

            if(s+1<steps){

               std::cout << "About to find borders" << std::endl;

               graph.findBorders(borderSizes.at(s), stepPathNodes, stepBorder, borderKeptProportion.at(s), reversedProportions.at(s));
            }

            std::cout << "About to reset graph" << std::endl;

            graph.resetGraph();

            src = std::vector<int>(stepPathNodes);

            stepPathNodes.clear();
            destinations.clear();
            if(s+1<steps){
               sampleFromSet(stepBorder, samplesCounts.at(s+1), destinations);
            }
         }
      }

      // Return the edge weight computer corresponding to the given index :
      // 0 - Default
      // 1 - By distance only
      // 2 - Penalizing upward edges (TODO)
      std::function<float(const Node& src, int dest, float squaredLength, const Grid& grid)> edgeWeightComputerFromIndex(int index){
         switch (index)
         {
            case 0:
               return computeEdgeWeightDefault;
            case 1:
               return computeEdgeWeightDistance;
            case 2: 
               return computeEdgeWeightOnlyDown;
            default:
               break;
         }
      }

      // Generate the tree structure depending on the type of tree 
      void generateStructure(std::vector<std::vector<glm::vec3>>& meshes3D){

         

         if(type == 0){
            
               std::vector<glm::vec3> meshes1;
               std::vector<glm::vec3> meshes2;
               std::vector<glm::vec3> meshes3;

               std::vector<int> src({graph.grid.indexFromCell(0,0,0)}); 
               std::vector<int> pathEdges;


               sampleFromHalfAnulus(glm::vec3(0,50,0), 20.f, 50.f, 5, dests);

               graph.setComputeEdgeWeight(computeEdgeWeightDefault);
               graph.setComputeGuidingVector(computeGuidingVectorsDefault(-3*PI/180.f));

               graph.shortestPath(src, dests);
               graph.reconstructPath(dests, src, pathNodes, pathEdges, branches);

               addToStructure(pathEdges);

               firstLim = structure.size();  
               generate3DMeshes(meshes1, 1);


               graph.findBorders(4, pathNodes, border, 0.8f, false);


               graph.resetGraph();

               src = pathNodes;
               dests.clear();
               sampleFromSet(border, 300, dests);

               graph.shortestPath(src, dests);
               branches.clear();
               graph.reconstructPath(dests, src, pathNodes2, pathEdges, branches);
               
               addToStructure(pathEdges);

               maxBranchRadius = 0.3f;
               minBranchRadius = 0.1f;

               generate3DMeshes(meshes2, 1);

               graph.findBorders(6, pathNodes2, border2, 0.7f, false);

               
               graph.resetGraph();


               src = pathNodes2;
               dests.clear();

               std::cout << "before sampling" << std::endl;
               sampleFromSet(border2, 700, dests);
               std::cout << dests.size() << std::endl;
               std::cout << "after sampling" << std::endl;

               
               graph.shortestPath(src, dests);
               branches.clear();
               graph.reconstructPath(dests, src, pathNodes2, pathEdges, branches);


               maxBranchRadius = 0.1f;
               minBranchRadius = 0.01f;
               generate3DMeshes(meshes3, 3);
               addToStructure(pathEdges);

               meshes3D.push_back(meshes1);
               meshes3D.push_back(meshes2);
               meshes3D.push_back(meshes3);
            
         } 


      };

      // Sample n point of the grid from the sphere of center c and radius r
      void sampleFromSphere(glm::vec3 c, float r, int n, std::vector<int>& samples){
         glm::ivec3 cell;
         graph.grid.discretize(c, cell);
         int index = graph.grid.indexFromCell(cell.x, cell.y, cell.z);

         std::vector<int> extendedVolume;
         std::vector<int> extendedSamples;
         graph.grid.getNeighboorhood(index, (int) std::ceil(r), extendedVolume);

         glm::vec3 v;
         for(auto i : extendedVolume){
            if(graph.grid.fromIndex(i,v))
               extendedSamples.push_back(i);
         }

         sampleFromSet(extendedSamples, n, samples);
      }

      void sampleFromHalfAnulus(glm::vec3 c, float rin, float rout, int  n, std::vector<int>& samples){
         glm::ivec3 cell;
         graph.grid.discretize(c, cell);
         int index = graph.grid.indexFromCell(cell.x, cell.y, cell.z);

         std::vector<int> extendedVolume;
         std::vector<int> extendedSamples;
         graph.grid.getNeighboorhood(index, (int) std::ceil(rout), extendedVolume);

         glm::vec3 v;
         for(auto i : extendedVolume){
            if(graph.grid.fromIndex(i,v)){
               float d = glm::length(c-v);
               if(d <= rout and d >= rin and v.y > c.y)
                  extendedSamples.push_back(i);
            }
         }

         sampleFromSet(extendedSamples, n, samples);
         
      }

      // Sample n point of the grid from the 3d anulus of center c and radius r and R
      void sampleFromAnulus(glm::vec3 c, float rin, float rout, int n, std::vector<int>& samples){
         glm::ivec3 cell;
         graph.grid.discretize(c, cell);
         int index = graph.grid.indexFromCell(cell.x, cell.y, cell.z);

         std::vector<int> extendedVolume;
         std::vector<int> extendedSamples;
         graph.grid.getNeighboorhood(index, (int) std::ceil(rout), extendedVolume);

         glm::vec3 v;
         for(auto i : extendedVolume){
            if(graph.grid.fromIndex(i,v)){
               float d = glm::length(c-v);
               if(d <= rout and d >= rin)
                  extendedSamples.push_back(i);
            }
               
         }

         sampleFromSet(extendedSamples, n, samples);
      }
      
      void sampleFromSet(std::vector<int>& set, int n, std::vector<int>& samples){
         std::cout << "About to sample from set " << set.size() << std::endl;
         n = (set.size() <= n) ? set.size()-1 : n;
         while(samples.size() < n){
            int chosen(randomInt(0, set.size()-1));
            samples.push_back(set.at(chosen));
            set.erase(set.begin() + chosen);
         }
         std::cout << "Sampled  " << n << " from set" << std::endl;
      }

      void addToStructure(const std::vector<int>& newEdges){
         glm::vec3 v;
         for(auto i : newEdges){
            graph.grid.fromIndex(i, v);
            structure.push_back(v);
         }
      }
      
      void generate3DMeshes(std::vector<glm::vec3>& triangles, int genStep){
         Inflator inflator;

         std::sort(branches.begin(), branches.end(), [&](std::vector<int>& b1, std::vector<int>& b2) -> bool {
            if(b1.size() == 0) return true;
            if(b2.size() == 0) return false;
            return graph.nodes.at(b1.at(0)).steps < graph.nodes.at(b2.at(0)).steps;
         });

         std::cout << "Sorted " << branches.back().size() << std::endl;
         float longestBranchSize = graph.nodes.at(branches.back().at(0)).steps;
         std::cout << "longest branch size : " << longestBranchSize << std::endl;

         int i = 0;
         for(const auto& b : branches){
            // if(i++ != 5) continue;

            Node end = graph.nodes.at(b.at(0));
            Node start = graph.nodes.at(b.back());

            std::vector<glm::vec3> branchMesh;
            std::transform(b.begin(), b.end(), std::back_inserter(branchMesh), [&](int n) -> glm::vec3 {
               glm::vec3 v;
               graph.grid.fromIndex(graph.nodes.at(n).id, v);
               return v;
            });

            // std::cout << "Branch start steps : " << start.steps  << " r : " << (longestBranchSize - 2 - start.steps) * maxBranchRadius / longestBranchSize << std::endl;
            // std::cout << "Branch end steps : " << end.steps << " r : " << (longestBranchSize - 2 - end.steps) * maxBranchRadius / longestBranchSize << std::endl; 

            //inflator.inflateChain(
            //   branchMesh,
            //   (longestBranchSize - start.steps) * (maxBranchRadius-minBranchRadius) / longestBranchSize + minBranchRadius, 
            //   (longestBranchSize - end.steps) * (maxBranchRadius-minBranchRadius) / longestBranchSize + minBranchRadius, 
            //   triangles
            //);

            inflator.inflateChain(
               branchMesh,
               (longestBranchSize - start.steps) * (maxBranchRadius-minBranchRadius) * (steps-genStep) / (longestBranchSize*steps) + minBranchRadius, 
               (longestBranchSize - end.steps) * (maxBranchRadius-minBranchRadius) * (steps-genStep) / (longestBranchSize*steps) + minBranchRadius, 
               triangles
            );
         }
      }

};

#endif