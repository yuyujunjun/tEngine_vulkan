#ifndef GLABOL
#define GLABOL
#define MaxForceFieldPerEmitter 8
#define Pi 3.1415927

#define InvPi 0.31830989
#define Inv2Pi 0.10132119

struct ParticleProperity
{
//	vec4 initialPosition;
//	vec4 initialVelocity;
	vec4 position;
	vec4 rotation;  //new add
	vec4 rotationVelocity;
	vec4 velocity;
	vec4 color;
//	vec4 ambient;
	vec2 massAndSize;
	vec2 ageAndlife;
	uint ShuffleID;
	float subResidual;
	int TrackingCount;
	int padding;
};

struct ForceProperity
{
	vec4 PositionAndStrength;
	vec4 DirAndDecay;
	vec2 padding;
	float radius;
	int ForceType;

};

struct PointCacheProperity
{
	vec4 position;
	vec4 rotation;
	vec4 color;
	vec2 massAndSize;
	vec2 ageAndlife;
};

struct UnityPC
{
	float position[3];
	float color[3];
};


vec4 AngleToQuat(vec3 r){
	float ca=cos(r.x);
	float cb=cos(r.y);
	float cr=cos(r.z);
	float sa=sin(r.x);
	float sb=sin(r.y);
	float sr=sin(r.z);
	mat3 M=mat3(ca*cb, ca*sb*sr-sa*cr, ca*sb*cr+sa*sr,
	        sa*cb, sa*sb*sr+ca*cr, sa*sb*cr-ca*sr,
			-sb, cb*sr, cb*cr);
    vec4 rtn;
	rtn.w=sqrt(1+M[0][0]+M[1][1]+M[2][2])/2.0;
	rtn.x=(M[2][1]-M[1][2])/(4.0*rtn.w);
	rtn.y=(M[0][2]-M[2][0])/(4.0*rtn.w);
	rtn.z=(M[1][0]-M[0][1])/(4.0*rtn.w);
	return rtn;
}


vec3 ACESToneMapping(in vec3 color, in float adapted_lum)
{
	const float A = 2.51f;
	const float B = 0.03f;
	const float C = 2.43f;
	const float D = 0.59f;
	const float E = 0.14f;
	vec3 result = color;
	result *= adapted_lum;
	result = (result * (A * result + B)) / (result * (C * result + D) + E);
	return result;
}
mediump float Brightness(mediump vec3 c)
{
	return max(max(c.r, c.g), c.b);
}

#endif