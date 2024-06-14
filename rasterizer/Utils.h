#ifndef MAKING_TREES_RASTERIZER_UTILS
#define MAKING_TREES_RASTERIZER_UTILS

#include "SDL2Auxiliary.h"
#include <glm/glm.hpp>

void MakingTrees(SDL2Aux* aux){
   for(int i(aux->getHeight()/3); i < aux->getHeight()*2/3; i++){
      for(int j(aux->getWidth()/3); j < aux->getWidth()*2/3; j++){
         aux->putPixel(i, j, glm::vec3(1.f,1.f,1.f));
      }
   }
}

#endif