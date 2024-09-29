#ifndef MAKING_TREE_CORE_GRAPH
#define MAKING_TREE_CORE_GRAPH

#include "Grid.h"
#include "Utils.h"
#include <vector>
#include "../rasterizer/Camera.h"
#include "SDL2Auxiliary.h"
#include <map>
#include <functional>
#include <queue>
#include <algorithm>

#define K 15
#define W 128
#define S W*W*W
#define R 5

struct Edge{
   int src;
   int dest;
   float weight;
};

struct Node{
   int id;
   int parentId;
   int steps;
   float dist;
   glm::vec3 guidingVector;
};

class Graph{

   public:
      void generateGraph(){
         populateGridSequential();
         //initilizeEdgesMap();
      }
      
      // Direct seequential implementation
      void populateGridSequential(){

         grid.set(0,0,0,glm::vec3(0.5f,0.1f,0.5f)); // Tree root
         grid.set(0,W-1,0, glm::vec3(0.5f, W-0.5f, 0.5f)); // Top of the trunk ? 
         for(int x(grid.minXPosition()); x < grid.maxXPosition(); x+=5){
            for(int y(grid.minYPosition()); y < grid.maxYPosition(); y+=5){
               for(int z(grid.minZPosition()); z < grid.maxZPosition(); z+=5){
                  populateSubgrid(x,y,z);
               }
            }
         }
      }

      // Shows all the nodes in the graph
      void drawNodes(Camera& camera, SDL2Aux* aux){
         grid.showElements(camera, aux);
      }

      // Shows all the generated edges
      void drawEdges(Camera& camera, SDL2Aux* aux){
         glm::vec3 v1;
         glm::vec3 v2;
         for(const auto& v : edges){
            for(const auto& e : v.second){
               grid.fromIndex(e.src, v1);
               grid.fromIndex(e.dest, v2);
               camera.drawLine(v1, v2, cyan, aux);
            }
         }
      }

      // The grid holding the graph 
      Grid grid = Grid(W,W,W);

      // Uses Dijkstra algorithm to compute a branch to a given endpoint
      void shortestPath(const std::vector<int>& sources, std::vector<int> dests){
         std::cout << "About to compute the shortest paths" << std::endl;
         
         auto compareNodes = [this](int n1, int n2) -> bool {
            return nodes.at(n1).dist > nodes.at(n2).dist;
         };
         
         std::priority_queue<int, std::vector<int>, decltype(compareNodes)> queue(compareNodes);
         
         for(auto src : sources){
            if(nodes.find(src) == nodes.end()){
               glm::vec3 srcGuidingVector = defaultSrcVector;
               if(usePreviousGuidingVector and guidingVectorMemoryMap.find(src) != guidingVectorMemoryMap.end()){
                  srcGuidingVector = guidingVectorMemoryMap.at(src);
               }
               if(steps.find(src) != steps.end()){
                  nodes.emplace(src, Node({src, -1, steps.at(src), 0, srcGuidingVector}));
               }else{
                  nodes.emplace(src, Node({src, -1, 0, 0, srcGuidingVector}));
               }  
            }
            else{
               nodes.at(src).dist = 0;
            }
            queue.push(src);
         }

         int current = 0;
         int other = 0;

         while(!queue.empty() and !dests.empty()){
            current = queue.top();
            bool opened = exploreNode(current, std::find(sources.begin(), sources.end(), current) != sources.end());
            queue.pop();

            if(opened){
               auto currentDestIterator(std::find(dests.begin(), dests.end(), current));
               if(currentDestIterator != dests.end()){
                  dests.erase(currentDestIterator);
               }

               for(const auto& e : edges.at(current)){
                  if(nodes.find(e.dest) == nodes.end()){
                     nodes.emplace(e.dest, Node({e.dest, current, nodes.at(current).steps + 1, std::numeric_limits<float>::max(), glm::vec3(NaN, NaN, NaN)}));
                  }
                  // TODO : Update distances and parent node
                  if(nodes.at(current).dist + e.weight < nodes.at(e.dest).dist){
                     nodes.at(e.dest).dist = nodes.at(current).dist + e.weight;
                     nodes.at(e.dest).parentId = current;
                     nodes.at(e.dest).steps = nodes.at(current).steps + 1;
                     queue.push(e.dest);
                  }
               }
            }
         }

         std::cout << "Computed the shortest path" << std::endl;

      }

      // Creates edges and return a chain of nodes using the node parent relations defined by shortestPath
      void reconstructPath(
         std::vector<int> destinations, 
         const std::vector<int>& sources, 
         std::vector<int>& pathNodes, 
         std::vector<int>& pathEdges
      ){
         int currentNode;
         int parentNode;

         for(auto n : sources){
            pathNodes.push_back(n);
         }

         while(!destinations.empty()){
            currentNode = destinations.back();
            destinations.pop_back();
            while(std::find(pathNodes.begin(), pathNodes.end(), currentNode) == pathNodes.end()){
               parentNode = nodes.at(currentNode).parentId;
               pathEdges.push_back(parentNode);
               pathEdges.push_back(currentNode);
               pathNodes.push_back(currentNode);
               currentNode = parentNode;
            }
         }
      }

