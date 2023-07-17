#version 430 core

layout (location = 0) in vec2 POSITION;
layout (location = 1) in vec2 TEXCOORD;
layout (location = 2) in vec2 RESOLUTION;
layout (location = 3) in int OBJ;

struct Rect { vec2 size, pos; };

struct Fixture {

    float alpha;
    float r;
    float g;
    float b;
    vec2 pos;
    vec2 size;
    vec4 gobo;
    float orientation; 
    float feedback; 
    float strobe;  
    float ratio; // unused for now mostly for alignment

};

layout(std140, binding = 1) uniform MatriceUBO  { Rect mat[24]; };
layout(std140, binding = 2) uniform FixtureUBO  { Fixture fix[24];} ;
layout(binding = 3, std140) uniform FixtureUBO2  { Fixture fix2[24];} ;

flat out int obj;
flat out int id;
flat out vec2 FBResolution;

out vec2 texcoord;
out float FBratio;

uniform int offset = 0;

uniform float debug0 = 0;
uniform float debug1 = 0;
uniform float debug2 = 0;
uniform float debug3 = 0;
uniform float debug4 = 0;

void main() {

    FBratio = RESOLUTION.x/RESOLUTION.y;
    FBResolution = vec2(RESOLUTION.x, RESOLUTION.y);

    obj = OBJ;

    id = gl_InstanceID+offset;

    texcoord = TEXCOORD;
    
    gl_Position = vec4(POSITION.x,POSITION.y,0,1);

    if (obj  == 1) {
    
        gl_Position.xy *= mat[id].size;

        gl_Position.xy -= mat[id].size*(vec2(1/mat[0].size)-1); // pourrait etre cpp
        
        gl_Position.xy += mat[id].pos*2;

        texcoord = gl_Position.xy*.5+.5;

    }

}
