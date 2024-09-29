#ifndef MAKING_TREE_CORE_GRID
#define MAKING_TREE_CORE_GRID

#include <glm/glm.hpp>
#include <vector>
#include <cmath>
#include "../rasterizer/Camera.h"
#include "SDL2Auxiliary.h"

class Grid{

   private:
      std::vector<bool> grid;
      std::vector<glm::vec3> elements;

      int width;
      int height;
      int depth;

      int elementCount = 0;

   public:

      Grid(int width, int height, int depth):
         width(width), height(height), depth(depth)
      {
         grid = std::vector<bool>(width*height*depth, 0);
         elements = std::vector<glm::vec3>(width*height*depth);
      }

      int minXPosition() const {
         return -width/2;
      }

      int minYPosition() const {
         return 0;
      }

      int minZPosition() const {
         return -depth/2;
      }

      int maxXPosition() const {
         return (width % 2 == 0) ? width/2 : width/2+1;
      }

      int maxYPosition() const {
         return height;
      }

      int maxZPosition() const {
         return (depth % 2 == 0) ? depth/2 : depth/2+1;
      }

      int indexFromCell(int x, int y, int z) const {
         if(!isCellInGrid(x,y,z)) return -1;
         return (x + width/2) + y * width + (z + depth/2) * height*width;
      }

      // Return the discrete coordinates of the index'th cell
      bool cellFromIndex(int index, glm::ivec3& iv3) const {
         if(index >= grid.size() or index < 0) return false;
         int x = (index%height)-width/2;
         int y = (index%(height*width))/height;
         int z = (index/(height*width)) - depth/2;
         iv3 = glm::ivec3(x, y, z);
         return true;
      }

      // Update v and return true if there's an element in that cell, false otherwise.
      bool at(int x, int y, int z, glm::vec3& v) const {
         int index = indexFromCell(x,y,z);
         if(index < 0 or !grid.at(index)) return false;

         v = elements.at(index);
         return true;
      }

      bool at(const glm::ivec3& coords, glm::vec3& v) const {
         return at(coords.x, coords.y, coords.z, v);
      }

      // Update v with the index'th element added in the grid
      // Return true if this element exist false otherwise
      bool fromIndex(int index, glm::vec3& v) const {
         if(index < elements.size() and grid.at(index)){
            v = elements.at(index);
            return true;
         }

         return false;
      }

      // Add v to the grid if the cell is free, return the index of the new element
      int set(int x, int y, int z, const glm::vec3& v){
         if(v.x == 0.f and v.y == 0.f and v.z == 0.f) std::cout << "ahahah" << std::endl;
         if(x == 0 and y == 0 and z == -1) std::cout << "ahahah" << std::endl;

         if(isCellOccupied(x,y,z)) return -1;
         int index = indexFromCell(x,y,z);
         elements.at(index)= v;
         grid.at(index) = true;
         elementCount++;
         return index;
      }

      // Return true and update discreteCoords with the coordinate in the grid of the cell containing the 3d point
      // if the point is in the grid. False otherwise. 
      bool discretize(const glm::vec3& coords, glm::ivec3& discreteCoords) const {
         int x = (int) floor(coords.x);
         int y = (int) floor(coords.y);
         int z = (int) floor(coords.z);
         
         if(isCellInGrid(x,y,z)){
            discreteCoords = glm::ivec3(x,y,z);
            return true;
         }
         return false;
      }

      bool isCellInGrid(int x, int y, int z) const {
         return x >= -(width>>1) and x < (width>>1) and 
                y >= 0 and y < height and 
                z >= -(depth>>1) and z < (depth>>1);
      }

      bool isCellOccupied(int x, int y, int z) const {
         int index = indexFromCell(x,y,z);
         if(index < 0) return true;

         return grid.at(index);
      }

      int getElementCount() const {
         return elementCount;
      }

      // Return the indexes of all the cells in the neighboorhood of the cell given by index
      // expect for the cell itself.
      bool getNeighboorhood(int index, int r, std::vector<int>& neighboorhood) const {
         glm::ivec3 cell;
         if(!cellFromIndex(index, cell)) return false;
         int nx, ny, nz;
         for(int x(-r); x <= r; x++){
            for(int y(-r); y <= r; y++){
               for(int z(-r); z <= r; z++){
                  nx = cell.x+x;
                  ny = cell.y+y;
                  nz = cell.z+z;
                  if((x!=0 or y!=0 or z!=0) and isCellInGrid(nx, ny, nz)){
                     neighboorhood.push_back(indexFromCell(nx, ny, nz));
                  }
               }
            }
         }
         return true;
      }

      bool getQuadrantNeighboorhood(int index, int r, int quadrant, std::vector<int>& neighboorhood) const {
         glm::ivec3 cell;
         if(!cellFromIndex(index, cell)) return false;
         int nx, ny, nz; 
         int sx(quadrant & 1 ? 1 : -1);
         int sy(quadrant & 2 ? 1 : -1);
         int sz(quadrant & 4 ? 1 : -1);

         for(int x(0); x < r; x++){
            for(int y(0); y < r; y++){
               for(int z(0); z < r; z++){
                  nx = cell.x+sx*x;
                  ny = cell.y+sy*y;
                  nz = cell.z+sz*z;
                  if((x!=0 or y!=0 or z!=0) and isCellInGrid(nx, ny, nz)){
                     neighboorhood.push_back(indexFromCell(nx, ny, nz));
                  }
               }
            }
         }

         return true;
      }

      // Clearly need a better implementation.
      void showGrid(Camera& camera, SDL2Aux* aux) const {
         glm::vec3 v1, v2;
         for(int y = 0; y < height; y+=height-1){
            for(int z = 0; z < depth; z+=depth-1){
               v1 = glm::vec3(-(width>>1), y, z-(depth>>1));
               v2 = glm::vec3(width>>1, y, z-(depth>>1));
               camera.drawLineBright(v1,v2, green*0.2f, aux);
            }
         }
         for(int x = 0; x < width; x+=width-1){
            for(int z = 0; z < depth; z+=depth-1){
               v1 = glm::vec3(x-(width>>1), 0, z-(depth>>1));
               v2 = glm::vec3(x-(width>>1), height, z-(depth>>1));
               camera.drawLineBright(v1,v2, green*0.2f, aux);
            }
         }
         for(int x = 0; x < width; x+=width-1){
            for(int y = 0; y < height; y+=height-1){
               v1 = glm::vec3(x-(width>>1), y, -(depth>>1));
               v2 = glm::vec3(x-(width>>1), y, (depth>>1));
               camera.drawLineBright(v1,v2, green*0.2f, aux);
            }
         }
      }

      void showElements(Camera& camera, SDL2Aux* aux) const {
         for(int i = 0; i < grid.size(); i++){
            if(grid.at(i)){
               camera.drawPoint(elements.at(i), white, aux);
            }
         }
      }
};

#endif