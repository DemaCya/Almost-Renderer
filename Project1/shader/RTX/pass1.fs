#version 330 core

layout (location = 0) out vec3 Color;

in vec3 pix;
in vec2 texcoord;

uniform samplerBuffer triangles;
uniform samplerBuffer nodes;
uniform sampler2D lastFrame;
uniform sampler2D hdrMap;

uniform int frameCounter;                                             
uniform int nTriangles;
uniform int nNodes;
uniform int width;
uniform int height;

#define PI 3.1415926
#define SIZE_TRIANGLE 12
#define SIZE_BVHNODE 4
#define INF 114514.0

float sqr(float x) {
    return x*x;
}


float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
} 

//0 - 1均匀分布的随机数函数
uint seed = uint(
uint((pix.x * 0.5 + 0.5) * width) * uint(1973) +
uint((pix.y * 0.5 + 0.5) * height) * uint(9277) +
uint(frameCounter) * uint(26699)) | uint(1);

uint wang_hash(inout uint seed) {
    seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
    seed *= uint(9);
    seed = seed ^ (seed >> 4);
    seed *= uint(0x27d4eb2d);
    seed = seed ^ (seed >> 15);
    return seed;
}

float rand() {
return float(wang_hash(seed)) / 4294967296.0;
}

vec2 randomArr[2048];
vec2 GenRandomArr(){
    uint j = 0u;
    for(int i=0;i<2048;++i)
    {
        randomArr[i] = Hammersley(j,2048u);
        ++j;
    }
    return randomArr[int(rand()*2048)];
}

//半球均匀采样
vec3 SampleHemisphere() {
    float z = rand();
    float r = max(0, sqrt(1.0 - z*z));
    float phi = 2.0 * PI * rand();
    return vec3(r * cos(phi), r * sin(phi), z);
}


//这里将主轴转到N上。
vec3 toNormalHemisphere(vec3 v,vec3 N){
    vec3 helper = vec3(1,0,0);
    if(abs(N.x)>0.999f) helper = vec3(0,0,1);
    vec3 tangent = normalize(cross(N,helper));
    
    vec3 bitangent = normalize(cross(N,tangent));

    return v.x*tangent+v.y*bitangent+v.z*N;
}



struct Ray{
    vec3 origin;
    vec3 direction;
};

struct Material {
        vec3 emissive; // 作为光源时的发光颜色
        vec3 baseColor;
        float subsurface;
        float metallic;
        float specular;
        float specularTint;
        float roughness;
        float anisotropic;
        float sheen;
        float sheenTint;
        float clearcoat;
        float clearcoatGloss;
        float IOR;
        float transmission;
    };

    // 三角形定义
struct Triangle {
        vec3 p1, p2, p3; // 顶点坐标
        vec3 n1, n2, n3; // 顶点法线
};
struct BVHNode{
    int left,right;
    int n,index;
    vec3 AA,BB;
};

struct HitResult{
    bool isHit;         //是否击中
    bool isInside;      //是否从里面往外面射
    float distance;     //原点到交点的距离
    vec3 hitpoint;      //交点
    vec3 normal;        //命中点的法线
    vec3 viewDir;       //击中该点的光线的方向
    Material material;  //击中的三角形的材质
};

