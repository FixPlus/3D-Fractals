
#include "VulkanExample.h"



void VulkanExample::mouseMoved(double x, double y, bool &handled){
//		std::cout << mousePos.x << " " << x << std::endl;
//		std::cout << mousePos.y << " " << y << std::endl;
//	std::cout << "RotX: " << rotation.x << "  Rot y: " << rotation.y << std::endl;
	glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f) , glm::radians( - rotation.x * 0.25f), glm::vec3(1.0f, 0.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.y * 0.25f), glm::vec3(0.0f, 1.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.z * 0.25f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::vec4 dir =glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	if(mouseButtons.right == true){
		dir.x *= (float)(x - mousePos.x) / 10.0;
		dir.y *= (float)(y - mousePos.y) / 10.0;
		dir.z = 0.0f;
	}
	else
		if(mouseButtons.middle == true){
			dir.x = 0.0f;
			dir.y = 0.0f;
			dir.z *= (float)(y - mousePos.y) / 10.0;
		}
		else{
			dir = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
		}
	dir = dir * rotateMat;
	uboVS.viewDirection = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f) * rotateMat;
	uboVS.viewRef1 = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f) * rotateMat;
	uboVS.viewRef2 = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f) * rotateMat;

	cameraPos += glm::vec3(dir);
//	std::cout << "x: " << cameraPos.x << " y:" << cameraPos.y << "z: " << cameraPos.z << std::endl;
}

void VulkanExample::tiltCamera(float angle){

//	rotation.z += angle * 4.0f;

	glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f) , glm::radians( - rotation.x * 0.25f), glm::vec3(1.0f, 0.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.y * 0.25f), glm::vec3(0.0f, 1.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.z * 0.25f), glm::vec3(0.0f, 0.0f, 1.0f));

	uboVS.viewDirection = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f) * rotateMat;
	uboVS.viewRef1 = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f) * rotateMat;
	uboVS.viewRef2 = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f) * rotateMat;

}

void VulkanExample::moveCameraForward(float distance){
	cameraPos += glm::vec3(getCameraForwardDir() *  distance);
}

void VulkanExample::moveCameraBackward(float distance){
	cameraPos += glm::vec3(getCameraBackwardDir() *  distance);
}

void VulkanExample::moveCameraLeft(float distance){
	cameraPos += glm::vec3(getCameraLeftDir() *  distance);
}

void VulkanExample::moveCameraRight(float distance){
	cameraPos += glm::vec3(getCameraRightDir() *  distance);
}

void VulkanExample::moveCameraUp(float distance){
	cameraPos += glm::vec3(getCameraUpDir() *  distance);
}

void VulkanExample::moveCameraDown(float distance){
	cameraPos += glm::vec3(getCameraDownDir() *  distance);
}
glm::vec4 VulkanExample::getCameraForwardDir(){
	glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f) , glm::radians( - rotation.x * 0.25f), glm::vec3(1.0f, 0.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.y * 0.25f), glm::vec3(0.0f, 1.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.z * 0.25f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::vec4 dir =glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
	dir = dir * rotateMat;
	return dir;
}

glm::vec4 VulkanExample::getCameraBackwardDir(){
	return -getCameraForwardDir();
}

glm::vec4 VulkanExample::getCameraLeftDir(){
	glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f) , glm::radians( - rotation.x * 0.25f), glm::vec3(1.0f, 0.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.y * 0.25f), glm::vec3(0.0f, 1.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.z * 0.25f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::vec4 dir =glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f);
	dir = dir * rotateMat;
	return dir;

}

glm::vec4 VulkanExample::getCameraRightDir(){
	return -getCameraLeftDir();
}

glm::vec4 VulkanExample::getCameraUpDir(){
	glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f) , glm::radians( - rotation.x * 0.25f), glm::vec3(1.0f, 0.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.y * 0.25f), glm::vec3(0.0f, 1.0f, 0.0f));
	rotateMat = glm::rotate(rotateMat, glm::radians( - rotation.z * 0.25f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::vec4 dir =glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	dir = dir * rotateMat;
	return dir;

}

glm::vec4 VulkanExample::getCameraDownDir(){
	return -getCameraUpDir();
}

glm::vec3 VulkanExample::getCameraPos(){
	return cameraPos;
}

void VulkanExample::setCameraPos(glm::vec3 newPos){
	cameraPos = newPos;
}

void VulkanExample::OnUpdateUIOverlay(vks::UIOverlay *overlay)
{
}

void VulkanExample::viewChanged()
{
	// This function is called by the base example class each time the view is changed by user input
	updateUniformBuffers();
}

