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


float epsilon = 0.01f;
float spoungeUnit = 1.0f;
float collisionRadius = 0.3f;

glm::vec4 planeMirror(glm::vec4 point, glm::vec4 planeRoot, glm::vec4 planeNormal){
	planeNormal = glm::normalize(planeNormal);

	glm::vec4 div = point - planeRoot;
	float normProj = glm::dot(planeNormal, div);
	if(normProj < 0.0f){
		point -= 2.0f * planeNormal *  normProj;
	}

	return point;
}

float closestDist(glm::vec4 pos){

	float rank = 6;
	for(int i = 0; i < rank; i++){
		float curRank = rank - i;
		float multiplier = glm::pow(3, curRank - 1);
		pos = planeMirror(pos, glm::vec4(0.0f, 0.0f, 3.0f * multiplier, 0.0f), glm::vec4(-1.0f, 0.0f, -1.0f, 0.0f));
		pos = planeMirror(pos, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, -1.0f, 0.0f));
		pos = planeMirror(pos, glm::vec4(1.5f * multiplier, 0.0f, 0.0f, 0.0f), glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f));
		pos = planeMirror(pos, glm::vec4(0.0f, 1.5f * multiplier, 0.0f, 0.0f), glm::vec4(0.0f, -1.0f, 0.0f, 0.0f));
		pos = planeMirror(pos, glm::vec4(2.0f * multiplier, 1.0f * multiplier, 0.0f, 0.0f), glm::vec4(-1.0f, -1.0f, 0.0f, 0.0f));
		pos = planeMirror(pos, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), glm::vec4(1.0f, -1.0f, 0.0f, 0.0f));
		pos = planeMirror(pos, glm::vec4(1.0f * multiplier, 0.0f, 0.0f, 0.0f), glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f));
	}
	float x = pos.x / spoungeUnit;
	float y = pos.y / spoungeUnit;
	float z = pos.z / spoungeUnit;
	float divX = x > 1.0f ? (x - 1.0f) * spoungeUnit : (x < 0.0f ? x * spoungeUnit : 0.0f); 
	float divY = y > 1.0f ? (y - 1.0f) * spoungeUnit : (y < 0.0f ? y * spoungeUnit : 0.0f); 
	float divZ = z > 1.0f ? (z - 1.0f) * spoungeUnit : (z < 0.0f ? z * spoungeUnit : 0.0f); 
	return glm::sqrt(divX * divX + divY * divY + divZ * divZ);

}


float howMuchCanGo(glm::vec4 dir, glm::vec4 origin, float maxLen){
	int n = 0; //count of steps
	float totalRayLength = 0.0f; // pixel ray length
	glm::vec4 curPos = origin;

	float temp;
	if((temp = closestDist(curPos)) - collisionRadius < epsilon){
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
				return glm::min(totalRayLength, maxLen);
		}
		else{
			return 0.0f;
		}	
	}

	while((temp = closestDist(curPos)) - collisionRadius > epsilon && totalRayLength < maxLen){
		curPos += dir * (temp - collisionRadius);
		n++;
		totalRayLength += (temp - collisionRadius);
	}

	return glm::min(totalRayLength, maxLen);
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
	
	MazeUI::Window* debugWindow = new MazeUI::Window("Debug", 0.7f, 0.0f, 3.0f, 0.0f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	debugWindow->addNewItem(new MazeUI::StatText<float>(cltDist, "Estimated dist"));
	
	MazeUI::Window* fpsWindow = new MazeUI::Window("Fps", 0.0f, 0.0f, 0.0f, 0.0f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	fpsWindow->addNewItem(new MazeUI::StatText<float>(fpsCounter.fps, "fps"));
	
	MazeUI::manager.addNewElement(debugWindow);
	MazeUI::manager.addNewElement(fpsWindow);
	float deltaTime = 0.0f;
	drawer->updateOverlay();
	drawer->setCameraPos({-1.0f, -1.0f, -1.0f});
	while(!drawer->shouldQuit()){
	
		auto tStart = std::chrono::high_resolution_clock::now();


#if defined(VK_USE_PLATFORM_XCB_KHR)
		drawer->handleEvents(); // LISTENING TO USER INPUT
#endif

		cltDist =  closestDist(glm::vec4(drawer->getCameraPos(), 0.0f));

		if(lightRot){
			glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f) , glm::radians( deltaTime * 10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			drawer->uboVS.lightDirection = rotateMat * drawer->uboVS.lightDirection;
		}
		if(movingFwd)
			drawer->moveCameraForward(howMuchCanGo(drawer->getCameraForwardDir(), glm::vec4(drawer->getCameraPos(), 0.0f),deltaTime * (super_sonic_boom ? 100.0f : 20.0f)));
			//drawer->moveCameraForward(deltaTime *(super_sonic_boom ? 100.0f : 20.0f));
		if(movingBwd)
			drawer->moveCameraBackward(howMuchCanGo(drawer->getCameraBackwardDir(), glm::vec4(drawer->getCameraPos(), 0.0f),deltaTime * (super_sonic_boom ? 100.0f : 20.0f)));
			//drawer->moveCameraBackward(deltaTime *(super_sonic_boom ? 100.0f : 20.0f));
		if(movingRgt)
			drawer->moveCameraRight(howMuchCanGo(drawer->getCameraRightDir(), glm::vec4(drawer->getCameraPos(), 0.0f),deltaTime * (super_sonic_boom ? 100.0f : 20.0f)));
			//drawer->moveCameraRight(deltaTime *(super_sonic_boom ? 100.0f : 20.0f));
		if(movingLft)
			drawer->moveCameraLeft(howMuchCanGo(drawer->getCameraLeftDir(), glm::vec4(drawer->getCameraPos(), 0.0f),deltaTime * (super_sonic_boom ? 100.0f : 20.0f)));
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