HitResult hitTriangle(Triangle triangle,Ray ray){
    HitResult res;
    res.distance = INF;
    res.isHit = false;
    res.isInside = false;
    
    vec3 p1 = triangle.p1;
    vec3 p2 = triangle.p2;
    vec3 p3 = triangle.p3;

    vec3 S = ray.origin;
    vec3 d = ray.direction;
    vec3 N = normalize(cross(p2-p1,p3-p1));

    if(dot(N,d)>0.0f){
        N = -N;
        res.isInside = true;
    }
        
    if (abs(dot(N, d)) < 0.00001f) return res;

    float t = (dot(N,p1)-dot(S,N))/dot(d,N);
    if (t < 0.0005f) return res; 

    vec3 P = S +t*d;

    vec3 c1 = cross(p2-p1,P-p1);
    vec3 c2 = cross(p3-p2,P-p2);
    vec3 c3 = cross(p1-p3,P-p3);

    bool r1 = (dot(c1,N)>0 && dot(c2,N)>0 && dot(c3,N)>0);
    bool r2 = (dot(c1,N)<0 && dot(c2,N)<0 && dot(c3,N)<0);

    if(r1||r2){
        res.isHit = true;
        res.distance = t;
        res.hitpoint = P;
        res.viewDir = d;
    
        //三角形插值
       float alpha = (-(P.x-p2.x)*(p3.y-p2.y) + (P.y-p2.y)*(p3.x-p2.x)) / (-(p1.x-p2.x-0.00005)*(p3.y-p2.y+0.00005) + (p1.y-p2.y+0.00005)*(p3.x-p2.x+0.00005));
       float beta = (-(P.x-p3.x)*(p1.y-p3.y) + (P.y-p3.y)*(p1.x-p3.x)) / (-(p2.x-p3.x-0.00005)*(p1.y-p3.y+0.00005) + (p2.y-p3.y+0.00005)*(p1.x-p3.x+0.00005));
       float gama = 1.0-alpha-beta;
        vec3 Nsmooth = alpha*triangle.n1+beta*triangle.n2+gama*triangle.n3;
        Nsmooth = normalize(Nsmooth);
        res.normal = (res.isInside)? (-Nsmooth):(Nsmooth);
    }
    return res;
}

Triangle getTriangle(int i){
    int offset = i * SIZE_TRIANGLE;
    Triangle t;

    //顶点坐标
    t.p1 = texelFetch(triangles,offset+0).xyz;
    t.p2 = texelFetch(triangles,offset+1).xyz;
    t.p3 = texelFetch(triangles,offset+2).xyz;

    //顶点法线
    t.n1 = texelFetch(triangles,offset+3).xyz;
    t.n2 = texelFetch(triangles,offset+4).xyz;
    t.n3 = texelFetch(triangles,offset+5).xyz;

    return t;   
}

Material getMaterial(int i){
    int offset = i * SIZE_TRIANGLE;
    Material m;

    m.emissive = texelFetch(triangles,offset+6).xyz;
    m.baseColor = texelFetch(triangles,offset+7).xyz;
    m.subsurface = texelFetch(triangles,offset+8).x;
    m.metallic = texelFetch(triangles,offset+8).y;
    m.specular = texelFetch(triangles,offset+8).z;
    m.specularTint = texelFetch(triangles,offset+9).x;
    m.roughness = texelFetch(triangles,offset+9).y;
    m.anisotropic = texelFetch(triangles,offset+9).z;
    m.sheen = texelFetch(triangles,offset+10).x;
    m.sheenTint = texelFetch(triangles,offset+10).y;
    m.clearcoat = texelFetch(triangles,offset+10).z;
    m.clearcoatGloss = texelFetch(triangles,offset+11).x;
    m.IOR = texelFetch(triangles,offset+11).y;
    m.transmission = texelFetch(triangles,offset+11).z;

    return m;
}
BVHNode getBVH(int i){
    BVHNode node;
    int offset = i * SIZE_BVHNODE;
    ivec3 childs = ivec3(texelFetch(nodes,offset+0).xyz);
    ivec3 leafInfo = ivec3(texelFetch(nodes,offset+1).xyz);
    node.left = int(childs.x);
    node.right = int(childs.y);
    node.n = int(leafInfo.x);
    node.index = int(leafInfo.y);
   

    node.AA = texelFetch(nodes,offset+2).xyz;
    node.BB = texelFetch(nodes,offset+3).xyz;

    return node;
}

float hitAABB(Ray r, vec3 AA, vec3 BB) {
    vec3 invdir = 1.0 / r.direction;

    vec3 f = (BB - r.origin) * invdir;
    vec3 n = (AA - r.origin) * invdir;

    vec3 tmax = max(f, n);
    vec3 tmin = min(f, n);

    float t1 = min(tmax.x, min(tmax.y, tmax.z));
    float t0 = max(tmin.x, max(tmin.y, tmin.z));

    return (t1 >= t0) ? ((t0 > 0.0) ? (t0) : (t1)) : (-1);
}

