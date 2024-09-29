#include <glm/glm.hpp>
#include "SDL2Auxiliary.h"

#include "rasterizer/Utils.h"
#include "rasterizer/Camera.h"
#include "core/Grid.h"
#include "core/Graph.h"
#include "core/Tree.h"
#include "inflator/Inflator.h"

void Update(void);
void Draw(void);

int SCREEN_HEIGHT = 1000;
int SCREEN_WIDTH = 1000;
float t;

Camera camera(SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_WIDTH);

SDL2Aux* sdlAux;

glm::vec3 v1(-1.f,-1.f,-1.f);
glm::vec3 v2(1.f,1.f,1);

std::vector<glm::vec3> edges(24);
std::vector<glm::vec3> path;
std::vector<glm::vec3> branches;
std::vector<int> border;
std::vector<glm::vec3> inflated;
std::vector<std::vector<glm::vec3>> treeMeshes;


Graph graph;
Tree tree;

int main( int argc, char* argv[] )
{

	sdlAux = new SDL2Aux(SCREEN_WIDTH, SCREEN_HEIGHT);

	Cube(edges);
	Inflator inflator(5);

	//std::vector<glm::vec3> chain = {glm::vec3(0,0,0), glm::vec3(0,1,0), glm::vec3(0,1.5,0.5), glm::vec3(0,2.5,0.5)};
	//inflator.inflateChain(chain, 0.2f, 0.f, inflated);


	// t = SDL_GetTicks();
	// graph.generateGraph();
	// int t2 = SDL_GetTicks();
	// float dt = float(t2-t);
	// t = t2;
	// std::cout << "Generation time: " << dt << " ms." << std::endl;

	// std::cout << "populated graph with " << graph.grid.getElementCount() << " nodes" << std::endl;
	
	// glm::vec3 rootVector;
	// glm::vec3 endVector;
	// std::cout << "Root vector found ? " << graph.grid.at(0,0,0, rootVector) << std::endl;
	// //std::cout << "End vector found at (0,63,0) ?  " << graph.grid.at(0,W-1,0, endVector) << std::endl;
	// graph.setComputeGuidingVector([](Node& node, const Node& parent, const Grid& grid)-> void {
	// 	node.guidingVector = glm::vec3(0,1,0);
	// });
	// //graph.setComputeEdgeWeight([](const Node& src, int dest, float squaredLength, const Grid& grid) -> float {
	// //	return std::sqrt(squaredLength);
	// //});






	// graph.setComputeEdgeWeight(tree.computeEdgeWeightDefault);
	// graph.setComputeGuidingVector(tree.computeGuidingVectorsDefault(0.1f));

	// std::vector<int> ng;
	// graph.grid.getNeighboorhood(graph.grid.indexFromCell(-20,64,-30), 7, ng);
	// std::vector<int> dests;
	// for(int i(0); i < ng.size(); i++){
	//  	if(graph.grid.fromIndex(ng.at(i), rootVector)){
	//  		dests.push_back(ng.at(i));
	//  	}
	//  	if(dests.size() > 5) break;
	// }
	// ng.clear();
	// graph.grid.getNeighboorhood(graph.grid.indexFromCell(20,48,15), 7, ng);
	// for(int i(0); i < ng.size(); i++){
	//  	if(graph.grid.fromIndex(ng.at(i), rootVector)){
	//  		dests.push_back(ng.at(i));
	//  	}
	//  	if(dests.size() > 10) break;
	// }

	// std::cout << "Found " << dests.size() << " destinations" << std::endl;

	// std::cout << "Computing shortest path ..." << std::endl;
	// t = SDL_GetTicks();
	// graph.shortestPath(std::vector<int>({graph.grid.indexFromCell(0,0,0)}), {graph.grid.indexFromCell(0,W-1,0)});
	// t2 = SDL_GetTicks();
	// dt = float(t2-t);
	// std::cout << "... Done in "<< dt << " ms" << std::endl;
	
	// std::vector<int> pathNodes, pathEdges;
	// graph.reconstructPath(dests, std::vector<int>({graph.grid.indexFromCell(0,0,0)}), pathNodes, pathEdges);
	// std::cout << "Path reconstructed" << std::endl;

	// std::cout << "Finding borders" << std::endl;
	// t = SDL_GetTicks();
	// graph.findBorders(3, pathNodes, border, 0.8f);
	// t2 = SDL_GetTicks();
	// dt = float(t2-t);
	// std::cout << "Found borders in " << dt << "ms" << std::endl;

	// dests.clear();
	// for(int i(0); i < border.size(); i+=5){
	// 	dests.push_back(border.at(i));
	// }

	// glm::vec3 v;
	// for(auto node : pathEdges){
	// 	graph.grid.fromIndex(node, v);
	// 	path.push_back(v);
	// }

	// for(int i(0); i < path.size(); i+=2){
	// 	inflator.inflateEdge(path.at(i), path.at(i+1), (float) 2, inflated);
	// }

	// std::cout << inflated.size() << std::endl;





	// std::vector<int> newPathNode;
	// pathEdges.clear();
	// graph.resetGraph();
	// graph.shortestPath(pathNodes, dests);
	// graph.reconstructPath(dests, pathNodes, newPathNode, pathEdges);

	// for(auto node : pathEdges){
	// 	graph.grid.fromIndex(node, v);
	// 	branches.push_back(v);
	// }

	tree.initializeGraph();
	tree.generateStructure(treeMeshes);
	//tree.generate3DMeshes(inflated);
	std::cout << "3d meshes generated : " << inflated.size() << " trianlges" << std::endl;
	// std::cout << tree.firstLim << std::endl;

	while (!sdlAux->quitEvent())
	{
	 	Update();
		Draw();
	}
	sdlAux->saveBMP("screenshot.bmp");
	return 0;
}

