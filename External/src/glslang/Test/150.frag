#version 150 core

in vec4 gl_FragCoord;
layout(origin_upper_left, pixel_center_integer) in vec4 gl_FragCoord;  // ERROR
layout(pixel_center_integer) in vec4 gl_FragCoord;  // ERROR
layout(origin_upper_left) in vec4 foo;  // ERROR
layout(origin_upper_left, pixel_center_integer) in vec4 gl_FragCoord;

void main()
{
    vec4 c = gl_FragCoord;
}

layout(origin_upper_left, pixel_center_integer) in vec4 gl_FragCoord;  // ERROR, declared after use

in struct S { float f; } s;

float patch = 3.1;

uniform sampler2DMS sms;
uniform isampler2DMS isms;
uniform usampler2DMS usms;
uniform sampler2DMSArray smsa;
uniform isampler2DMSArray ismsa;
uniform usampler2DMSArray usmsa;

flat in ivec2 p2;
flat in ivec3 p3;
flat in int samp;

void barWxyz()
{
    ivec2 t11 = textureSize( sms);
    ivec2 t12 = textureSize(isms);
    ivec2 t13 = textureSize(usms);
    ivec3 t21 = textureSize( smsa);
    ivec3 t22 = textureSize(ismsa);
    ivec3 t23 = textureSize(usmsa);
     vec4 t31 = texelFetch( sms, p2, samp);
    ivec4 t32 = texelFetch(isms, p2, samp);
    uvec4 t33 = texelFetch(usms, p2, 3);
     vec4 t41 = texelFetch( smsa, p3, samp);
    ivec4 t42 = texelFetch(ismsa, ivec3(2), samp);
    uvec4 t43 = texelFetch(usmsa, p3, samp);
}

int primitiveID()
{
   return gl_PrimitiveID;
   gl_PerFragment; // ERROR, block name can't get reused
}

in double type1;    // ERROR
#extension GL_ARB_gpu_shader_fp64 : enable
double type2;
double type3 = 2.0;
int absTest = sqrt(type3);
double absTest2 = sqrt(type3);
double absTest3 = sqrt(2);
float dk = sqrt(11);

#extension GL_ARB_shader_bit_encoding: enable

float f;
vec4 v4;
ivec4 iv4a;
uvec2 uv2c;
void bitEncodingPass()
{
    int i = floatBitsToInt(f);
    uvec4 uv11 = floatBitsToUint(v4);
    vec4 v14 = intBitsToFloat(iv4a);
    vec2 v15 = uintBitsToFloat(uv2c);
}

#extension GL_ARB_shader_bit_encoding: disable

void bitEncodingFail()
{
    int i = floatBitsToInt(f); // Error, extention GL_ARB_bit_encoding is diabled
}

#extension GL_ARB_shading_language_packing : enable
vec2 v2a;
uint uy;

void packingPass()
{
    uint u19 = packSnorm2x16(v2a);
    vec2 v20 = unpackSnorm2x16(uy);
    uint u15 = packUnorm2x16(v2a);
    vec2 v16 = unpackUnorm2x16(uy);
    uint u17 = packHalf2x16(v2a);
    vec2 v18 = unpackHalf2x16(uy);
}

#extension GL_ARB_shading_language_packing : disable
void packingFail()
{
    uint u19 = packSnorm2x16(v2a); // Error, extension GL_ARB_shading_language_packing is disabled
}

// Testing extension GL_ARB_texture_query_lod
uniform sampler1D samp1D;
uniform sampler2DShadow samp2Ds;

void qlodFail()
{
    vec2 lod;
    float pf;
    vec2 pf2;
    vec3 pf3;

    lod = textureQueryLod(samp1D, pf);      // ERROR, extension GL_ARB_texture_query_lod needed
    lod = textureQueryLod(samp2Ds, pf2);    // ERROR, extension GL_ARB_texture_query_lod needed
}

#extension GL_ARB_texture_query_lod : enable

uniform isampler2D isamp2D;
uniform usampler3D usamp3D;
uniform samplerCube sampCube;
uniform isampler1DArray isamp1DA;
uniform usampler2DArray usamp2DA;

uniform sampler1DShadow samp1Ds;
uniform samplerCubeShadow sampCubes;
uniform sampler1DArrayShadow samp1DAs;
uniform sampler2DArrayShadow samp2DAs;

uniform samplerBuffer sampBuf;
uniform sampler2DRect sampRect;

void qlodPass()
{
    vec2 lod;
    float pf;
    vec2 pf2;
    vec3 pf3;

    lod = textureQueryLod(samp1D, pf);
    lod = textureQueryLod(isamp2D, pf2);
    lod = textureQueryLod(usamp3D, pf3);
    lod = textureQueryLod(sampCube, pf3);
    lod = textureQueryLod(isamp1DA, pf);
    lod = textureQueryLod(usamp2DA, pf2);

    lod = textureQueryLod(samp1Ds, pf);
    lod = textureQueryLod(samp2Ds, pf2);
    lod = textureQueryLod(sampCubes, pf3);
    lod = textureQueryLod(samp1DAs, pf);
    lod = textureQueryLod(samp2DAs, pf2);

    lod = textureQueryLod(sampBuf, pf);     // ERROR
    lod = textureQueryLod(sampRect, pf2);   // ERROR
}
