#version 430

in layout(location=0) vec3 position;
in layout(location=1) vec3 normal;
in layout(location=2) vec2 texCoord;

uniform mat4 modelTransformMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 normal;
out vec3 fragPosition;
out vec2 texCoords;

void main()
{
	vec4 v = vec4(position, 1.0);
	fragPosition = vec3(modelTransformMatrix * v);
	normal = mat3(transpose(inverse(modelTransformMatrix))) * normal;
	texCoords = texCoord;

	vec4 out_position = projectionMatrix * viewMatrix * modelTransformMatrix * v;
	gl_Position = out_position;	
}