void inflatorTest(){
	Inflator inflator;
	std::cout << "About to inflate" << std::endl;


	for(int i(0); i < edges.size(); i+=2){
	 	inflator.inflateEdge(edges.at(i), edges.at(i+1), 0.2f, inflated);
	}

	inflator.inflateEdge(glm::vec3(-1,0,0), glm::vec3(-2,0,0), 0.4f, inflated);
	inflator.inflateEdge(glm::vec3(1,0,0), glm::vec3(2,0,0), 0.4f, inflated);

	inflator.inflateEdge(glm::vec3(0,-1,0), glm::vec3(0,-2,0), 0.4f, inflated);
	inflator.inflateEdge(glm::vec3(0,1,0), glm::vec3(0,2,0), 0.4f, inflated);

	inflator.inflateEdge(glm::vec3(0,0,-1), glm::vec3(0,0,-2), 0.4f, inflated);
	inflator.inflateEdge(glm::vec3(0,0,1), glm::vec3(0,0,2), 0.4f, inflated);

	glm::vec3 x = glm::vec3(1,0,0);
	glm::vec3 y = glm::vec3(0,1,0);
	glm::vec3 z = glm::vec3(0,0,1);

	glm::vec3 xs = carthesianToSpherical(-x);
	glm::vec3 ys = carthesianToSpherical(-y);
	glm::vec3 zs = carthesianToSpherical(-z);

	std::cout << "Test carthesian to spherical : " << std::endl;
	std::cout << "		" << x << " -> " << carthesianToSpherical(x) << " " << sphericalToCarthesian(carthesianToSpherical(x)) << std::endl;
	std::cout << "		" << y << " -> " << carthesianToSpherical(y) << " " << sphericalToCarthesian(carthesianToSpherical(y)) << std::endl;
	std::cout << "		" << z << " -> " << carthesianToSpherical(z) << " " << sphericalToCarthesian(carthesianToSpherical(z)) << std::endl;

	float cosT = std::cos(xs.y);
	float sinT = std::sin(xs.y);
	float cosP = std::cos(xs.z);
	float sinP = std::sin(xs.z);

	glm::mat3 rotationMatrix = glm::mat3(glm::vec3(cosT,0,sinT), glm::vec3(0,1,0), glm::vec3(-sinT, 0, cosT)) 
		* glm::mat3(glm::vec3(cosP, sinP, 0), glm::vec3(-sinP, cosP, 0), glm::vec3(0,0,1));

	std::cout << "rotated x " << rotationMatrix*x << " rotated y " << rotationMatrix*y << " rotated z " << rotationMatrix*z << std::endl;

	std::cout << "Inflated" << std::endl;
}

void Update(void)
{

	int t2 = SDL_GetTicks();
	float dt = float(t2-t);
	t = t2;
	//std::cout << "Render time: " << dt << " ms." << std::endl;

	const Uint8* keystate = SDL_GetKeyboardState(NULL);
	if (keystate[SDL_SCANCODE_UP]) {
		camera.rotateDown();
	}
	if (keystate[SDL_SCANCODE_DOWN]) {
		camera.rotateUp();
	}
	if (keystate[SDL_SCANCODE_LEFT]) {
		camera.rotateLeft();
	}
	if (keystate[SDL_SCANCODE_RIGHT]) {
		camera.rotateRight();
	}
	if (keystate[SDL_SCANCODE_W]) {
		camera.moveFront();
	}
	if (keystate[SDL_SCANCODE_S]) {
		camera.moveBack();
	}
	if (keystate[SDL_SCANCODE_A]) {
		camera.moveLeft();
	}
	if (keystate[SDL_SCANCODE_D]) {
		camera.moveRight();
	}
	if (keystate[SDL_SCANCODE_LSHIFT]) {
		camera.moveDown();
	}
	if (keystate[SDL_SCANCODE_SPACE]) {
		camera.moveUp();
	}
	if (keystate[SDL_SCANCODE_KP_4]) {
		camera.moveLeftConstrained();
	}
	if (keystate[SDL_SCANCODE_KP_6]) {
		camera.moveRightConstrained();
	}
	
}

