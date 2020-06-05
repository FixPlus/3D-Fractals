#include "drawer.h"
#include "MazeUI.h"
#include <cmath>

using namespace triGraphic;
Drawer* drawer;
MazeUI::Manager MazeUI::manager;

#if defined(_WIN32)

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)						
{																									
	if (drawer != NULL)																		
	{																								
		drawer->handleMessages(hWnd, uMsg, wParam, lParam);									
	}																								
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));												
}	


#endif


bool movingFwd = false, movingBwd = false, movingRgt = false, movingLft = false, tiltRgt = false, tiltLft = false;
bool super_sonic_boom = false, lightRot = false;
bool sliding = false, morphing = false;

float epsilon = 0.01f;
float spoungeUnit = 10.0f;
float collisionRadius = 1.0f;

enum{ NUM_OF_MIRRORS = 7 };

glm::vec4 mirrorRoots[NUM_OF_MIRRORS]   =   {glm::vec4(0.0f, 0.0f, 3.0f, 0.0f), glm::vec4(0.0f), glm::vec4(1.5f, 0.0f, 0.0f, 0.0f),
										glm::vec4(0.0f, 1.5f, 0.0f, 0.0f), glm::vec4(2.0f, 1.0f, 0.0f, 0.0f), glm::vec4(0.0f),
										glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)};
glm::vec4 mirrorNormals[NUM_OF_MIRRORS] =   {glm::vec4(-1.0f, 0.0f, -1.0f, 0.0f), glm::vec4(1.0f, 0.0f, -1.0f, 0.0f), glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f),
										glm::vec4(0.0f, -1.0f, 0.0f, 0.0f), glm::vec4(-1.0f, -1.0f, 0.0f, 0.0f), glm::vec4(1.0f, -1.0f, 0.0f, 0.0f),
										glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f)};


bool mirrorFlag = false;

glm::vec4 planeMirrorForced(glm::vec4 point, glm::vec4 planeRoot, glm::vec4 planeNormal){
	planeNormal = glm::normalize(planeNormal);

	glm::vec4 div = point - planeRoot;
	float normProj = glm::dot(planeNormal, div);
	point -= 2.0f * planeNormal *  normProj;

	return point;
}

glm::vec4 planeMirror(glm::vec4 point, glm::vec4 planeRoot, glm::vec4 planeNormal){
	planeNormal = glm::normalize(planeNormal);

	glm::vec4 div = point - planeRoot;
	float normProj = glm::dot(planeNormal, div);
	if(normProj < 0.0f){
		point -= 2.0f * planeNormal *  normProj;
		mirrorFlag = true;
	}
	else
		mirrorFlag = false;

	return point;
}
float closestDist(glm::vec4 pos){

	float rank = 6;
	for(int i = 0; i < rank; i++){
		float curRank = rank - i;
		float multiplier = glm::pow(3, curRank - 1) * spoungeUnit;
		for(int j = 0; j < NUM_OF_MIRRORS; j++){
			glm::vec4 normal = mirrorNormals[j];
			if(j == 5)
				normal.x *= drawer->uboVS.params.x;

			pos = planeMirror(pos, mirrorRoots[j] * multiplier, normal);
		}

	}
	float x = pos.x / spoungeUnit;
	float y = pos.y / spoungeUnit;
	float z = pos.z / spoungeUnit;
	float divX = x > 1.0f ? (x - 1.0f) * spoungeUnit : (x < 0.0f ? x * spoungeUnit : 0.0f); 
	float divY = y > 1.0f ? (y - 1.0f) * spoungeUnit : (y < 0.0f ? y * spoungeUnit : 0.0f); 
	float divZ = z > 1.0f ? (z - 1.0f) * spoungeUnit : (z < 0.0f ? z * spoungeUnit : 0.0f); 
	return glm::sqrt(divX * divX + divY * divY + divZ * divZ);

}

glm::vec4 closestNormal(glm::vec4 pos){

	float rank = 6;
	glm::vec4 xAxis = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec4 yAxis = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	glm::vec4 zAxis = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
	glm::vec4 nullVec = glm::vec4(0.0f);
	for(int i = 0; i < rank; i++){
		float curRank = rank - i;
		float multiplier = glm::pow(3, curRank - 1) * spoungeUnit;
		for(int j = 0; j < NUM_OF_MIRRORS; j++){
			glm::vec4 normal = mirrorNormals[j];
			if(j == 5)
				normal.x *= drawer->uboVS.params.x;
			pos = planeMirror(pos, mirrorRoots[j] * multiplier, normal);
			if(mirrorFlag == 1){
				xAxis = planeMirrorForced(xAxis, nullVec, normal);
				yAxis = planeMirrorForced(yAxis, nullVec, normal);
				zAxis = planeMirrorForced(zAxis, nullVec, normal);
			}
		}
	}

	float x = pos.x / spoungeUnit - 0.5f;
	float y = pos.y / spoungeUnit - 0.5f;
	float z = pos.z / spoungeUnit - 0.5f;
	glm::vec4 ret;
	if(glm::abs(x) >= glm::abs(y) && glm::abs(x) >= glm::abs(z) ){
		ret = glm::vec4(x > 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f, 0.0f);
	}
	else
		if(glm::abs(y) >= glm::abs(z)){
			ret = glm::vec4(0.0f, y > 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f);

		}
		else{
			ret = glm::vec4(0.0f, 0.0f, z > 0.0f ? 1.0f : -1.0f, 0.0f);

		}
	x = glm::dot(xAxis, ret);
	y = glm::dot(yAxis, ret);
	z = glm::dot(zAxis, ret);

	ret = glm::vec4(x, y, z, 0.0f);

	return ret;
}

