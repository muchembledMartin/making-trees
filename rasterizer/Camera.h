#ifndef MAKING_TREES_RASTERIZER_CAMERA
#define MAKING_TREES_RASTERIZER_CAMERA

#include <glm/glm.hpp>
#include "Utils.h"
#include <vector>
#include <iostream>

struct Pixel{
   glm::ivec2 position;
   int borders; // 4 bits
   bool isBehind;
};

class Camera{

   public:

      Camera(
         int screenHeight, 
         int screenWidth, 
         float focalDistance,
         float speed, 
         float sensitivity, 
         glm::vec3 position,
         float phi, 
         float theta
      ): 
         screenHeight(screenHeight), 
         screenWidth(screenWidth),
         focalDistance(focalDistance),
         speed(speed),
         sensitivity(sensitivity),
         position(position),
         phi(phi),
         theta(theta)
      {
         updateRotations();
      }

      Camera(
         int screenHeight, 
         int screenWidth, 
         float focalDistance
      ): 
         screenHeight(screenHeight), 
         screenWidth(screenWidth),
         focalDistance(focalDistance),
         speed(0.05),
         sensitivity(0.01),
         position(glm::vec3()),
         phi(0),
         theta(0)
      {
         updateRotations();
      }

      int screenHeight;
      int screenWidth;

      void moveRight(){
         position += speed * rotationMatrix[0];
      }

      void moveLeft(){
         position -= speed * rotationMatrix[0];
      }

      void moveUp(){
         position -= glm::vec3(0,1,0) * speed;
      }

      void moveDown(){
         position += glm::vec3(0,1,0) * speed;
      }

      void moveFront(){
         position += glm::normalize(glm::cross(rotationMatrix[0], glm::vec3(0,1,0))) * speed;
      }

      void moveBack(){
         position -= glm::normalize(glm::cross(rotationMatrix[0], glm::vec3(0,1,0))) * speed;
      }

      void rotateUp(){
         phi -=  sensitivity;
         updateRotations();
      }

      void rotateDown(){
         phi += sensitivity;
         updateRotations();
      } 

      void rotateLeft(){
         theta -= sensitivity;
         updateRotations();
      }

      void rotateRight(){
         theta += sensitivity;
         updateRotations();
      }

      void drawLine(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& color, SDL2Aux* aux){
         Pixel p1, p2;
         std::vector<Pixel> line;
         if(projectLine(v1, v2, p1, p2)){
            interpolate(p2,p1, line);
            
            for(Pixel p : line){
               aux->putPixel(p.position.x, p.position.y, color);
            }
         }
      }
      
      void drawPoint(const glm::vec3& v, const glm::vec3& color, SDL2Aux* aux){
         Pixel p;
         vertexShader(v, p);
         if(!p.borders and !p.isBehind){
            aux->putPixel(p.position.x, p.position.y, color);
         }
      }

   private:

      float focalDistance;

      float speed = 1.f;
      float sensitivity = 1.f;

      glm::vec3 position;

      float phi; // rotation around x
      float theta; // rotation around y

      glm::mat3 rotationMatrix;
      glm::mat3 invRotationMatrix;

      void updateRotations(){
         if(phi < -PI_2) phi = -PI_2;
         if(phi > PI_2) phi = PI_2;
         if(theta > _2PI) theta -= _2PI;
         if(theta < 0) theta += _2PI;

         float sinX = sin(phi);
         float cosX = cos(phi);
         float sinY = sin(theta);
         float cosY = cos(theta);
         
         rotationMatrix = glm::mat3(glm::vec3(cosY, 0, -sinY), glm::vec3(0,1,0), glm::vec3(sinY, 0 ,cosY));
         rotationMatrix *= glm::mat3(glm::vec3(1, 0, 0), glm::vec3(0,cosX,sinX), glm::vec3(0, -sinX, cosX));
         invRotationMatrix = glm::inverse(rotationMatrix);
      }

      void vertexShader(const glm::vec3& v, Pixel& pixel){
         glm::vec3 p(invRotationMatrix * (v-position));

         if(p.z < 0.0001 and p.z > -0.0001){
            p.z = p.z < 0 ? -0.0001 : 0.0001;
         }
         if(p.z < 0 or isnan(p.z)){
            pixel.isBehind = true;
         }else{
            pixel.isBehind = false;
         }

         p.z = p.z < 0 ? -p.z : p.z;
	      int x((int) focalDistance*(p.x/p.z)+screenWidth/2);
	      int y((int) focalDistance*(p.y/p.z)+screenHeight/2);

         pixel.position = glm::ivec2(x,y);
         setBorders(pixel);
      }

      bool projectLine(const glm::vec3& v1, const glm::vec3& v2, Pixel& p1, Pixel& p2){
         vertexShader(v1, p1);
         vertexShader(v2, p2);  

         if((p1.isBehind and p2.isBehind) or (p1.borders and p2.borders and (p1.borders & p2.borders))){
            return false;
         }
         if(!p1.borders and !p2.borders) return true;


         glm::vec2 diff;

         for(int i(0); i < 4; i++){
            if(p1.borders & (1<<i)){
               diff = p1.position-p2.position;
               projectOnBorder(p2.position, diff, p1, i);
            }
         }

         for(int i(0); i < 4; i++){
            if(p2.borders & (1<<i)){
               diff = p2.position-p1.position;
               projectOnBorder(p1.position, diff, p2, i);
            }
         }

         return true;
      }

      void projectOnBorder(const glm::ivec2& p0, const glm::vec2& diff, Pixel& p1, int border){
         float t(0);
         switch (border)
         {
            case 0:
               t = -p0.x/diff.x;
               break;
            case 1:
               t = (screenWidth - p0.x)/diff.x;
               break;
            case 2:
               t = -p0.y/diff.y;
               break;
            case 3:
               t = (screenHeight - p0.y)/diff.y;
               break;
            default:
               break;
         }

         if(t > 0 and t < 1){
            p1.position = p0 + glm::ivec2(t*diff.x, t*diff.y);
         }
      }

      void setBorders(Pixel& p){
         p.borders = 0;

         if(p.position.x < 0){
            p.borders |= 1;
         }else if(p.position.x > screenWidth){
            p.borders |= 2;
         }

         if(p.position.y < 0){
            p.borders |= 4;
         }else if(p.position.y > screenHeight){
            p.borders |= 8;
         }
      }

      void interpolate(const Pixel& p1, const Pixel& p2, std::vector<Pixel>& line){
         glm::ivec2 diff = p2.position-p1.position;
         int n(glm::max(glm::abs(diff.x), glm::abs(diff.y)) + 1);
         line = std::vector<Pixel>(n);
	      glm::vec2 step = glm::vec2(diff) / float(std::max(n-1, 1));
	      glm::vec2 current(p1.position);
	      for(int i = 0; i < n; i++){
		      line[i].position = current;
		      current += step;
	      }
      }

};

#endif