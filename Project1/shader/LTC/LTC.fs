#version 330 core

out vec4 fragColor;

in vec3 worldPosition;
in vec3 worldNormal;
in vec2 texcoord;

struct Light{
	float intensity;
	vec3 color;
	vec3 points[4];
	bool twoSided;
};

uniform Light areaLight;
uniform vec3 areaLightTranslate;

struct Material{
	sampler2D diffuse;
	vec4 albedoRoughness;
};
uniform Material material;

uniform vec3 viewPosition;
uniform sampler2D LTC1;
uniform sampler2D LTC2;

const float LUT_SIZE = 64.0; // LUT大小
const float LUT_SCALE = (LUT_SIZE - 1.0)/LUT_SIZE;
const float LUT_BIAS = 0.5/LUT_SIZE;

vec3 IntegrateEdgeVec(vec3 v1, vec3 v2)
{
    float x = dot(v1, v2);
    float y = abs(x);

    float a = 0.8543985 + (0.4965155 + 0.0145206*y)*y;
    float b = 3.4175940 + (4.1616724 + y)*y;
    float v = a / b;

    float theta_sintheta = (x > 0.0) ? v : 0.5*inversesqrt(max(1.0 - x*x, 1e-7)) - v;

    return cross(v1, v2)*theta_sintheta;
}

vec3 LTC_Evaluate(vec3 N, vec3 V, vec3 worldPosition, mat3 Minv, vec3 points[4], bool twoSided){
	vec3 T1,T2;
	T1 = normalize(V-N*dot(V,N));
	T2 = cross(N,T1);

	Minv = Minv*transpose(mat3(T1,T2,N));

	vec3 L[4];

	L[0] = Minv*(points[0]-worldPosition);
	L[1] = Minv*(points[1]-worldPosition);
	L[2] = Minv*(points[2]-worldPosition);
	L[3] = Minv*(points[3]-worldPosition);

	vec3 dir = points[0] - worldPosition;
	vec3 lightNormal = cross(points[1] - points[0], points[3] - points[0]);
	bool behind = (dot(dir,lightNormal)<0.0);

	L[0] = normalize(L[0]);
	L[1] = normalize(L[1]);
	L[2] = normalize(L[2]);
	L[3] = normalize(L[3]);

	vec3 vsum = vec3(0.0);
	vsum += IntegrateEdgeVec(L[0],L[1]);
	vsum += IntegrateEdgeVec(L[1],L[2]);
	vsum += IntegrateEdgeVec(L[2],L[3]);
	vsum += IntegrateEdgeVec(L[3],L[0]);

	float len = length(vsum);

    float z = vsum.z/len;
    if (behind)
        z = -z;

    vec2 uv = vec2(z*0.5f + 0.5f, len); // range [0, 1]
    uv = uv*LUT_SCALE + LUT_BIAS;

    // 通过参数获得几何衰减系数
    float scale = texture(LTC2, uv).w;

    float sum = len*scale;
    if (!behind && !twoSided)
        sum = 0.0;

    // 输出
    vec3 Lo_i = vec3(sum, sum, sum);
    return Lo_i;

}

vec3 PowVec3(vec3 v, float p)
{
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}
const float gamma = 2.2;
vec3 ToLinear(vec3 v) { return PowVec3(v, gamma); }
vec3 ToSRGB(vec3 v)   { return PowVec3(v, 1.0/gamma); }

void main(){

	vec3 mDiffuse = texture(material.diffuse, texcoord).xyz;// * vec3(0.7f, 0.8f, 0.96f);
    vec3 mSpecular = ToLinear(vec3(0.23f, 0.23f, 0.23f)); // mDiffuse
    vec3 result = vec3(0.0f);

	vec3 N = normalize(worldNormal);
	vec3 V = normalize(viewPosition-worldPosition);
	float dotNV = clamp(dot(N,V),0.0f,1.0f);

	vec2 uv = vec2(material.albedoRoughness.w,sqrt(1.0f-dotNV));
	uv = uv*LUT_SIZE+LUT_BIAS;

	vec4 t1 = texture(LTC1,uv);
	vec4 t2 = texture(LTC2,uv);

	mat3 Minv = mat3(
		vec3(t1.x,0,t1.y),
		vec3(   0,1,   0),
		vec3(t1.z,0,t1.z)
	);

	vec3 translatedPoints[4];
	translatedPoints[0] = areaLight.points[0] + areaLightTranslate;
	translatedPoints[1] = areaLight.points[1] + areaLightTranslate;
	translatedPoints[2] = areaLight.points[2] + areaLightTranslate;
	translatedPoints[3] = areaLight.points[3] + areaLightTranslate;

	vec3 diffuse = LTC_Evaluate(N, V, worldPosition, mat3(1), translatedPoints, areaLight.twoSided);
    vec3 specular = LTC_Evaluate(N, V, worldPosition, Minv, translatedPoints, areaLight.twoSided);

	specular *= mSpecular*t2.x + (1.0f - mSpecular) * t2.y;

    result = areaLight.color * areaLight.intensity * (specular + mDiffuse * diffuse);

    fragColor = vec4(ToSRGB(result), 1.0f);
}
