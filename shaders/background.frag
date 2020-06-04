#version 450

#define TEXTURE_SAMPLER(a) samplerColor ## a

#define TEXTURE(N) layout (binding = N) uniform sampler2D TEXTURE_SAMPLER(N)

TEXTURE(1);

layout (binding = 0) uniform UBO
{
	vec4 viewPos;
	vec4 viewDir;
	vec4 viewRef1;
	vec4 viewRef2;
	vec4 lightPos;

} ubo;

layout (location = 0) in vec3 inUVW;

layout (location = 0) out vec4 outFragColor;

float FOV = 0.5f; //in radian
float epsilon = 0.01f; // distance epsilon vincinity
float maxLength = 10000.0f;
int maxSteps = 500;

/*

//SPHERE ARRAY

float closestDist(vec4 pos){
	vec4 sphereCenter = vec4(5.0f, 5.0f, 5.0f, 0.0f);
	float radius = 1.0f;
	if(pos.x < 0) pos.x = -pos.x;
	if(pos.y < 0) pos.y = -pos.y;
	if(pos.z < 0) pos.z = -pos.z;

	pos.x = pos.x - int(pos.x / 10.0f) * 10.0f;
	pos.y = pos.y - int(pos.y / 10.0f) * 10.0f;
	pos.z = pos.z - int(pos.z / 10.0f) * 10.0f;

	return length(sphereCenter - pos) - radius;
}

vec4 closestNormal(vec4 pos){
	vec4 sphereCenter = vec4(5.0f, 5.0f, 5.0f, 0.0f);
	float radius = 1.0f;
	vec3 mult = vec3(1.0f, 1.0f, 1.0f);
	if(pos.x < 0) {pos.x = -pos.x; mult.x = -1.0f;}
	if(pos.y < 0) {pos.y = -pos.y; mult.y = -1.0f;}
	if(pos.z < 0) {pos.z = -pos.z; mult.z = -1.0f;}

	pos.x = pos.x - int(pos.x / 10.0f) * 10.0f;
	pos.y = pos.y - int(pos.y / 10.0f) * 10.0f;
	pos.z = pos.z - int(pos.z / 10.0f) * 10.0f;

	vec4 ret = sphereCenter - pos;
	ret = -normalize(ret);
	ret.x *= mult.x;
	ret.y *= mult.y;
	ret.z *= mult.z;

	return ret;
}

*/
float spoungeUnit = 10.0f;

float roundUp(float f){
	float ret = round(f);
	if(ret < f){
		ret += 1.0f;
	}
	return ret;
}
float roundDown(float f){
	float ret = round(f);
	if(ret > f){
		ret -= 1.0f;
	}
	return ret;
}


vec4 planeMirror(vec4 point, vec4 planeRoot, vec4 planeNormal){
	planeNormal = normalize(planeNormal);

	vec4 div = point - planeRoot;
	float normProj = dot(planeNormal, div);
	if(normProj < 0.0f){
		point -= 2.0f * planeNormal *  normProj;
	}

	return point;
}

