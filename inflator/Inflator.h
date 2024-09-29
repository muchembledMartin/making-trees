#ifndef MAKING_TREE_INFLATOR
#define MAKING_TREE_INFLATOR

#include <glm/glm.hpp>
#include <vector>
#include "../core/Utils.h"

class Inflator{
   
   private:
      
      int basePointCount = 5;
      bool pointy = false;
      float stepAngle;
   
      void computeCircle(const glm::vec3& direction, const glm::vec3& start, float r, std::vector<glm::vec3>& circle){
         circle = std::vector<glm::vec3>();
         glm::mat3 rotationMatrix = computeRotationMatrix(direction);
         for(int i(0); i < basePointCount; i++){
            circle.push_back(rotationMatrix * glm::vec3(0.f, r * cos(stepAngle * i),  r * sin(stepAngle * i)) + start);
         }
      }

      glm::mat3 computeRotationMatrix(const glm::vec3& direction){
         glm::vec3 sphericalDirection = carthesianToSpherical(direction);
         
         float sinT = sin(sphericalDirection.y);
         float cosT = cos(sphericalDirection.y);
         float sinP = sin(sphericalDirection.z);
         float cosP = cos(sphericalDirection.z);

         return glm::mat3(glm::vec3(cosT,0,sinT), glm::vec3(0,1,0), glm::vec3(-sinT, 0, cosT)) 
		      * glm::mat3(glm::vec3(cosP, sinP, 0), glm::vec3(-sinP, cosP, 0), glm::vec3(0,0,1));
      }



   public:



      Inflator(): basePointCount(5), pointy(false){
         stepAngle = _2PI / (float) basePointCount;
      }

      Inflator(int basePointCount): basePointCount(basePointCount), pointy(false){
         stepAngle = _2PI / (float) basePointCount;
      }

      void inflateChain(const std::vector<glm::vec3> chain, float rstart, float rend, std::vector<glm::vec3>& triangles){
         std::vector<glm::vec3> b1, b2;
         glm::vec3 dir = glm::normalize(chain.at(1) - chain.at(0));
         computeCircle(dir, chain.at(0), rend, b1);
         computeCircle(dir, chain.at(1), rend, b2);
         
         for(int i(0); i < basePointCount; i++){
            triangles.push_back(chain.at(0));
            triangles.push_back(b1.at((i+1)%basePointCount));
            triangles.push_back(b1.at(i));
         }

         for(int i(0); i < basePointCount; i++){
               triangles.push_back(b1.at((i+1) % basePointCount));
               triangles.push_back(b2.at(i));
               triangles.push_back(b1.at(i));

               triangles.push_back(b2.at((i+1)%basePointCount));
               triangles.push_back(b2.at(i));
               triangles.push_back(b1.at((i+1)%basePointCount));
         }

         cubeFromPoint(chain.at(1), 2*rend, triangles);

         float rstep = (rstart-rend) / (float) chain.size();
         for(int i(1); i < chain.size()-1; i++){
            float r = rend + i*rstep;
            dir = glm::normalize(chain.at(i+1) - chain.at(i));
            computeCircle(dir, chain.at(i), r, b1);
            computeCircle(dir, chain.at(i+1), r, b2);

            for(int j(0); j < basePointCount; j++){
               triangles.push_back(b1.at((j+1) % basePointCount));
               triangles.push_back(b2.at(j));
               triangles.push_back(b1.at(j));

               triangles.push_back(b2.at((j+1)%basePointCount));
               triangles.push_back(b2.at(j));
               triangles.push_back(b1.at((j+1)%basePointCount));
            }

            cubeFromPoint(chain.at(i+1), 1.8*r, triangles);
         }
      }

      void inflateEdge(const glm::vec3& start, const glm::vec3& end, float r, std::vector<glm::vec3>& triangles){
         std::vector<glm::vec3> base(basePointCount);
         float stepAngle = _2PI / (float) basePointCount;

         glm::vec3 edgeDirection = glm::normalize(end-start);
         
         glm::mat3 rotationMatrix = computeRotationMatrix(edgeDirection);
         for(int i(0); i < basePointCount; i++){
            //glm::vec3 b = glm::vec3(0.f, r * cos(stepAngle * i), r * sin(stepAngle * i)) + start;
            glm::vec3 b = rotationMatrix * glm::vec3(0.f, r * cos(stepAngle * i),  r * sin(stepAngle * i)) + start;
            base.at(i) = b;
         }

         for(int i(0); i < basePointCount; i++){
            triangles.push_back(start);
            triangles.push_back(base.at((i+1)%basePointCount));
            triangles.push_back(base.at(i));


         }

         if(pointy){
            for(int i(0); i < basePointCount; i++){
               triangles.push_back(base.at(i));
               triangles.push_back(base.at((i+1)%basePointCount));
               triangles.push_back(end);
            }
         }else{
            std::vector<glm::vec3> endBase(basePointCount);
            for(int i(0); i < basePointCount; i++){
               endBase.at(i) = rotationMatrix * glm::vec3(0.f, r * cos(stepAngle * i), r * sin(stepAngle * i)) + end;
            }
            for(int i(0); i < basePointCount; i++){
               triangles.push_back(endBase.at(i));
               triangles.push_back(endBase.at((i+1)%basePointCount));
               triangles.push_back(end);


            }
            for(int i(0); i < basePointCount; i++){
               triangles.push_back(base.at((i+1) % basePointCount));
               triangles.push_back(endBase.at(i));
               triangles.push_back(base.at(i));

               triangles.push_back(endBase.at((i+1)%basePointCount));
               triangles.push_back(endBase.at(i));
               triangles.push_back(base.at((i+1)%basePointCount));
            }
         }
      }

      void cubeFromPoint(const glm::vec3& center, float side, std::vector<glm::vec3>& cube){
         float hf = side * 0.5f;
         glm::vec3 a = glm::vec3(center - hf*x - hf*y + hf*z);
         glm::vec3 b = glm::vec3(center + hf*x - hf*y + hf*z);
         glm::vec3 c = glm::vec3(center + hf*x - hf*y - hf*z);
         glm::vec3 d = glm::vec3(center - hf*x - hf*y - hf*z);
         glm::vec3 e = glm::vec3(center - hf*x + hf*y + hf*z);
         glm::vec3 f = glm::vec3(center + hf*x + hf*y + hf*z);
         glm::vec3 g = glm::vec3(center + hf*x + hf*y - hf*z);
         glm::vec3 h = glm::vec3(center - hf*x + hf*y - hf*z);


         cube.push_back(a); cube.push_back(d); cube.push_back(b);
         cube.push_back(d); cube.push_back(c); cube.push_back(b);
         cube.push_back(e); cube.push_back(h); cube.push_back(a);
         cube.push_back(a); cube.push_back(h); cube.push_back(d);
         cube.push_back(f); cube.push_back(e); cube.push_back(b);
         cube.push_back(e); cube.push_back(a); cube.push_back(b);
         cube.push_back(f); cube.push_back(g); cube.push_back(h);
         cube.push_back(e); cube.push_back(f); cube.push_back(h);
         cube.push_back(h); cube.push_back(g); cube.push_back(c);
         cube.push_back(h); cube.push_back(c); cube.push_back(d);
         cube.push_back(g); cube.push_back(f); cube.push_back(c);
         cube.push_back(f); cube.push_back(b); cube.push_back(c);
      }
      
};

#endif