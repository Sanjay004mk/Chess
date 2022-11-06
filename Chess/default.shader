$type vertex
#version 450

layout(location = 0) in vec2 aposition;
layout(location = 1) in vec2 auv;
layout(location = 2) in vec3 acolor;

layout(location = 0) out vec2 ouv;
layout(location = 1) out vec3 ocolor;

layout(set = 0, binding = 0) uniform PROJ
{
	mat4 proj;
}p;

void main()
{
	gl_Position = p.proj * vec4(aposition, 0.0, 1.0);
	ouv = auv;
	ocolor = acolor;
}

$type fragment
#version 450

layout(location = 0) out vec4 color;

layout(location = 0) in vec2 ouv;
layout(location = 1) in vec3 ocolor;

void main()
{
	color = vec4(ocolor, 1.0);
}