void Draw()
{

	sdlAux->clearPixels();
	camera.resetDepthBuffer();

	//MakingTrees(sdlAux);
	
	//camera.drawLine(v1,v2, magenta, sdlAux);
	
	// camera.drawLine(d,a,cyan*0.3f,sdlAux);
	// camera.drawLine(h,e,cyan*0.3f,sdlAux);
	// camera.drawPoint(d,white,sdlAux);
	// camera.drawPoint(a,white,sdlAux);
	//camera.drawPoint(h,white,sdlAux);
	//camera.drawPoint(e,white,sdlAux);
	//for(int i(0); i < 12; i++){
	//	camera.drawLine(edges.at(2*i), edges.at(2*i+1), cyan*0.3f, sdlAux);
	//	camera.drawPoint(edges.at(2*i), white, sdlAux);
	//	camera.drawPoint(edges.at(2*i+1), white, sdlAux);
	//}

	for(int i(0); i < treeMeshes.at(0).size(); i+=3){
		if(!isnan(treeMeshes.at(0).at(i)) and !isnan(treeMeshes.at(0).at(i+1)) and !isnan(treeMeshes.at(0).at(i+2)))
			camera.drawTriangle(treeMeshes.at(0).at(i), treeMeshes.at(0).at(i+1), treeMeshes.at(0).at(i+2), sdlAux);
		//std::cout << "Drew triangle " << i/3 << std::endl;
	}

	for(int i(0); i < treeMeshes.at(1).size(); i+=3){
		if(!isnan(treeMeshes.at(1).at(i)) and !isnan(treeMeshes.at(1).at(i+1)) and !isnan(treeMeshes.at(1).at(i+2)))
			camera.drawTriangle(treeMeshes.at(1).at(i), treeMeshes.at(1).at(i+1), treeMeshes.at(1).at(i+2), blue, glm::vec3(0,0,0), sdlAux);
		//std::cout << "Drew triangle " << i/3 << std::endl;
	}

	for(int i(0); i < treeMeshes.at(2).size(); i+=3){
		if(!isnan(treeMeshes.at(2).at(i)) and !isnan(treeMeshes.at(2).at(i+1)) and !isnan(treeMeshes.at(2).at(i+2)))
			camera.drawTriangle(treeMeshes.at(2).at(i), treeMeshes.at(2).at(i+1), treeMeshes.at(2).at(i+2), green, glm::vec3(0,0,0), sdlAux);
		//std::cout << "Drew triangle " << i/3 << std::endl;
	}

	//camera.drawTriangle(glm::vec3(-1,0,0), glm::vec3(1,1,0), glm::vec3(0,0,1), sdlAux);
	//camera.drawTriangle(edges.at(0), edges.at(1), edges.at(9), sdlAux);
	//tree.graph.grid.showGrid(camera, sdlAux);
	//tree.graph.drawNodes(camera, sdlAux);


	//graph.drawEdges(camera, sdlAux);
	// for(int i(0); i < path.size(); i+=2){
	// 	camera.drawLineBright(path.at(i), path.at(i+1), yellow, sdlAux);
	// }
	// for(int i(0); i < branches.size(); i+=2){
	// 	camera.drawLine(branches.at(i), branches.at(i+1), cyan, sdlAux);
	// }

	// camera.drawPoint(glm::vec3(0.f,0.f,0.f), red, sdlAux);

	// glm::vec3 v;
	// for(auto p : border){
	// 	graph.grid.fromIndex(p, v);
	// 	camera.drawPoint(v, red, sdlAux);
	// }*
	
	

	// for(int i(tree.firstLim); i < tree.structure.size(); i+=2){
	//  	camera.drawLine(tree.structure.at(i), tree.structure.at(i+1), cyan, sdlAux);
	// }
	// for(int i(0); i < tree.firstLim; i+=2){
	//  	camera.drawLineBright(tree.structure.at(i), tree.structure.at(i+1), yellow, sdlAux);
	// }
	for(int i(0); i < tree.pathNodes2.size(); i++){
		glm::vec3 v;
		tree.graph.grid.fromIndex(tree.pathNodes2.at(i), v);
		camera.drawPointBright(v, magenta, sdlAux);
	}

	for(int i(0); i < tree.dests.size(); i++){
		glm::vec3 v;
		tree.graph.grid.fromIndex(tree.dests.at(i), v);
		camera.drawPointBright(v, green, sdlAux);
	}



	sdlAux->render();
}