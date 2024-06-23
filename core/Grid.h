#ifndef MAKING_TREE_CORE_GRID
#define MAKING_TREE_CORE_GRID

#include <glm/glm.hpp>
#include <vector>
#include <cmath>
#include "../rasterizer/Camera.h"
#include "SDL2Auxiliary.h"

class Grid{

   private:
      std::vector<int> grid;
      std::vector<glm::vec3> elements;

      int width;
      int height;
      int depth;

      int indexFromCell(int x, int y, int z){
         if(!isCellInGrid(x,y,z)) return -1;
         return (x + width/2) + y * height + (z + depth/2) * height*depth;
      }

   public:

      Grid(int width, int height, int depth):
         width(width), height(height), depth(depth)
      {}

      // Update v and return true if there's an element in that cell, false otherwise.
      bool at(int x, int y, int z, glm::vec3& v){
         int index = indexFromCell(x,y,z);
         if(index < 0 or grid.at(index) == -1) return false;

         v = elements.at(grid.at(index));
         return true;
      }

      bool at(const glm::ivec3& coords, glm::vec3& v){
         return at(coords.x, coords.y, coords.z, v);
      }

      // Add v to the grid if the cell is free
      bool set(int x, int y, int z, const glm::vec3& v){
         if(isCellOccupied(x,y,z)) return false;
         elements.push_back(v);
         grid.at(indexFromCell(x,y,z)) = elements.size()-1;
         return true;
      }

      // Return true and update discreteCoords with the coordinate in the grid of the cell containing the 3d point
      // if the point is in the grid. False otherwise. 
      bool discretize(const glm::vec3& coords, glm::ivec3& discreteCoords){
         int x = (int) floor(coords.x);
         int y = (int) floor(coords.y);
         int z = (int) floor(coords.z);
         
         if(isCellInGrid(x,y,z)){
            discreteCoords = glm::ivec3(x,y,z);
            return true;
         }
         return false;
      }

      bool isCellInGrid(int x, int y, int z){
         return x >= -(width>>1) and x < (width>>1) and 
                y >= 0 and y < height and 
                z >= -(depth>>1) and z < (depth>>1);
      }

      bool isCellOccupied(int x, int y, int z){
         int index = indexFromCell(x,y,z);
         if(index < 0) return true;

         return grid.at(index) > -1;
      }

      // Clearly need a better implementation.
      void showGrid(Camera& camera, SDL2Aux* aux){
         glm::vec3 v1, v2;
         for(int y = 0; y < height; y++){
            for(int z = 0; z < depth; z++){
               v1 = glm::vec3(-(width>>1), y, z-(depth>>1));
               v2 = glm::vec3(width>>1, y, z-(depth>>1));
               camera.drawLine(v1,v2, white*0.1f, aux);
            }
         }
         for(int x = 0; x < width; x++){
            for(int z = 0; z < depth; z++){
               v1 = glm::vec3(x-(width>>1), 0, z-(depth>>1));
               v2 = glm::vec3(x-(width>>1), height, z-(depth>>1));
               camera.drawLine(v1,v2, white*0.1f, aux);
            }
         }
         for(int x = 0; x < width; x++){
            for(int y = 0; y < height; y++){
               v1 = glm::vec3(x-(width>>1), y, -(depth>>1));
               v2 = glm::vec3(x-(width>>1), y, (depth>>1));
               camera.drawLine(v1,v2, white*0.1f, aux);
            }
         }
      }

};

#endif