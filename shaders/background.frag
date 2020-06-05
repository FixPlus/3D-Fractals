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
	vec4 params;
	float shadowOption; // < 0 == shadow off ; >= 0 == shadow on
} ubo;

layout (location = 0) in vec3 inUVW;

layout (location = 0) out vec4 outFragColor;

float FOV = 0.5f; //in radian
float epsilon = 0.01f; // distance epsilon vincinity
float maxLength = 10000.0f;
int maxSteps = 500;

#define NUM_OF_MIRRORS 7

vec4 mirrorRoots[NUM_OF_MIRRORS]   =   {vec4(0.0f, 0.0f, 3.0f, 0.0f), vec4(0.0f), vec4(1.5f, 0.0f, 0.0f, 0.0f),
										vec4(0.0f, 1.5f, 0.0f, 0.0f), vec4(2.0f, 1.0f, 0.0f, 0.0f), vec4(0.0f),
										vec4(1.0f, 0.0f, 0.0f, 0.0f)};
vec4 mirrorNormals[NUM_OF_MIRRORS] =   {vec4(-1.0f, 0.0f, -1.0f, 0.0f), vec4(1.0f, 0.0f, -1.0f, 0.0f), vec4(-1.0f, 0.0f, 0.0f, 0.0f),
										vec4(0.0f, -1.0f, 0.0f, 0.0f), vec4(-1.0f, -1.0f, 0.0f, 0.0f), vec4(1.0f, -1.0f, 0.0f, 0.0f),
										vec4(-1.0f, 0.0f, 0.0f, 0.0f)};

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

vec4 planeMirrorForced(vec4 point, vec4 planeRoot, vec4 planeNormal){
	planeNormal = normalize(planeNormal);

	vec4 div = point - planeRoot;
	float normProj = dot(planeNormal, div);
	point -= 2.0f * planeNormal *  normProj;

	return point;
}

int flagMirror = 0;

vec4 planeMirror(vec4 point, vec4 planeRoot, vec4 planeNormal){
	planeNormal = normalize(planeNormal);

	vec4 div = point - planeRoot;
	float normProj = dot(planeNormal, div);
	if(normProj < 0.0f){
		point -= 2.0f * planeNormal *  normProj;
		flagMirror = 1;
	}
	else
		flagMirror = 0;

	return point;
}

float closestDist(vec4 pos){

	float rank = 6;
	for(int i = 0; i < rank; i++){
		float curRank = rank - i;
		float multiplier = pow(3, curRank - 1) * spoungeUnit;
		for(int j = 0; j < NUM_OF_MIRRORS; j++){
			vec4 normal = mirrorNormals[j];
			if(j == 5)
				normal.x *= ubo.params.x;

			pos = planeMirror(pos, mirrorRoots[j] * multiplier, normal);
		}

	}
	float x = pos.x / spoungeUnit;
	float y = pos.y / spoungeUnit;
	float z = pos.z / spoungeUnit;
	float divX = x > 1.0f ? (x - 1.0f) * spoungeUnit : (x < 0.0f ? x * spoungeUnit : 0.0f); 
	float divY = y > 1.0f ? (y - 1.0f) * spoungeUnit : (y < 0.0f ? y * spoungeUnit : 0.0f); 
	float divZ = z > 1.0f ? (z - 1.0f) * spoungeUnit : (z < 0.0f ? z * spoungeUnit : 0.0f); 
	return sqrt(divX * divX + divY * divY + divZ * divZ);

}

vec2 closestTextureCoord(vec4 pos){
	float rank = 6;
	for(int i = 0; i < rank; i++){
		float curRank = rank - i;
		float multiplier = pow(3, curRank - 1) * spoungeUnit;
		for(int j = 0; j < NUM_OF_MIRRORS; j++){
			vec4 normal = mirrorNormals[j];
			if(j == 5)
				normal.x *= ubo.params.x;
			
			pos = planeMirror(pos, mirrorRoots[j] * multiplier, normal);
		}
		
	}
	float x = pos.x / spoungeUnit - 0.5f;
	float y = pos.y / spoungeUnit - 0.5f;
	float z = pos.z / spoungeUnit - 0.5f;
	vec2 ret;
	if(abs(x) >= abs(y) && abs(x) >= abs(z) ){

		ret.x = y > 0.0f ? min(0.5f, y) : max(-0.5f, y);
		ret.x += 0.5f;

		ret.y = z > 0.0f ? min(0.5f, z) : max(-0.5f, z);
		ret.y += 0.5f;

	}
	else
		if(abs(y) >= abs(z)){
			ret.x = x > 0.0f ? min(0.5f, x) : max(-0.5f, x);
			ret.x += 0.5f;

			ret.y = z > 0.0f ? min(0.5f, z) : max(-0.5f, z);
			ret.y += 0.5f;

		}
		else{
			ret.x = x > 0.0f ? min(0.5f, x) : max(-0.5f, x);
			ret.x += 0.5f;

			ret.y = y > 0.0f ? min(0.5f, y) : max(-0.5f, y);
			ret.y += 0.5f;

		}

		return ret;

}

vec4 closestNormal(vec4 pos){

	float rank = 6;
	vec4 xAxis = vec4(1.0f, 0.0f, 0.0f, 0.0f);
	vec4 yAxis = vec4(0.0f, 1.0f, 0.0f, 0.0f);
	vec4 zAxis = vec4(0.0f, 0.0f, 1.0f, 0.0f);
	vec4 nullVec = vec4(0.0f);
	for(int i = 0; i < rank; i++){
		float curRank = rank - i;
		float multiplier = pow(3, curRank - 1) * spoungeUnit;
		for(int j = 0; j < NUM_OF_MIRRORS; j++){
			vec4 normal = mirrorNormals[j];
			if(j == 5)
				normal.x *= ubo.params.x;
			pos = planeMirror(pos, mirrorRoots[j] * multiplier, normal);
			if(flagMirror == 1){
				xAxis = planeMirrorForced(xAxis, nullVec, normal);
				yAxis = planeMirrorForced(yAxis, nullVec, normal);
				zAxis = planeMirrorForced(zAxis, nullVec, normal);
			}
		}
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
	x = dot(xAxis, ret);
	y = dot(yAxis, ret);
	z = dot(zAxis, ret);

	ret = vec4(x, y, z, 0.0f);

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
		//ambient occlusion calculated from number of steps 
		float occlusion = 1.0f - pow(2.715, -20.0f/float(n));

		//getting texture coordinates of the object point
		vec2 texCoords = closestTextureCoord(curPos);

		//enlighted calculated by the angle between surface normal and lightSource beam 
		float normDot = dot(closestNormal(curPos), -normalize(ubo.lightPos));
		float enlighted = normDot > 0.0f ? 0.5f + 0.3f * normDot : 0.5f + 0.1f * normDot;
		

		//	next step is to calculate dropped shadow
		//	for that we use same technic by shooting the ray from the object point 
		//	in the direction of light source 

		// here we use prevPos variable to start our marching from the point where we still
		// didnt hit epsilon treshhold
		if(ubo.shadowOption >= 0.0f && normDot > 0.0f && n < maxSteps / 2.0f){ 
		// we only calculate it if it is on, there are no self-shadowing and object is normally lit

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
			totalRayLength = maxLength * 1.1f;
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
		//Now it is using previously calculated texture coordintes

		//color = vec4(0.5f, 0.3f, 0.6f, 0.0f);
		color = texture(samplerColor1, texCoords, 1.0f); 
	

		outFragColor = color * totalLight;
	}


}