#ifndef MAKING_TREES_RASTERIZER_CAMERA
#define MAKING_TREES_RASTERIZER_CAMERA

#include <glm/glm.hpp>
#include "Utils.h"
#include "../core/Utils.h"
#include <vector>
#include <iostream>

struct Pixel{
   glm::ivec2 position;
   int borders; // 4 bits
   bool isBehind;
   float zInverse;
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
         depthBuffer = std::vector<float>(screenHeight*screenWidth);
      }

      Camera(
         int screenHeight, 
         int screenWidth, 
         float focalDistance
      ): 
         screenHeight(screenHeight), 
         screenWidth(screenWidth),
         focalDistance(focalDistance),
         speed(0.5),
         sensitivity(0.01),
         position(glm::vec3(0,0,-300)),
         phi(0),
         theta(0)
      {
         updateRotations();
         depthBuffer = std::vector<float>(screenHeight*screenWidth, 0);
      }

      int screenHeight;
      int screenWidth;

      void moveUpConstrained(){
      }

      void moveDownConstrained(){
      }

      void moveLeftConstrained(){
         rotateRight();
         updateConstrainedPosition();
      }

      void moveRightConstrained(){
         rotateLeft();
         updateConstrainedPosition();
      }

      void updateConstrainedPosition(){
         this->position = sphericalToCarthesian(glm::vec3(constrainedDistanceToOrigin, -theta-PI_2, 0))+50.f*y;
      }

      void moveRight(){
         position += speed * rotationMatrix[0];
      }

      void moveLeft(){
         position -= speed * rotationMatrix[0];
      }

      void moveUp(){
         position += glm::vec3(0,1,0) * speed;
      }

      void moveDown(){
         position -= glm::vec3(0,1,0) * speed;
      }

      void moveFront(){
         position += glm::normalize(glm::cross(rotationMatrix[0], glm::vec3(0,1,0))) * speed;
         
      }

      void moveBack(){
         position -= glm::normalize(glm::cross(rotationMatrix[0], glm::vec3(0,1,0))) * speed;
      }

      void rotateUp(){
         phi +=  sensitivity;
         updateRotations();
      }

      void rotateDown(){
         phi -= sensitivity;
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

      void resetDepthBuffer(){
         std::fill(depthBuffer.begin(), depthBuffer.end(), 0);
      }

      void drawLine(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& color, SDL2Aux* aux){
         Pixel p1, p2;
         std::vector<Pixel> line;
         if(projectLine(v1, v2, p1, p2)){
            interpolate(p2,p1, line);
            
            for(Pixel p : line){
               if(!p.isBehind and isInScreen(p) and  p.zInverse >= depthBuffer.at(p.position.x + p.position.y * screenWidth)){
                  aux->putPixel(p.position.x, p.position.y, color*clipZInverse(p.zInverse*10));
                  depthBuffer.at(p.position.x + p.position.y * screenWidth) = p.zInverse;
               }
            }
         }
      }

      void drawLineBright(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& color, SDL2Aux* aux){
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
            aux->putPixel(p.position.x, p.position.y, color*clipZInverse(p.zInverse*10));
         }
      }
      
      void drawPointBright(const glm::vec3& v, const glm::vec3& color, SDL2Aux* aux){
         Pixel p;
         vertexShader(v, p);
         if(!p.borders and !p.isBehind){
            aux->putPixel(p.position.x, p.position.y, color);
         }
      }

      void drawTriangle(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, SDL2Aux* aux){
         drawTriangle(v1,v2,v3,color1, color2, aux);
      }
      
      void drawTriangle(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3& color1, const glm::vec3& color2, SDL2Aux* aux){
         std::vector<Pixel> polygon;
         if(!projectTriangle(v1, v2, v3, polygon)) return;
         glm::vec3 normal = computeNormal(v1, v2, v3);
         float coef = -glm::dot(glm::normalize(v1-position), normal);
         if(coef < 0) return;
         drawPolygon(polygon, color1*coef + color2*(1-coef), aux);
      }

      void drawTriangles(const std::vector<glm::vec3>& triangles, const glm::vec3& color1, const glm::vec3& color2, SDL2Aux* aux){
         for(int i(0); i < triangles.size(); i+=3){
            if(!isnan(triangles.at(i)) and !isnan(triangles.at(i+1)) and !isnan(triangles.at(i+2)))
               drawTriangle(triangles.at(i), triangles.at(i+1), triangles.at(i+2), color1, color2, aux);
         }		
      }

   private:
      glm::vec3 color1 = red; 
      glm::vec3 color2 = black;

      float focalDistance;

      float speed = 1.f;
      float sensitivity = 1.f;

      float constrainedDistanceToOrigin = 200.f;

      glm::vec3 position;

      std::vector<float> depthBuffer;

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
	      int y((int) focalDistance*(-p.y/p.z)+screenHeight/2);

         pixel.position = glm::ivec2(x,y);
         pixel.zInverse = 1/p.z;
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
               projectOnBorder(p2.position, diff, p2.zInverse, p1, i);
            }
         }

         for(int i(0); i < 4; i++){
            if(p2.borders & (1<<i)){
               diff = p2.position-p1.position;
               projectOnBorder(p1.position, diff, p1.zInverse, p2, i);
            }
         }

         return true;
      }

      bool projectTriangle(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, std::vector<Pixel>& polygon){
         Pixel p1, p2, p3;

         vertexShader(v1, p1);
         vertexShader(v2, p2);
         vertexShader(v3, p3);

         int in((p1.borders == 0) + (p2.borders == 0) + (p3.borders == 0));

         if(!p1.isBehind and !p2.isBehind and !p3.isBehind){
            if(in == 3){ // Base case, three points in
               polygon.push_back(p1);
               polygon.push_back(p2);
               polygon.push_back(p3);
               return true;
            }else if(in == 2){ // In case there is one point out
               if(p1.borders) return projectTriangleOneOut(p2, p3, p1, polygon);
               if(p2.borders) return projectTriangleOneOut(p1, p3, p2, polygon);
               if(p3.borders) return projectTriangleOneOut(p1, p2, p3, polygon);
            }else if(in == 1){ // In case there are two points out
               if(!p1.borders) return projectTriangleTwoOut(p1, p2, p3, polygon);
               if(!p2.borders) return projectTriangleTwoOut(p2, p1, p3, polygon);
               if(!p3.borders) return projectTriangleTwoOut(p3, p1, p2, polygon);
            }else{ // In case the three points are out
               return false;
            }
         }  
      }

      bool projectTriangleOneOut(const Pixel& in1, const Pixel& in2, Pixel& out, std::vector<Pixel>& polygon){
         polygon.push_back(in1);

         //std::cout << "Projecting one out" << std::endl;

         Pixel p1;
         Pixel p2;
         int b1 = projectOnBorder(out, in1, p1);
         int b2 = projectOnBorder(out, in2, p2);
         polygon.push_back(p1);

         if(b1 == b2){ // same border
            polygon.push_back(p2);
            //std::cout << "    With the two projection on the same border " << b1 << std::endl;
         }else{
            if((b1 == 0 and b2 == 1) or (b1 == 1 and b2 == 0)){
               //std::cout << "Corner 0" << std::endl;
               polygon.push_back(findCornerPixel(0, in1, in2, out));
            }else if((b1 == 1 and b2 == 2) or (b2 == 2  and b1 == 1)){
               //std::cout << "Corner 1" << std::endl;
               polygon.push_back(findCornerPixel(1, in1, in2, out));
            }else if((b1 == 2 and b2 == 3) or (b1 == 3 and b2 == 2)){
               //std::cout << "Corner 2" << std::endl;
               polygon.push_back(findCornerPixel(2, in1, in2, out));
            }else if((b1 == 3 and b2 == 0) or (b1 == 0 and b2 == 3)){
               //std::cout << "Corner 3" << std::endl;
               polygon.push_back(findCornerPixel(3, in1, in2, out));
            }
            polygon.push_back(p2);
         }  

         polygon.push_back(in2);
         return true;
      }

      // Project the point p on one of the border of the screen along the vector v = dest-p
      // Returns
      // 0 - if it has been projected to the left border
      // 1 - if it has been projected to the top border 
      // 2 - if it has been projected to the right border
      // 3 - if it has been projected to the bottom border
      // -1 - if the line between p and dest does not cross the screen
      int projectOnBorder(const Pixel& p, const Pixel& dest, Pixel& projection){
         float t(0);
         glm::vec2 diff = glm::vec2(dest.position-p.position);
         int b(0);

         //std::cout << "projecting p : " << p.position << " on dest : " << dest.position << std::endl;

         
            switch(p.borders){
               case 1: // left
                  t = -p.position.x/diff.x;
                  b = 0;
                  break;
               case 2: // right
                  t = (screenWidth-1-p.position.x)/diff.x;
                  b = 2;
                  break;
               case 4: // top
                  t = -p.position.y/diff.y;
                  b = 1;
                  break;
               case 8: // bottom
                  t = (screenHeight-1-p.position.y)/diff.y;
                  b = 3;
                  break;
               case 5: // top left
                  t = -p.position.x/diff.x;
                  b = 0;
                  if(p.position.y + diff.y*t < 0){
                     t = -p.position.y/diff.y;
                     b = 1;
                  }
                  break;
               case 6: // top right
                  t = (screenWidth-1-p.position.x)/diff.x;
                  b = 2;
                  if(p.position.y + diff.y*t < 0){
                     t = -p.position.y/diff.y;
                     b = 1;
                  }
                  break;
               case 10: // bottom right
                  t = (screenWidth-1-p.position.x)/diff.x;
                  b = 2;
                  if(p.position.y + diff.y*t < 0){
                     t = (screenHeight-1-p.position.y)/diff.y;
                     b = 3;
                  }
                  break;
               case 9: // bottom left
                  t = -p.position.x/diff.x;
                  b = 0;
                  if(p.position.y + diff.y*t < 0){
                     t = (screenHeight-1-p.position.y)/diff.y;
                     b = 3;
                  }
                  break;
            }
         

         projection.position = p.position + glm::ivec2(t*diff);
         if(b==0) projection.position.x = 0;
         if(b==1) projection.position.y = 0;
         if(b==2) projection.position.x = screenWidth-1;
         if(b==3) projection.position.y = screenHeight-1;
         //std::cout << "projection : " << projection.position << std::endl;
         projection.zInverse = p.zInverse + t*(dest.zInverse - p.zInverse);
         setBorders(projection);
         if(projection.borders) return -1;
         else return b;
      }

      // return a pixel with the good value for a given triangle at a given corner of the screen
      // 0 - top left
      // 1 - top right
      // 2 - bottom right
      // 3 - bottom left
      Pixel findCornerPixel(int corner, const Pixel& p1, const Pixel& p2, const Pixel& p3){

         //std::cout << "finding corner : " << corner << std::endl;
         glm::vec2 u = glm::vec2(p2.position - p1.position);
         glm::vec2 v = glm::vec2(p3.position - p1.position);
         Pixel main = p1;
         Pixel m1 = p2;
         Pixel m2 = p3;

         if(u.x == 0){
            u = glm::vec2(p2.position - p3.position);
            v = glm::vec2(p1.position - p3.position);
            main = p3;
            m1 = p2;
            m2 = p1;
         }

         if(v.x == 0){
            u = glm::vec2(p1.position - p2.position);
            v = glm::vec2(p3.position - p2.position);
            main = p2;
            m1 = p1;
            m2 = p3;
         }

         float t1 = ((corner%3 == 0 ? 0.f : (float) screenWidth) - main.position.x)/u.x;
         float t2 = ((corner%3 == 0 ? 0.f : (float) screenWidth) - main.position.x)/v.x;
         
         Pixel p4 = interpolate(main, m1, t1);
         Pixel p5 = interpolate(main, m2, t2);

         // interpolate between p4 and p5

         //std::cout << "p4 : " << p4.position << " p5 : " << p5.position << std::endl;

         float t3 = ((corner <= 1 ? 0 : screenHeight) - p4.position.y)/((float) p5.position.y-p4.position.y);
         //std::cout << "t3 : " << t3 << std::endl;
         Pixel p = interpolate(p4, p5, t3);
         //std::cout << "corner : " << p.position << std::endl;
         return p;
      }

      bool projectTriangleTwoOut(const Pixel& in, Pixel& o1, Pixel& o2, std::vector<Pixel>& polygon){
         //std::cout << "Projecting two out" << std::endl;
         Pixel p1, p2;
         int b1 = projectOnBorder(o1, in, p1);
         int b2 = projectOnBorder(o2, in, p2);

         polygon.push_back(in);
         polygon.push_back(p1);
         
         if(b1 == b2){ // case 0 : two with same border
            polygon.push_back(p2);
            //std::cout << "    with both on the same border : " << b1 << std::endl;
         }else if((b1+1)%3 == b2%3 or (b1-1)%3 == b2%3){
            Pixel p4, p5;
            int b4 = projectOnBorder(o1, o2, p4);
            //std::cout << "b4 : " << b4 << std::endl;
            if(b4 != -1){
               polygon.push_back(p4);
               projectOnBorder(o2, o1, p5);
               polygon.push_back(p5);
               //std::cout << "    with consecutive crossing borders b1 : " << b1 << " b2 : " << b2 << std::endl;
            }else{
               if((o1.borders | o2.borders) == 15){
                  //std::cout << "    with consecutive reverse borders o1 : " << o1.borders << " o2 : " << o2.borders << std::endl;
                  int other = getCornerFromConsecutiveBorders(b1, b2);
                  polygon.push_back(findCornerPixel((other+1)%3, in, o1, o2));
                  polygon.push_back(findCornerPixel((other+2)%3, in, o1, o2));
                  polygon.push_back(findCornerPixel((other+3)%3, in, o1, o2));
               }else{
                  //std::cout << "    with consecutive normal borders b1 : " << b1 << " b2 : " << b2 << std::endl; 
                  polygon.push_back(findCornerPixel(getCornerFromConsecutiveBorders(b1,b2), in, o1, o2));
               }
            }
            polygon.push_back(p2);
         }else if((b1+2)%3 == b2%3 or b1%3 == (b2+2)%3){
            //std::cout << "    with opposite borders b1 : " << b1 << " b2 : " << b2 << std::endl; 
            Pixel p4, p5;
            int b4 = projectOnBorder(o1, o2, p4);
            if(b4 == -1){
               // Add the two corners
               if((b1 == 0 and b2 == 2) or (b1 == 2 and b2 == 0)){ // Horizontal
                  glm::vec2 t = glm::inverse(glm::mat2(o1.position-in.position, o2.position-in.position))*glm::vec2(0, 0);
                  if(t.x > 0 and t.y > 0){ // top side
                     polygon.push_back(findCornerPixel(0, in, o1, o2));
                     polygon.push_back(findCornerPixel(1, in, o1, o2));
                  }else{
                     polygon.push_back(findCornerPixel(2, in, o1, o2));
                     polygon.push_back(findCornerPixel(3, in, o1, o2));
                  }
               }else{ // Vertical
                  glm::vec2 t = glm::inverse(glm::mat2(o1.position-in.position, o2.position-in.position))*glm::vec2(0, screenWidth);
                  if(t.x > 0 and t.y > 0){ // top side
                     polygon.push_back(findCornerPixel(1, in, o1, o2));
                     polygon.push_back(findCornerPixel(2, in, o1, o2));
                  }else{
                     polygon.push_back(findCornerPixel(3, in, o1, o2));
                     polygon.push_back(findCornerPixel(0, in, o1, o2));
                  }
               }
            }else{
               // either one or no corners
               if(b4 != b1){
                  polygon.push_back(findCornerPixel(getCornerFromConsecutiveBorders(b1, b4), in, o1, o2));
               }
               polygon.push_back(p4);
               int b5 = projectOnBorder(o2, o1, p5);
               polygon.push_back(p5);
               if(b5 != b2){
                  polygon.push_back(findCornerPixel(getCornerFromConsecutiveBorders(b2, b5), in, o1, o2));
               }
            }
            polygon.push_back(p2);
         }
         // case 1 : two with same corner
         // case 2 : two with consecutive border and corner
         // case 3 : two with consecutive border but no corner 
         // case 4 : two with consectutive corners
         // case 5 : two with opposite borders / corners

         return true;
      }
      
      int getCornerFromConsecutiveBorders(int b1, int b2){
         if((b1 == 0 and b2 == 1) or (b1 == 1 and b2 == 0)) return 0;
         if((b1 == 1 and b2 == 2) or (b1 == 2 and b2 == 1)) return 1; 
         if((b1 == 2 and b2 == 3) or (b1 == 3 and b2 == 2)) return 2; 
         if((b1 == 3 and b2 == 0) or (b1 == 0 and b2 == 3)) return 3; 
         return -1;
      }

      int getCornerFromBorders(int borders){
         if(borders == 5) return 0;
         if(borders == 6) return 1;
         if(borders == 10) return 2;
         if(borders == 9) return 3;
      }

      void projectOnBorder(const glm::ivec2& p0, const glm::vec2& diff, float zInverseP0, Pixel& p1, int border){
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
            p1.zInverse = zInverseP0 + (p1.zInverse-zInverseP0)*t;
         }
      }

      void setBorders(Pixel& p){
         p.borders = 0;

         if(p.position.x < 0){
            p.borders |= 1;
         }else if(p.position.x >= screenWidth){
            p.borders |= 2;
         }

         if(p.position.y < 0){
            p.borders |= 4;
         }else if(p.position.y >= screenHeight){
            p.borders |= 8;
         }
      }

      bool isInScreen(const Pixel& p){
         return p.position.x >= 0 and p.position.x < screenWidth and p.position.y >= 0 and p.position.y < screenHeight;
      }

      void interpolate(const Pixel& p1, const Pixel& p2, std::vector<Pixel>& line){
         glm::ivec2 diff = p2.position-p1.position;
         int n(glm::max(glm::abs(diff.x), glm::abs(diff.y)) + 1);
         line = std::vector<Pixel>(n);
	      glm::vec2 step = glm::vec2(diff) / float(std::max(n-1, 1));
         float zInverseStep = (p2.zInverse - p1.zInverse) / float(std::max(n-1, 1));
	      glm::vec2 current(p1.position);
         float currentZInverse = p1.zInverse;
	      for(int i = 0; i < n-1; i++){
		      line[i].position = current;
            line[i].zInverse = currentZInverse;
            currentZInverse += zInverseStep;
		      current += step;
	      }
         line.at(n-1) = p2;
      }

      Pixel interpolate(const Pixel& p1, const Pixel& p2, float t){
         Pixel p;
         glm::ivec2 diff(p2.position-p1.position);
         p.position = glm::ivec2(p1.position.x + diff.x * t, p1.position.y + diff.y * t);
         p.zInverse = p1.zInverse + (p2.zInverse-p1.zInverse)*t;
         p.isBehind = p.zInverse < 0;
         return p;
      }

      float clipZInverse(float zInv){
         if(zInv < 0) return 0;
         if(zInv > 1) return 1;
         return zInv;
      }

      void drawPolygon(const std::vector<Pixel>& polygon, const glm::vec3& color, SDL2Aux* aux){
         int h(-1), l(screenHeight+1);
         
         //std::cout << polygon.size() << std::endl;

         for(const auto& p : polygon){
            //std::cout << p.position << std::endl;
            if(p.position.y > h){
               h = p.position.y;
            }
            if(p.position.y < l){
               l = p.position.y;
            }
         }


         //std::cout << "h : " << h << " l : " << l << std::endl;

         std::vector<Pixel> right(h-l+1); std::vector<Pixel> left(h-l+1);
         //std::cout << "allocated left and right " << h-l +1 << std::endl;

         for(int i(0); i < right.size(); i++){
		      left.at(i) = Pixel{glm::ivec2(std::numeric_limits<int>::max(), l + i)};
		      right.at(i) = Pixel{glm::ivec2(std::numeric_limits<int>::min(), l + i)};
	      }

         std::vector<Pixel> edge;
         for(int i(0); i < polygon.size(); i++){
            interpolate(polygon.at(i), polygon.at((i+1)%polygon.size()), edge);
            for(const auto& pixel : edge){
               int y(pixel.position.y - l);
               if(pixel.position.x < left.at(y).position.x){
                  left.at(y) = pixel;
               }
               if(pixel.position.x > right.at(y).position.x){
                  right.at(y) = pixel;
               }
            }
         }
         // std::cout << "populated left and right" << std::endl;

         std::vector<Pixel> line;
         for(int i(0); i < right.size(); i++){
            interpolate(left.at(i), right.at(i), line);
            //std::cout<< left.at(i).position << std::endl;

            for(auto& p : line){
               setBorders(p);

               if(!p.borders and p.zInverse > depthBuffer.at(p.position.x + p.position.y * screenWidth)){
                  aux->putPixel(p.position.x, p.position.y, color);
                  depthBuffer.at(p.position.x + p.position.y * screenWidth) = p.zInverse;
               }
            }
            // std::cout<< "ok" << std::endl;
         }
      }
};

#endif