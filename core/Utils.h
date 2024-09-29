#ifndef MAKING_TREE_CORE_UTILS
#define MAKING_TREE_CORE_UTILS

#include <glm/glm.hpp>
#include <random>
#include <cmath>
#include <../rasterizer/Utils.h>

#define SEED 14092003


glm::vec3 random3DVec(float minX, float minY, float minZ, float rangeX, float rangeY, float rangeZ){
   static std::default_random_engine e(SEED);
   static std::uniform_real_distribution<> dis(0, 1); // range [0, 1)

   return glm::vec3(minX + dis(e) * rangeX, minY + dis(e) * rangeY, minZ + dis(e)*rangeZ);
}

int randomInt(int min, int max){
   static std::default_random_engine e;
   std::uniform_int_distribution<> intDis = std::uniform_int_distribution<>(min, max); // range [0, 1)

   return intDis(e);
}

glm::vec3 sampleFromAnulus(const glm::vec3& center){
   glm::vec3 sphericalCoords(random3DVec(1.732f, 0.f, -PI_2, 1.732f, _2PI, PI));
   return glm::vec3(
      sphericalCoords.x * cos(sphericalCoords.y) * cos(sphericalCoords.z) + center.x,
      sphericalCoords.x * sin(sphericalCoords.y) * cos(sphericalCoords.z) + center.y,
      sphericalCoords.x * sin(sphericalCoords.z) + center.z
   );
}

float squaredDistance(const glm::vec3& p1, const glm::vec3& p2){
   glm::vec3 diff = p1-p2;
   return glm::dot(diff, diff);
}

glm::vec3 carthesianToSpherical(const glm::vec3& carthesian){
   float r = std::sqrt(glm::dot(carthesian, carthesian));
   return glm::vec3(r, (carthesian.x == 0 and carthesian.z == 0) ? 0 : std::atan2(carthesian.z,carthesian.x), std::asin(carthesian.y / r));
}

glm::vec3 sphericalToCarthesian(const glm::vec3& spherical){
   return glm::vec3(
      spherical.x*std::cos(spherical.y)*std::cos(spherical.z), 
      spherical.x*std::sin(spherical.z), 
      spherical.x*std::sin(spherical.y)*std::cos(spherical.z)
   );
}

bool isnan(glm::vec3 v){
   return isnan(v.x) or isnan(v.y) or isnan(v.z);
}

glm::vec3 x = glm::vec3(1,0,0);
glm::vec3 y = glm::vec3(0,1,0);
glm::vec3 z = glm::vec3(0,0,1);


float NaN = std::nan("");
#endif