      // Creates edges and return a chain of nodes using the node parent relations defined by shortestPath
      void reconstructPath(
         std::vector<int> destinations, 
         const std::vector<int>& sources, 
         std::vector<int>& pathNodes, 
         std::vector<int>& pathEdges,
         std::vector<std::vector<int>>& branches
      ){

         std::cout << "About to reconstruct paths" << std::endl;

         int currentNode;
         int parentNode;

         for(auto n : sources){
            pathNodes.push_back(n);
         }

         std::sort(
            destinations.begin(), 
            destinations.end(), 
            [&](int n1, int n2) {
               return nodes.at(n1).steps < nodes.at(n2).steps; 
            }
         );

         while(!destinations.empty()){
            currentNode = destinations.back();
            std::vector<int> branch = std::vector<int>({currentNode});
            destinations.pop_back();
            while(std::find(pathNodes.begin(), pathNodes.end(), currentNode) == pathNodes.end()){
               parentNode = nodes.at(currentNode).parentId;
               pathEdges.push_back(parentNode);
               pathEdges.push_back(currentNode);
               pathNodes.push_back(currentNode);
               branch.push_back(currentNode);
               currentNode = parentNode;
            }
            branch.push_back(currentNode);
            branches.push_back(branch);
         }

         std::cout << "Path reconstructed" << std::endl;

      }

      // Find all the nodes at a distance exactly equal to H hops from the core nodes
      void findBorders(int H, const std::vector<int>& core, std::vector<int>&  border, float savedProportion, bool reversedProportion){
         std::cout << "About to compute border" << std::endl;

         std::map<int, std::vector<Edge>> edges;
         std::map<int, int> hs;
         std::map<int, int> ss;
         std::vector<int> c1;
         std::vector<int> c2;
         for(auto c : core){
            hs.emplace(c, 0);
            c1.push_back(c);
            ss.emplace(c, nodes.at(c).steps);
         }

         for(int i(0); i < H; i++){
            if(i%2 == 0){
               std::cout << "c1 " << c1.size() << std::endl;
               c2.clear();
            }else{
               std::cout << "c2 " << c2.size() << std::endl;
               c1.clear();
            }
            for(auto c : ((i % 2) == 0 ? c1 : c2)){
               
               edges.emplace(c, std::vector<Edge>());
               int d;
               float l;
               for(int j(0); j < 8; j++){
                  findEdge(c, j, d, l);
                  edges.at(c).push_back(Edge{c, d, 0});
               }

               for(const auto& e : edges.at(c)){
                  if(hs.find(e.dest) == hs.end()){
                     if((i%2) == 0){
                        c2.push_back(e.dest);
                     }else{
                        c1.push_back(e.dest);
                     }
                     hs.emplace(e.dest, i+1);
                     ss.emplace(e.dest, ss.at(c) +1);
                  }
               }
            }
         }

         if((H%2) == 0){
            border = c1;
         }else{
            border = c2;
         }

         std::sort(border.begin(), border.end(), [&](int a, int b) -> bool {
               return ss.at(b) < ss.at(a);
         });

         if(reversedProportion){
            border.erase(border.begin(), border.begin() + (int) std::floor(border.size() * (savedProportion)));
         }else{
            border.erase(border.end() - (int) std::floor(border.size() * (1-savedProportion)), border.end());
         }

         std::cout << "Computed borders" << std::endl;
      }

      void resetGraph(){
         std::cout << "About to reset graph" << std::endl;
         edges.clear();
         initilizeEdgesMap();
         for(const auto n : nodes){
            if(steps.find(n.first) == steps.end())
               steps.emplace(n.first, n.second.steps);
         }
         nodes.clear();
         std::cout << "Graph reset done" << std::endl;
      }

      void setComputeGuidingVector(std::function<void(Node&, const Node&, const Grid&)> computeGuidingVector){
         this->computeGuidingVector = computeGuidingVector;
      }

      void setComputeEdgeWeight(std::function<float(const Node& src, int dest, float squaredLength, const Grid& grid)> computeEdgeWeight){
         this->computeEdgeWeight = computeEdgeWeight;
      }

      void setDefaultGuidingVectors(glm::vec3& defaultGuidingVector){
         this->defaultSrcVector = defaultGuidingVector;
      }

      void setUsePreviousGuidingVector(bool b){
         this->usePreviousGuidingVector = b;
      }

      void resetGuidingVectorMemory(){
         this->guidingVectorMemoryMap.clear();
      }

      std::map<int, Node> nodes;

   private:
      
      std::map<int, std::vector<Edge>> edges;
      std::map<int, int> steps;
      glm::vec3 defaultSrcVector = glm::vec3(0.f,1.f,0.f);
      bool usePreviousGuidingVector = false;

      std::map<int, glm::vec3> guidingVectorMemoryMap; // Keep in memory the computed guiding vectors accross the steps

      std::function<void(Node&, const Node&, const Grid&)> computeGuidingVector; // Node& node, const Node& parentNode, const Grid& grid
      std::function<float(const Node& src, int dest, float squaredLength, const Grid& grid)> computeEdgeWeight;