// ----------------------------------------------------------------------------- //

// 暴力遍历数组下标范围 [l, r] 求最近交点
HitResult hitArray(Ray ray, int l, int r) {
    HitResult res;
    res.isHit = false;
    res.distance = INF;
    for(int i=l; i<=r; i++) {
        Triangle triangle = getTriangle(i);
        HitResult r = hitTriangle(triangle, ray);
        if(r.isHit && r.distance<res.distance) {
            res = r;
            res.material = getMaterial(i);
        }
    }
    return res;
}

// 遍历 BVH 求交
HitResult hitBVH(Ray ray) {
    HitResult res;
    res.isHit = false;
    res.distance = INF;

    // 栈
    int stack[256];
    int sp = 0;

    stack[sp] = 0;
    while(sp>-1) {
        int top = stack[sp--];
        BVHNode node = getBVH(top);
        
        // 是叶子节点，遍历三角形，求最近交点
        if(node.n>0) {
            int L = node.index;
            int R = node.index + node.n - 1;
            HitResult r = hitArray(ray, L, R);
            if(r.isHit && r.distance<res.distance) res = r;
            continue;
        }
        
        // 和左右盒子 AABB 求交
        float d1 = INF; // 左盒子距离
        float d2 = INF; // 右盒子距离
        if(node.left>0) {
            BVHNode leftNode = getBVH(node.left);
            d1 = hitAABB(ray, leftNode.AA, leftNode.BB);
        }
        if(node.right>0) {
            BVHNode rightNode = getBVH(node.right);
            d2 = hitAABB(ray, rightNode.AA, rightNode.BB);
        }

        // 在最近的盒子中搜索
        if(d1>0 && d2>0) {
            if(d1<d2) { // d1<d2, 左边先
                stack[++sp] = node.right;
                stack[++sp] = node.left;
            } else {    // d2<d1, 右边先
                stack[++sp] = node.left;
                stack[++sp] = node.right;
            }
        } else if(d1>0) {   // 仅命中左边
            stack[++sp] = node.left;
        } else if(d2>0) {   // 仅命中右边
            stack[++sp] = node.right;
        }
    }

    return res;
}

vec3 SampleCosineHemisphere(vec2 xi_1,vec2 xi_2, vec3 N){

    

    float r = sqrt(xi_1.x);
    float theta = xi_1.y*2.0*PI;
    float x = r*cos(theta);
    float y = r*sin(theta);
    float z = sqrt(1.0-x*x-y*y);

    vec3 L= toNormalHemisphere(vec3(x,y,z),N);
   return L;
}
vec3 SampleGTR2(vec2 xi_1,vec2 xi_2,vec3 V,vec3 N,float alpha){
    

    float phi_h = 2.0*PI*xi_1.x;
    float sin_phi_h = sin(phi_h);
    float cos_phi_h = cos(phi_h);
    
    float cos_theta = sqrt((1.0-xi_1.y)/(1.0+(alpha*alpha-1.0)*xi_1.y));
    float sin_theta = sqrt(max(0.0,1.0-cos_theta*cos_theta));

    vec3 H = vec3(sin_theta*cos_phi_h,sin_theta*sin_phi_h,cos_theta);

    H = toNormalHemisphere(H,N);

    vec3 L = reflect(-V,H);

    return L;

}

vec3 SampleGTR1(vec2 xi_1,vec2 xi_2,vec3 V,vec3 N,float alpha){
    

    float phi_h = 2.0*PI*xi_1.x;
    float sin_phi_h = sin(phi_h);
    float cos_phi_h = cos(phi_h);
    
    float cos_theta = sqrt((1.0-pow(alpha*alpha, 1.0-xi_1.y))/(1.0-alpha*alpha));

    float sin_theta = sqrt(max(0.0,1.0-cos_theta*cos_theta));

    vec3 H = vec3(sin_theta*cos_phi_h,sin_theta*sin_phi_h,cos_theta);

    H = toNormalHemisphere(H,N);

    vec3 L = reflect(-V,H);

    return L;

}