float closestDist(vec4 pos){

	float rank = 6;
	for(int i = 0; i < rank; i++){
		float curRank = rank - i;
		float multiplier = pow(3, curRank - 1) * spoungeUnit;
		pos = planeMirror(pos, vec4(0.0f, 0.0f, 3.0f * multiplier, 0.0f), vec4(-1.0f, 0.0f, -1.0f, 0.0f));
		pos = planeMirror(pos, vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, 0.0f, -1.0f, 0.0f));
		pos = planeMirror(pos, vec4(1.5f * multiplier, 0.0f, 0.0f, 0.0f), vec4(-1.0f, 0.0f, 0.0f, 0.0f));
		pos = planeMirror(pos, vec4(0.0f, 1.5f * multiplier, 0.0f, 0.0f), vec4(0.0f, -1.0f, 0.0f, 0.0f));
		pos = planeMirror(pos, vec4(2.0f * multiplier, 1.0f * multiplier, 0.0f, 0.0f), vec4(-1.0f, -1.0f, 0.0f, 0.0f));
		pos = planeMirror(pos, vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, -1.0f, 0.0f, 0.0f));
		pos = planeMirror(pos, vec4(1.0f * multiplier, 0.0f, 0.0f, 0.0f), vec4(-1.0f, 0.0f, 0.0f, 0.0f));
	}
	float x = pos.x / spoungeUnit;
	float y = pos.y / spoungeUnit;
	float z = pos.z / spoungeUnit;
	float divX = x > 1.0f ? (x - 1.0f) * spoungeUnit : (x < 0.0f ? x * spoungeUnit : 0.0f); 
	float divY = y > 1.0f ? (y - 1.0f) * spoungeUnit : (y < 0.0f ? y * spoungeUnit : 0.0f); 
	float divZ = z > 1.0f ? (z - 1.0f) * spoungeUnit : (z < 0.0f ? z * spoungeUnit : 0.0f); 
	return sqrt(divX * divX + divY * divY + divZ * divZ);

}

vec4 closestNormal(vec4 pos){

	float rank = 6;
	for(int i = 0; i < rank; i++){
		float curRank = rank - i;
		float multiplier = pow(3, curRank - 1) * spoungeUnit;
		pos = planeMirror(pos, vec4(0.0f, 0.0f, 3.0f * multiplier, 0.0f), vec4(-1.0f, 0.0f, -1.0f, 0.0f));
		pos = planeMirror(pos, vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, 0.0f, -1.0f, 0.0f));
		pos = planeMirror(pos, vec4(1.5f * multiplier, 0.0f, 0.0f, 0.0f), vec4(-1.0f, 0.0f, 0.0f, 0.0f));
		pos = planeMirror(pos, vec4(0.0f, 1.5f * multiplier, 0.0f, 0.0f), vec4(0.0f, -1.0f, 0.0f, 0.0f));
		pos = planeMirror(pos, vec4(2.0f * multiplier, 1.0f * multiplier, 0.0f, 0.0f), vec4(-1.0f, -1.0f, 0.0f, 0.0f));
		pos = planeMirror(pos, vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, -1.0f, 0.0f, 0.0f));
		pos = planeMirror(pos, vec4(1.0f * multiplier, 0.0f, 0.0f, 0.0f), vec4(-1.0f, 0.0f, 0.0f, 0.0f));
	}

	float x = pos.x / spoungeUnit - 0.5f;
	float y = pos.y / spoungeUnit - 0.5f;
	float z = pos.z / spoungeUnit - 0.5f;
	vec4 ret;
	if(abs(x) >= abs(y) && abs(x) >= abs(z) ){
		ret = vec4(x > 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f, 0.0f);
	}
	else
		if(abs(y) >= abs(z)){
			ret = vec4(0.0f, y > 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f);

		}
		else{
			ret = vec4(0.0f, 0.0f, z > 0.0f ? 1.0f : -1.0f, 0.0f);

		}

	return ret;
}

vec4 getMarchDirection(){
	vec4 dir = ubo.viewDir + (ubo.viewRef1 * tan((inUVW.x - 0.5f) * 16.0f / 9.0f * FOV)) + (ubo.viewRef2 * tan((inUVW.y - 0.5f) * FOV));
	dir = normalize(dir);
	return dir;
}

