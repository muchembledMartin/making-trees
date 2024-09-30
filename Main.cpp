#include <glm/glm.hpp>
#include "SDL2Auxiliary.h"

#include <array>
#include "rasterizer/Utils.h"
#include "rasterizer/Camera.h"
#include "core/Grid.h"
#include "core/Graph.h"
#include "core/Tree.h"
#include "inflator/Inflator.h"

// ------------------------ PARAMETERS --------------------------

int colors[10] = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF, 0xFFFFFF};

// --------------------------------------------------------------

int SCREEN_HEIGHT = 1000;
int SCREEN_WIDTH = 1000;

float t;

void update(void);
void draw(void);

void printElapsedTime();

void setGuidingVectorComputer();
glm::vec3 intToColor(int i);

SDL2Aux* aux;
std::vector<std::vector<glm::vec3>> treeMeshes;
Camera camera(SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_WIDTH);

std::array<glm::vec3, 10> colorsVec;


int stepsToShow;


// CONTROL STATE

int mainColor;
bool isNumpadEnabled = true;
bool monochromatic = false; // if false, show the generations steps in different colors
bool showTimeBetweenFrame = false; 
float monochromaticDebouncer;
float stepToShowDebouncer;
float numpadEnableDebouncer;

int main( int argc, char* argv[] ){

   aux = new SDL2Aux(SCREEN_WIDTH, SCREEN_HEIGHT, false);

   Tree tree = Tree();
   tree.initializeGraph();
   camera.moveRightConstrained();

   tree.setDefaultConfig();
   // tree.setExample2Config();
   // tree.setExample3Config();
   // tree.setExample4Config();

   tree.generateTree(treeMeshes);

   for(int i(0); i < 10; i++){
      colorsVec.at(i) = intToColor(colors[i]);
   }

   while (!aux->quitEvent())
	{
	 	update();
		draw();
	}
	aux->saveBMP("screenshot.bmp");
	return 0;
}


void update(){

   if(showTimeBetweenFrame){
      printElapsedTime();
   }

   const Uint8* keystate = SDL_GetKeyboardState(NULL);
   if(keystate[SDL_SCANCODE_LEFT]){
      camera.moveLeftConstrained();
   }
   if(keystate[SDL_SCANCODE_RIGHT]){
      camera.moveRightConstrained();
   }
   if(keystate[SDL_SCANCODE_UP]){
      if(SDL_GetTicks() - stepToShowDebouncer > 100 and stepsToShow < treeMeshes.size()){
         stepsToShow++;
         stepToShowDebouncer = SDL_GetTicks();
      } 
   }
   if(keystate[SDL_SCANCODE_DOWN]){
      if(SDL_GetTicks() - stepToShowDebouncer > 100 and stepsToShow > 0){
         stepsToShow--;
         stepToShowDebouncer = SDL_GetTicks();
      }
   }
   if(keystate[SDL_SCANCODE_C]){
      if(SDL_GetTicks() - monochromaticDebouncer > 100){
         monochromatic = !monochromatic;
         monochromaticDebouncer = SDL_GetTicks();
      }
   }
   if(keystate[SDL_SCANCODE_N]){
      if(SDL_GetTicks() - numpadEnableDebouncer > 100){
         isNumpadEnabled = !isNumpadEnabled;
         numpadEnableDebouncer = SDL_GetTicks();
      }
   }
   if(keystate[SDL_SCANCODE_F]){
      showTimeBetweenFrame = !showTimeBetweenFrame;
   }
   if((keystate[SDL_SCANCODE_KP_0] and isNumpadEnabled) or (keystate[SDL_SCANCODE_0] and !isNumpadEnabled)){
      mainColor = 0;
   }
   if((keystate[SDL_SCANCODE_KP_1] and isNumpadEnabled) or (keystate[SDL_SCANCODE_1] and !isNumpadEnabled)){
      mainColor = 1;
   }
   if((keystate[SDL_SCANCODE_KP_2] and isNumpadEnabled) or (keystate[SDL_SCANCODE_2] and !isNumpadEnabled)){
      mainColor = 2;
   }
   if((keystate[SDL_SCANCODE_KP_3] and isNumpadEnabled) or (keystate[SDL_SCANCODE_3] and !isNumpadEnabled)){
      mainColor = 3;
   }
   if((keystate[SDL_SCANCODE_KP_4] and isNumpadEnabled) or (keystate[SDL_SCANCODE_4] and !isNumpadEnabled)){
      mainColor = 4;
   }
   if((keystate[SDL_SCANCODE_KP_5] and isNumpadEnabled) or (keystate[SDL_SCANCODE_5] and !isNumpadEnabled)){
      mainColor = 5;
   }
   if((keystate[SDL_SCANCODE_KP_6] and isNumpadEnabled) or (keystate[SDL_SCANCODE_6] and !isNumpadEnabled)){
      mainColor = 6;
   }
   if((keystate[SDL_SCANCODE_KP_7] and isNumpadEnabled) or (keystate[SDL_SCANCODE_7] and !isNumpadEnabled)){
      mainColor = 7;
   }
   if((keystate[SDL_SCANCODE_KP_8] and isNumpadEnabled) or (keystate[SDL_SCANCODE_8] and !isNumpadEnabled)){
      mainColor = 8;
   }
   if((keystate[SDL_SCANCODE_KP_9] and isNumpadEnabled) or (keystate[SDL_SCANCODE_9] and !isNumpadEnabled)){
      mainColor = 9;
   }

}

void draw(){

   aux->clearPixels();
   camera.resetDepthBuffer();

   for(int i(0); i < stepsToShow; i++){
      camera.drawTriangles(treeMeshes.at(i), monochromatic ? colorsVec.at(mainColor) : colorsVec.at(i), black, aux);
   }

   aux->render();
}


void printElapsedTime(){
   float t2 = SDL_GetTicks();
   std::cout << "Render time : " <<  t2-t << " ms" << std::endl;
}

glm::vec3 intToColor(int i){
   return glm::vec3((i & (255) << 16) >> 16, (i & (255) << 8) >> 8, i & 255) / 255.f;  
}

