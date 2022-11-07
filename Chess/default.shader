$type vertex
#version 450

layout(location = 0) in vec2 aposition;
layout(location = 1) in vec2 auv;
layout(location = 2) in vec3 acolor;
layout(location = 3) in int atexindex;

layout(location = 0) out vec2 ouv;
layout(location = 1) out vec3 ocolor;
layout(location = 2) flat out int otexindex;

layout(set = 0, binding = 0) uniform PROJ
{
	mat4 proj;
}p;

void main()
{
	gl_Position = p.proj * vec4(aposition, 0.0, 1.0);
	ouv = auv;
	ocolor = acolor;
	otexindex = atexindex;
}

$type fragment
#version 450

layout(location = 0) out vec4 color;

layout(location = 0) in vec2 ouv;
layout(location = 1) in vec3 ocolor;
layout(location = 2) flat in int otexindex;

layout(set = 0, binding = 1) uniform sampler2D textures[13];

void main()
{
	if (otexindex > -1)
	{
		color = texture(textures[otexindex], ouv);
		color.rgb *= ocolor;
	}
	else
		color = vec4(ocolor, 1.0);

	if (color.w <= 0.3)
		discard;
}