void main(){

	// color and mix used for the background. Now it just uses single color, but could be altered by texture 

	vec4 color = vec4(0.2f, 0.7f, 0.7f,1.0f); //texture(samplerColor1, vec2(inUVW.x, inUVW.y), 1.0f);
	vec4 mix = vec4(1.0f, 1.0f, 1.0f, 1.0f) - color; //texture(samplerColor1, vec2(inUVW.x, inUVW.y), 1.0f);


	vec4 dir = getMarchDirection(); //direction od ray shooting out of camera
	vec4 curPos = ubo.viewPos; // contains the posirion of ray's front point
	
	int n = 0; //count of steps
	float totalRayLength = 0.0f; // counter for ray length


	float temp; // used to temproary contaion ED for each step
	float lastL; // used to contain the prev value of temp var
	vec4 prevPos = curPos; // used to contain the prev value of curPos

	/* 
		Next cycle is representation of ray marching technic
		
		Procces goes till ED hits the epsilon treshhold (aka ray collides an object)

		or rayLength hits maxLength treshhold, so there are no objects in this direction
		
		or number of steps hits maxSteps treshhold, so it will be considered we hit complex structure
		of object that is poorly lit

		closestDist is DE function
	*/

	while((temp = closestDist(curPos)) > epsilon && totalRayLength < maxLength && n < maxSteps){
		prevPos = curPos;
		curPos += dir * temp * 0.99f;
		n++;
		totalRayLength += temp * 0.99f;
		lastL = temp* 0.99f;
	}

	if(totalRayLength >= maxLength){ //should draw background case

		//sun function is to draw the light source
		float sun = pow(100,  10.0f * (-1.0f + max(0.0f, dot(dir, -normalize(ubo.lightPos)))));
		
		float glowing = pow(2.715, -20.0f/float(n));
		vec4 glowCol = vec4(1.0f, 0.0f, 1.0f, 0.0f);
		//color = color + mix * inUVW.y;
		vec4 sunCol = vec4(sun, sun, sun * 0.8f, 0.0f);
		outFragColor = color + sunCol + glowCol * glowing;
	}
	else{
		float periodic = pow(cos(length(curPos)), 2);
		//ambient occlusion calculated from number of steps 
		float occlusion = 1.0f - pow(2.715, -20.0f/float(n));

		//enlighted calculated by the angle between surface normal and lightSource beam 
		float normDot = dot(closestNormal(curPos), -normalize(ubo.lightPos));
		float enlighted = 0.2f + (0.5f + 0.5f * normDot) * 0.6f;
		

		//	next step is to calculate dropped shadow
		//	for that we use same technic by shooting the ray from the object point 
		//	in the direction of light source 

		// here we use prevPos variable to start our marching from the point where we still
		// didnt hit epsilon treshhold

		if(normDot > 0.0f && n < maxSteps){ // we only calculate it if there are no self-shadowing and object is normally lit

			curPos = prevPos;
			
			// but to be sure this point is not far away from real object point we use lastL variable
			// to check how much did last step took
			// if it too much (more than 1.5 * epsilon) we do an additional step to our point
			// that is less by 1.2 * epsilon than in previous marhing was. That unsures ud that 
			// we not hit epsilon treshhold again and be as close to real object point as we could

			if(lastL > epsilon * 1.5f)
				lastL -= epsilon * 1.2f;
			else
				lastL = 0.0f;

			curPos += dir * lastL;

			// then the technic is applied again

			totalRayLength = 0.0f;
			n = 0;
			vec4 normLightPos = normalize(ubo.lightPos);
		
			while(temp != 0.0f && (temp = closestDist(curPos)) > epsilon  && totalRayLength < maxLength && n < 5000){
				curPos -= normLightPos * temp * 0.99f;
				n++;
				totalRayLength += temp * 0.99f;
			}
		}
		else{
			totalRayLength = 0.0f;
		}

		// if we hit object on the way(rayLength < maxLength) - we apply a shadow
		// if not - normal enlighted var will be applied

		float shadow = totalRayLength < maxLength ? 0.5f : 1.0f;
		

		// and finnaly the totalLight is calculated based on ambient occlusion and shadowing

		float totalLight = occlusion;

		if(shadow == 1.0f)
			totalLight *= enlighted;
		else	
			totalLight *= shadow;

		//object also has a color that could be replaced by texture if needed

		color = vec4(0.5f, 0.8f, 1.0f, 0.0f);

		vec4 colInverted = vec4(1.0f) - color;

		color = color * periodic + colInverted * (1.0f - periodic);

		outFragColor = color * totalLight;
	}


}