vec3 SampleBRDF(vec2 xi_1,vec2 xi_2,float xi_3,vec3 V,vec3 N,in Material material){
    float alpha_GTR1 = mix(0.1,0.001,material.clearcoatGloss);
    float alpha_GTR2 = max(0.001,sqr(material.roughness));

    float r_diffuse = (1.0-material.metallic);
    float r_specular = 1.0;
    float r_clearcoat = 0.25*material.clearcoat;
    float r_sum = r_diffuse + r_specular + r_clearcoat;

    float p_diffuse = r_diffuse / r_sum;
    float p_specular = r_specular / r_sum;
    float p_clearcoat = r_clearcoat / r_sum;

    float rd = xi_3;

    if(rd<=p_diffuse){
        return SampleCosineHemisphere(xi_1,xi_2,N);
    }
    else if(p_diffuse<rd && rd<=p_diffuse+p_specular){
        return SampleGTR2(xi_1,xi_2,V,N,alpha_GTR2);
    }
    else if(p_diffuse+p_specular<rd){
        return SampleGTR1(xi_1,xi_2,V,N,alpha_GTR1);
    }
    return vec3(0,1,0);
}

float GTR1(float NdotH,float a){
    if(a>=1) return 1/PI;
    float a2 = a*a;
    float t = 1+(a2-1)*NdotH*NdotH;
    return (a2-1)/(PI*log(a2)*t);
}
float GTR2(float NdotH,float a){
    float a2 = a*a;
    float t = 1+(a2-1)*NdotH*NdotH;
    return a2/(PI*t*t);
}

float BRDF_Pdf(vec3 V,vec3 N,vec3 L, in Material material){
    float NdotL = dot(N,L);
    float NdotV = dot(N,V);
    if(NdotL<0 || NdotV<0) return 0;

    vec3 H = normalize(L+V);
    float NdotH = dot(N,H);
    float LdotH = dot(L,H);

    float alpha = max(0.001,sqr(material.roughness));
    float Ds = GTR2(NdotH,alpha);
    float Dr = GTR1(NdotH,mix(0.1,0.001,material.clearcoatGloss));

    float pdf_diffuse = NdotL/PI;
    float pdf_specular  = Ds*NdotH / (4.0*dot(L,H));
    float pdf_clearcoat = Dr*NdotH / (4.0*dot(L,H));

    float r_diffuse = (1.0 - material.metallic);
    float r_specular = 1.0;
    float r_clearcoat = 0.25 * material.clearcoat;
    float r_sum = r_diffuse + r_specular + r_clearcoat;

    float p_diffuse = r_diffuse / r_sum;
    float p_specular = r_specular / r_sum;
    float p_clearcoat = r_clearcoat / r_sum;

    float pdf = p_diffuse * pdf_diffuse + p_specular * pdf_specular + p_clearcoat * pdf_clearcoat;

    pdf = max(1e-10, pdf);
    return pdf;

}

vec2 SampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv /= vec2(2.0 * PI, PI);
    uv += 0.5;
    uv.y = 1.0 - uv.y;
    return uv;
}

vec3 sampleHdr(vec3 v) {
    vec2 uv = SampleSphericalMap(normalize(v));
    vec3 color = texture2D(hdrMap, uv).rgb;
    return color;
}


float SchlickFresnel(float u){
    float m =  clamp(1-u,0,1);
    float m2 = m*m;
    return m2*m2*m;
}



float smithG_GGX(float NdotV, float alphaG) {
    float a = alphaG*alphaG;
    float b = NdotV*NdotV;
    return 1 / (NdotV + sqrt(a + b - a*b));
}


float GTR2_aniso(float NdotH, float HdotX, float HdotY, float ax, float ay) {
    return 1 / (PI * ax*ay * sqr( sqr(HdotX/ax) + sqr(HdotY/ay) + NdotH*NdotH));
}