      // TODO play with threads for parallel Poisson disc sampling
      void populateGrid(){
         
      }

      // Not sure if I want to keep it
      void initilizeEdgesMap(){
         glm::vec3 v;
         for(int i(0); i < S; i++){
            if(grid.fromIndex(i, v)){
               edges.emplace(i, std::vector<Edge>());
            }
         }
      }

      // Explore neighbooring quadrant to find the outgoing edges and compute their costs 
      bool exploreNode(int node, bool isSource){
         if(!isSource and !isnan(nodes.at(node).guidingVector.x)) return false;
         if(!isSource){
            computeGuidingVector(nodes.at(node), nodes.at(nodes.at(node).parentId), grid);
            if(guidingVectorMemoryMap.find(node) == guidingVectorMemoryMap.end()){
               guidingVectorMemoryMap.emplace(node, nodes.at(node).guidingVector);
            }else{
               guidingVectorMemoryMap.at(node) = nodes.at(node).guidingVector;
            }
         }

         edges.emplace(node, std::vector<Edge>());

         int dest;
         float squaredLength;
         for(int i(0); i < 8; i++){
            if(findEdge(node, i, dest, squaredLength)){
               float weight(computeEdgeWeight(nodes.at(node), dest, squaredLength, grid));
               edges.at(node).push_back(Edge({node, dest, weight}));
            }
         }

         return true;
      }

      bool findEdge(int src, int quadrant, int& dest, float& squaredLength){
         std::vector<int> neighboorhood;
         glm::vec3 srcPoint;
         dest = -1;
         if(!grid.fromIndex(src, srcPoint)) return false;
         glm::vec3 other;
         if(!grid.getQuadrantNeighboorhood(src, R, quadrant, neighboorhood)) return false;
         squaredLength = std::numeric_limits<float>::max();
         for(auto cell : neighboorhood){
            if(grid.fromIndex(cell, other) and isWellPlaced(quadrant, srcPoint, other) and squaredDistance(srcPoint, other) < squaredLength){
               squaredLength = squaredDistance(srcPoint, other);
               dest = cell;
            }
         }
         return dest != -1;
      }

      bool isWellPlaced(int quadrant, const glm::vec3& cell, const glm::vec3& other){
         int x = quadrant & 1; int y = quadrant & 2; int z = quadrant & 4;
         return (x xor other.x < cell.x) and (y xor other.y < cell.y) and (z xor other.z < cell.z);
      }

      //Divides the grid in "supercells" of 5*5*5 cells and perform a poisson sampling on each of these supercells
      //allowing parallelization. 
      //Take the coordinates of the top-left-closest cell of the "supercell" and populate the supercell using poisson disc sampling 
      void populateSubgrid(int x, int y, int z){
         std::vector<int> activeList(0);
         glm::vec3 rand(random3DVec(x+2, y+2, z+2, 1, 1, 1));
         glm::ivec3 discrete(0,0,0);
         if(grid.discretize(rand, discrete)){
            activeList.push_back(grid.set(discrete.x, discrete.y, discrete.z, rand));
         }

         while(activeList.size() > 0){
            glm::vec3 point; 
            glm::vec3 newPoint;
            glm::ivec3 newPointDiscreteCoordinates;
            grid.fromIndex(activeList.at(0), point);

            for(int i(0); i<K; i++){
               newPoint = sampleFromAnulus(point);

               if(grid.discretize(newPoint, newPointDiscreteCoordinates) 
                  and isInSuperCell(x, y, z, newPointDiscreteCoordinates) 
                  and checkNeighboorhood(newPoint, newPointDiscreteCoordinates)){
                  
                  activeList.push_back(
                     grid.set(
                        newPointDiscreteCoordinates.x, 
                        newPointDiscreteCoordinates.y, 
                        newPointDiscreteCoordinates.z,
                        newPoint
                     )
                  );
               };
            }
            activeList.erase(activeList.begin());
         }

      }

      bool isInSuperCell(int superX, int superY, int superZ, const glm::ivec3& cell){
         return cell.x >= superX and cell.x < superX + 5 and 
                cell.y >= superY and cell.y < superY + 5 and
                cell.z >= superZ and cell.z < superZ + 5;
      }

      bool checkNeighboorhood(const glm::vec3& point, const glm::ivec3& discreteCoords){
         if(grid.isCellOccupied(discreteCoords.x, discreteCoords.y, discreteCoords.z)){
            return false;
         }
         glm::vec3 otherPoint;
         for(int i(-1); i<2; i++){
            for(int j(-1); j<2; j++){
               for(int k(-1); k<2; k++){

                  if(
                     grid.isCellInGrid(discreteCoords.x+i, discreteCoords.y+j, discreteCoords.z+k) and 
                     grid.at(discreteCoords.x+i, discreteCoords.y+j, discreteCoords.z+k, otherPoint) and 
                     squaredDistance(point, otherPoint) < 3 
                  ){
                     return false;
                  }
               }
            }
         }
         
         return true;
      }

};

#endif