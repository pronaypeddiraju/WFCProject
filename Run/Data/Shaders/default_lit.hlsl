#include "dot3_include.hlsl"

//--------------------------------------------------------------------------------------
// Stream Input
// ------
// Stream Input is input that is walked by the vertex shader.  
// If you say "Draw(3,0)", you are telling to the GPU to expect '3' sets, or 
// elements, of input data.  IE, 3 vertices.  Each call of the VertxShader
// we be processing a different element. 
//--------------------------------------------------------------------------------------

// inputs are made up of internal names (ie: uv) and semantic names
// (ie: TEXCOORD).  "uv" would be used in the shader file, where
// "TEXCOORD" is used from the client-side (cpp code) to attach ot. 
// The semantic and internal names can be whatever you want, 
// but know that semantics starting with SV_* usually denote special 
// inputs/outputs, so probably best to avoid that naming.
struct vs_input_t 
{
   float3 position      : POSITION;
   float3 normal        : NORMAL;
   float3 tangent       : TANGENT;
   float3 biTangent     : BITANGENT;

   float4 color         : COLOR; 
   float2 uv            : TEXCOORD; 
}; 



//--------------------------------------------------------------------------------------
// Uniform Input
// ------
// Uniform Data is also externally provided data, but instead of changing
// per vertex call, it is constant for all vertices, hence the name "Constant Buffer"
// or "Uniform Buffer".  This is read-only memory; 
//
// I tend to use all cap naming here, as it is effectively a 
// constant from the shader's perspective. 
//
// register(b2) determines the buffer unit to use.  In this case
// we'll say this data is coming from buffer slot 2. 
//--------------------------------------------------------------------------------------
cbuffer camera_constants : register(b2)
{
   float4x4 VIEW; 
   float4x4 PROJECTION; 
   
   float3 CAMERA_POSITION;    
   float cam_unused0;   
};

//--------------------------------------------------------------------------------------
cbuffer model_constants : register(b3)
{
	float4x4 MODEL;  // LOCAL_TO_WORLD
}

//--------------------------------------------------------------------------------------
// Texures & Samplers
// ------
// Another option for external data is a Texture.  This is usually a large
// set of data (like an image) that we want to "sample" from.  
//
// A sampler are the rules for how to collect texel data for a given UV. 
//
// Like constant buffers, these hav ea slot they're expecting to be bound
// t0 means use texture unit 0,
// s0 means use sampler unit 0,
//
// In D3D11, constant buffers, textures, and samplers all have their own set 
// of slots.  Some data types may share a slot space (for example, unordered access 
// views (uav) use the texture space). 
//--------------------------------------------------------------------------------------
Texture2D<float4> tAlbedo : register(t0); // texutre I'm using for albedo (color) information
SamplerState sAlbedo : register(s0);      // sampler I'm using for the Albedo texture

Texture2D<float4> tNormalMap : register(t1);   // default "flat" (.5, .5, 1.0)
SamplerState sNormalMap : register(s1);

Texture2D<float4> tEmissiveMap : register(t2); // defualt "black"
SamplerState sEmissiveMap : register(s2);

//--------------------------------------------------------------------------------------
// Programmable Shader Stages
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// for passing data from vertex to fragment (v-2-f)
struct v2f_t 
{
   float4 position : SV_POSITION; 
   float3 normal : NORMAL;
   float3 tangent : TANGENT;
   float3 biTangent : BITANGENT;

   float3 worldPos : WORLDPOS;
   float4 color : COLOR; 
   float2 uv : UV; 
}; 

//--------------------------------------------------------------------------------------
float RangeMap( float v, float inMin, float inMax, float outMin, float outMax ) 
{ 
	return ( ( (v - inMin) * (outMax - outMin) / (inMax - inMin) ) + outMin); 
}

//--------------------------------------------------------------------------------------
// Vertex Shader
v2f_t VertexFunction(vs_input_t input)
{
   v2f_t v2f = (v2f_t)0;

   float4 local_pos = float4( input.position, 1.0f ); 
   float4 world_pos = mul( MODEL, local_pos );
   float4 view_pos = mul( VIEW, world_pos ); 
   float4 clip_pos = mul( PROJECTION, view_pos ); 
   float4 world_normal = mul( MODEL, float4(input.normal, 0.f));

   float4 world_tangent = mul(MODEL, float4(input.tangent, 0.f));
   float4 world_biTangent = mul(MODEL, float4(input.biTangent, 0.f));

   v2f.position = clip_pos; 
   v2f.color = input.color; 
   v2f.uv = input.uv; 
   v2f.worldPos = world_pos.xyz;
   v2f.normal = world_normal.xyz;
   v2f.tangent = world_tangent.xyz;
   v2f.biTangent = world_biTangent.xyz;

   return v2f;
}

float4 NormalToColor( float3 world_normal )
{
   float4 normal;

   normal.x = RangeMap(world_normal.x, -1.0f, 1.0f, 0.0f, 1.0f);
   normal.y = RangeMap(world_normal.y, -1.0f, 1.0f, 0.0f, 1.0f);
   normal.z = RangeMap(world_normal.z, -1.0f, 1.0f, 0.0f, 1.0f);
   normal.w = 1.0f;

   return normal;
}

//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
float4 FragmentFunction( v2f_t input ) : SV_Target0
{
   // First, we sample from our texture
   float4 texColor = tAlbedo.Sample( sAlbedo, input.uv ) * input.color; 
   
   float4 normalColor = tNormalMap.Sample( sAlbedo, input.uv );

   float3 surface_normal = normalColor.xyz * float3(2, 2, 1) - float3(1, 1, 0); 

   float3 vertex_tangent = normalize(input.tangent); 
   float3 vertex_bitan = normalize(input.biTangent); 
   float3 vertex_normal = normalize(input.normal); 

   // commonly referred to the TBN matrix
   float3x3 surface_to_world = float3x3( vertex_tangent, vertex_bitan, vertex_normal ); 

   // if you just go with my matrix format...
   float3 world_normal = mul( surface_normal, surface_to_world ); 

   //return NormalToColor(surface_normal);

   // OR... if you're stubborn
   // surface_to_world = transpose(surface_to_world); 
   // float3 world_normal = mul( surface_to_world, surface_normal ); 

   //GAMMA correction
   texColor = pow(texColor, GAMMA);

   lighting_t lighting = GetLighting( CAMERA_POSITION, input.worldPos, world_normal );

   //TO-DO: Add specularity!
   float4 final_color = float4(lighting.diffuse, 1.0f) * texColor;
   final_color += float4(lighting.specular, 0.f); // * sample specular map
   final_color = pow( final_color, 1.0f / GAMMA ); // convert back to sRGB space

   // component wise multiply to "tint" the output
   float4 finalColor = final_color * input.color; 
   
   // EMISSIVE (map defaults to "black"); 
   float4 emissive = tEmissiveMap.Sample( sAlbedo, input.uv ) * GetEmissiveFactor(); 
   finalColor += float4(emissive.xyz * emissive.w, 0); 

   //DEBUGGING STUFF
   //float4 finalColor = float4(((normalize(CAMERA_POSITION)) * 0.5f) + 1.f, 0.f);


   // output it; 
   //return float4(1.f, 0.f, 0.f, 1.f);
   return finalColor; 
}