glm::vec4 howMuchCanGo(glm::vec4 dir, glm::vec4 origin, float maxLen){
	int n = 0; //count of steps
	float totalRayLength = 0.0f; // pixel ray length
	glm::vec4 curPos = origin;
	
	sliding = false;

	float temp;
	if((temp = closestDist(curPos)) - collisionRadius < epsilon){
		glm::vec4 prevPos = curPos;
		curPos += dir * epsilon * 0.99f;
		n++;
		totalRayLength += epsilon;
		float temp2 = closestDist(curPos);
		if(temp2 >= temp && temp2 != 0.0f){
			while((temp = closestDist(curPos)) - collisionRadius < epsilon && totalRayLength < maxLen && temp != 0.0f){
				curPos += dir * temp * 0.99f;
				n++;
				totalRayLength += temp;
			}
			if(temp == 0.0f || totalRayLength > maxLen)
				return glm::normalize(dir) * glm::min(totalRayLength, maxLen);
		}
		else{
			curPos = prevPos;
			n = 0;
			totalRayLength = 0.0f;
			glm::vec4 norm = glm::normalize(closestNormal(curPos));
			glm::vec4 newDir = dir -  norm * glm::dot(norm, dir);

			if(length(newDir) != 0.0f){
				newDir = normalize(newDir);
			
				maxLen *= glm::dot(glm::normalize(dir), newDir) * 0.4f;  
				float prevTemp = temp;
				while((temp = closestDist(curPos)) - collisionRadius >= epsilon * 0.25f && totalRayLength < maxLen && temp != 0.0f && n < 500){
					curPos += newDir * temp * 0.99f;
					n++;
					totalRayLength += temp * 0.99f;
				}
				sliding = true;
			
				return newDir * glm::min(totalRayLength, maxLen);
			}
			else
				return dir * 0.0f;
		}	
	}

	while((temp = closestDist(curPos)) - collisionRadius > epsilon && totalRayLength < maxLen){
		curPos += dir * (temp - collisionRadius);
		n++;
		totalRayLength += (temp - collisionRadius);
	}

	return glm::normalize(dir) * glm::min(totalRayLength, maxLen);
}



class FpsCounter{
	float timer = 0.0f;
	int frame_count = 0;
public:
	float fps = 60.0f;

	void addFrame(float dt){
		frame_count++;
		timer += dt;
		if(timer >= 1.0f){
			fps = static_cast<float>(frame_count) / timer;
			timer = 0.0f;
			frame_count = 0;
		}
	}

};

FpsCounter fpsCounter;


#if defined(VK_USE_PLATFORM_XCB_KHR)

int main(int argc, char** argv)

#elif defined(_WIN32)

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)