float smithG_GGX_aniso(float NdotV, float VdotX, float VdotY, float ax, float ay) {
    return 1 / (NdotV + sqrt( sqr(VdotX*ax) + sqr(VdotY*ay) + sqr(NdotV) ));
}


void getTangent(vec3 N, inout vec3 tangent, inout vec3 bitangent) {
    vec3 helper = vec3(1, 0, 0);
    if(abs(N.x)>0.999) helper = vec3(0, 0, 1);
    bitangent = normalize(cross(N, helper));
    tangent = normalize(cross(N, bitangent));
}


vec3 BRDF_Evaluate(vec3 V,vec3 N,vec3 L,in Material material){
    vec3 Cdlin = material.baseColor;

    vec3 H = normalize(L+V);
    float NdotL = dot(N,L);
    float NdotV = dot(N,V);
    float LdotH = clamp(dot(L,H),0,1);
    float NdotH = dot(N,H);

    vec3 X = vec3(1);
    vec3 Y = vec3(1);
    getTangent(N,X,Y);

    float fd90 = 0.5+2.0*LdotH*LdotH*material.roughness;
    float FL = SchlickFresnel(NdotL);
    float FV = SchlickFresnel(NdotV);
    float Fd = mix(1.0,fd90,FL)*mix(1.0,fd90,FV);

    float Fss90 = LdotH * LdotH * material.roughness;
    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * (1.0 / (NdotL + NdotV) - 0.5) + 0.5);

    float Cdlum = 0.3 * Cdlin.r + 0.6 * Cdlin.g + 0.1 * Cdlin.b;
    vec3 Ctint = (Cdlum > 0) ? (Cdlin/Cdlum) : (vec3(1));
    vec3 Cspec = material.specular * mix(vec3(1), Ctint, material.specularTint);
    vec3 Cspec0 = mix(0.08*Cspec, Cdlin, material.metallic);

    float aspect = sqrt(1.0 - material.anisotropic * 0.9);
    float ax = max(0.001, sqr(material.roughness)/aspect);
    float ay = max(0.001, sqr(material.roughness)*aspect);
    float Ds = GTR2_aniso(NdotH, dot(H, X), dot(H, Y), ax, ay);
    float FH = SchlickFresnel(LdotH);
    vec3 Fs = mix(Cspec0, vec3(1), FH);
    float Gs;
    Gs = smithG_GGX_aniso(NdotL, dot(L, X), dot(L, Y), ax, ay);
    Gs *= smithG_GGX_aniso(NdotV, dot(V, X), dot(V, Y), ax, ay);

    float Dr = GTR1(NdotH, mix(0.1, 0.001, material.clearcoatGloss));
    float Fr = mix(0.04, 1.0, FH);
    float Gr = smithG_GGX(NdotL, 0.25) * smithG_GGX(NdotV, 0.25);

    vec3 Csheen = mix(vec3(1), Ctint, material.sheenTint);
    vec3 Fsheen = FH * material.sheen * Csheen;

     
    vec3 clearcoat = vec3(0.25 * Gr * Fr * Dr * material.clearcoat);
    vec3 specular = Gs*Fs*Ds;
    vec3 diffuse  = (1.0/PI) * mix(Fd, ss, material.subsurface) * Cdlin;
    diffuse += Fsheen;

    return diffuse*(1.0 - material.metallic)+specular+clearcoat;
}
vec3 pathTracingImportanceSamping(HitResult hit, int maxBounce){
    vec3 Lo = vec3(0);      // 最终的颜色
    vec3 history = vec3(1); // 递归积累的颜色

    for(int bounce=0; bounce<maxBounce; bounce++) {
        // 随机出射方向 wi
        vec2 xi_1 = GenRandomArr();
        vec2 xi_2 = GenRandomArr();
        float xi_3 = rand()+0.00001;

        vec3 V = -hit.viewDir;
        vec3 N = hit.normal;

        vec3 L = SampleBRDF(xi_1,xi_2,xi_3,V,N,hit.material);

        float cosine_i = dot(N, L);
        if(cosine_i <= 0.0) break;

        // 漫反射: 随机发射光线
        Ray randomRay;
        randomRay.origin = hit.hitpoint;
        randomRay.direction = L;
        HitResult newHit = hitBVH(randomRay);

                                        
        vec3 f_r = BRDF_Evaluate(V,N,L,hit.material);                        
        float pdf_brdf = BRDF_Pdf(V,N,L,hit.material); 
        if(pdf_brdf <= 0.0) break;
        // 未命中
        if(!newHit.isHit) {
            vec3 skyColor = sampleHdr(randomRay.direction);
            Lo += history * skyColor * f_r * cosine_i / pdf_brdf;
            break;
        }
        
        // 命中光源积累颜色
        vec3 Le = newHit.material.emissive;
        Lo += history * Le * f_r * cosine_i / pdf_brdf;
        
        // 递归(步进)
        hit = newHit;
        history *= f_r * cosine_i / pdf_brdf;  // 累积颜色
    }
    
    return Lo;
}
vec3 pathTracing(HitResult hit, int maxBounce) {

    vec3 Lo = vec3(0);      // 最终的颜色
    vec3 history = vec3(1); // 递归积累的颜色

    for(int bounce=0; bounce<maxBounce; bounce++) {
        // 随机出射方向 wi
        vec3 wi = toNormalHemisphere(SampleHemisphere(), hit.normal);

        // 漫反射: 随机发射光线
        Ray randomRay;
        randomRay.origin = hit.hitpoint;
        randomRay.direction = wi;
        HitResult newHit = hitBVH(randomRay);

        float pdf = 1.0 / (2.0 * PI);                                   // 半球均匀采样概率密度
        float cosine_o = max(0, dot(-hit.viewDir, hit.normal));         // 入射光和法线夹角余弦
        float cosine_i = max(0, dot(randomRay.direction, hit.normal));  // 出射光和法线夹角余弦
        vec3 V = -hit.viewDir;
        vec3 N = hit.normal;
        vec3 L = wi;
        vec3 f_r = BRDF_Evaluate(V,N,L,hit.material);                         // 漫反射 BRDF

        // 未命中
        if(!newHit.isHit) {
            vec3 skyColor = sampleHdr(randomRay.direction);
            Lo += history * skyColor * f_r * cosine_i / pdf;
            break;
        }
        
        // 命中光源积累颜色
        vec3 Le = newHit.material.emissive;
        Lo += history * Le * f_r * cosine_i / pdf;
        
        // 递归(步进)
        hit = newHit;
        history *= f_r * cosine_i / pdf;  // 累积颜色
    }
    
    return Lo;
}

vec3 toneMapping(in vec3 c, float limit) {
	float luminance = 0.3*c.x + 0.6*c.y + 0.1*c.z;
	return c * 1.0 / (1.0 + luminance / limit);
}
void main(){
    Ray ray;
    ray.origin = vec3(0,0,1.8);
    vec2 AA = vec2((rand()-0.5)/float(width), (rand()-0.5)/float(height));
    vec4 dir = vec4(pix.xy+AA, -1.5, 0.0);
    ray.direction = normalize(dir.xyz);

    HitResult firstHit  = hitBVH(ray);


    if(!firstHit.isHit) {Color  = sampleHdr(ray.direction);}
    else{
        int maxBounce = 2;
        vec3 Li = pathTracingImportanceSamping(firstHit,maxBounce);
        vec3 Le = firstHit.material.emissive;
        Color = Li+Le;
    }

    vec3 lastColor = texture2D(lastFrame,pix.xy*0.5+0.5).rgb;
    float data = max(1.0/float(frameCounter+1),0.01);
    
    Color = mix(lastColor,Color,1.0/float(frameCounter+1));
    
    //FragColor = vec4(Color,1.0);
    //Color = toneMapping(Color, 1.5);
	//Color = pow(Color, vec3(1.0 / 2.2));
}