#ifndef MAKING_TREES_RASTERIZER_UTILS
#define MAKING_TREES_RASTERIZER_UTILS

#include "SDL2Auxiliary.h"
#include <glm/glm.hpp>
#include <vector>
#include <ostream>

const float PI = 3.141592653589f;
const float PI_2 = 1.570796327f;
const float _2PI = 6.283185307f;

const glm::vec3 a(-0.5f, -0.5f, -0.5f);
const glm::vec3 b( 0.5f, -0.5f, -0.5f);
const glm::vec3 c( 0.5f, -0.5f,  0.5f);
const glm::vec3 d(-0.5f, -0.5f,  0.5f);
const glm::vec3 e(-0.5f,  0.5f, -0.5f);
const glm::vec3 f( 0.5f,  0.5f, -0.5f);
const glm::vec3 g( 0.5f,  0.5f,  0.5f);
const glm::vec3 h(-0.5f,  0.5f,  0.5f);

void MakingTrees(SDL2Aux* aux){
   for(int i(aux->getHeight()/3); i < aux->getHeight()*2/3; i++){
      for(int j(aux->getWidth()/3); j < aux->getWidth()*2/3; j++){
         aux->putPixel(i, j, glm::vec3(1.f,1.f,1.f));
      }
   }
}

void Cube(std::vector<glm::vec3>& vs){

   vs = std::vector<glm::vec3>({
      a,b,
      b,c,
      c,d,
      d,a,
      e,f,
      f,g,
      g,h,
      h,e,
      a,e,
      b,f,
      c,g,
      d,h,
   });

}

const glm::vec3 white(1.f,1.f,1.f);
const glm::vec3 red(1.f,0.f,0.f);
const glm::vec3 blue(0.f,0.f,1.f);
const glm::vec3 green(0.f,1.f,0.f);
const glm::vec3 cyan(0.f,1.f,1.f);
const glm::vec3 yellow(1.f,1.f,0.f);
const glm::vec3 magenta(1.f,0.f,1.f);

std::ostream& operator<<(std::ostream& stream, glm::vec3 v){
   return stream << "(" << v.x << ", " << v.y << ", " << v.z << ") ";
}

std::ostream& operator<<(std::ostream& stream, glm::ivec2 iv2){
   return stream << "(" << iv2.x << ", " << iv2.y << ") ";
}

#endif