#endif
{

#if defined(_WIN32)

	for (int32_t i = 0; i < __argc; i++) { VulkanExample::args.push_back(__argv[i]); };  			

#endif

	auto style = WS_WINDOWED;

#if defined(VK_USE_PLATFORM_XCB_KHR)
		std::cout << "Constructing drawer" << std::endl;
	drawer = new Drawer(style, nullptr,  nullptr, "Fractal");

#elif defined(_WIN32)

	drawer = new Drawer(hInstance, WndProc, style, [](UserInputMessage msg){
		if(msg.type == UserInputMessage::Type::UIM_KEYDOWN){
			switch(msg.detail){
				case KEY_W:{
					movingFwd = true;
					break;					
				}
				case KEY_A:{
					movingLft = true;
					break;					
				}
				case KEY_S:{
					movingBwd = true;
					break;					
				}
				case KEY_D:{
					movingRgt = true;
					break;					
				}
				case KEY_E:{
					tiltRgt = true;
					break;
				}
				case KEY_Q:{
					tiltLft = true;
					break;
				}
				case KEY_P:{
					lightRot = !lightRot;
					break;
				}
				case KEY_R:{
					drawer->toggleShadows();
					break;
				}
				case KEY_F:{
					morphing = !morphing;
					//std::cout << drawer->uboVS.params.x << std::endl;
					break;
				}
				case KEY_B:{
					//std::cout << drawer->uboVS.params.x << std::endl;
					break;
				}
				case KEY_SHIFT:{
					super_sonic_boom = true;
					break;					
				}
			}
		}


		if(msg.type == UserInputMessage::Type::UIM_KEYUP){
			switch(msg.detail){
				case KEY_W:{
					movingFwd = false;
					break;					
				}
				case KEY_A:{
					movingLft = false;
					break;					
				}
				case KEY_S:{
					movingBwd = false;
					break;					
				}
				case KEY_D:{
					movingRgt = false;
					break;					
				}
				case KEY_E:{
					tiltRgt = false;
					break;
				}
				case KEY_Q:{
					tiltLft = false;
					break;
				}

				case KEY_SHIFT:{
					super_sonic_boom = false;
					break;					
				}
			}
		}
	}, "Fractal");

#endif

	float cltDist = 0.0f;

	glm::vec4 clNorm;
	
	MazeUI::Window* debugWindow = new MazeUI::Window("Debug", 0.7f, 0.0f, 3.0f, 0.0f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	debugWindow->addNewItem(new MazeUI::StatText<float>(cltDist, "Estimated dist"));
	debugWindow->addNewItem(new MazeUI::StatText<bool>(sliding, "sliding"));
	debugWindow->addNewItem(new MazeUI::Text("Closest Normal:"));
	debugWindow->addNewItem(new MazeUI::StatText<float>(clNorm.x, "x"));
	debugWindow->addNewItem(new MazeUI::StatText<float>(clNorm.y, "y"));
	debugWindow->addNewItem(new MazeUI::StatText<float>(clNorm.z, "z"));
	
	MazeUI::Window* fpsWindow = new MazeUI::Window("Fps", 0.0f, 0.0f, 0.0f, 0.0f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	fpsWindow->addNewItem(new MazeUI::StatText<float>(fpsCounter.fps, "fps"));
	
	MazeUI::manager.addNewElement(debugWindow);
	MazeUI::manager.addNewElement(fpsWindow);
	float deltaTime = 0.0f;
	float overallTime = 0.0f;
	drawer->updateOverlay();
	drawer->setCameraPos({-1.0f, -1.0f, -1.0f});
	while(!drawer->shouldQuit()){
	
		auto tStart = std::chrono::high_resolution_clock::now();


#if defined(VK_USE_PLATFORM_XCB_KHR)
		drawer->handleEvents(); // LISTENING TO USER INPUT
#endif
		if(morphing){
			overallTime += deltaTime;
			drawer->uboVS.params.x = 1.0f + glm::sin(overallTime / 10.0f);
		}
		cltDist =  closestDist(glm::vec4(drawer->getCameraPos(), 0.0f));
		clNorm = closestNormal(glm::vec4(drawer->getCameraPos(), 0.0f));
		if(lightRot){
			glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f) , glm::radians( deltaTime * 10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			drawer->uboVS.lightDirection = rotateMat * drawer->uboVS.lightDirection;
		}
		if(movingFwd)
			drawer->moveCameraInDir(howMuchCanGo(drawer->getCameraForwardDir(), glm::vec4(drawer->getCameraPos(), 0.0f),deltaTime * (super_sonic_boom ? 1000.0f : 50.0f)));
			//drawer->moveCameraForward(deltaTime *(super_sonic_boom ? 100.0f : 20.0f));
		if(movingBwd)
			drawer->moveCameraInDir(howMuchCanGo(drawer->getCameraBackwardDir(), glm::vec4(drawer->getCameraPos(), 0.0f),deltaTime * (super_sonic_boom ? 1000.0f : 50.0f)));
			//drawer->moveCameraBackward(deltaTime *(super_sonic_boom ? 100.0f : 20.0f));
		if(movingRgt)
			drawer->moveCameraInDir(howMuchCanGo(drawer->getCameraRightDir(), glm::vec4(drawer->getCameraPos(), 0.0f),deltaTime * (super_sonic_boom ? 1000.0f : 50.0f)));
			//drawer->moveCameraRight(deltaTime *(super_sonic_boom ? 100.0f : 20.0f));
		if(movingLft)
			drawer->moveCameraInDir(howMuchCanGo(drawer->getCameraLeftDir(), glm::vec4(drawer->getCameraPos(), 0.0f),deltaTime * (super_sonic_boom ? 1000.0f : 50.0f)));
			//drawer->moveCameraLeft(deltaTime *(super_sonic_boom ? 100.0f : 20.0f));
		if(tiltLft)
			drawer->tiltCamera(-deltaTime * 60.0f);
		if(tiltRgt)
			drawer->tiltCamera(+deltaTime * 60.0f);

		drawer->draw(); //RENDERING THE FRAME


		auto tEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();


		int timeToSleepMicroSecs = 1000000u/60u - tDiff * 1000;
		if(timeToSleepMicroSecs < 0)
			timeToSleepMicroSecs = 0;

#if defined(VK_USE_PLATFORM_XCB_KHR)
		usleep((unsigned int)timeToSleepMicroSecs);
#endif


		tEnd = std::chrono::high_resolution_clock::now();
		tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
		
		deltaTime = tDiff / 1000.0f; // time of current cycle turn in seconds
		
		fpsCounter.addFrame(deltaTime);
	}

	return 0;
}