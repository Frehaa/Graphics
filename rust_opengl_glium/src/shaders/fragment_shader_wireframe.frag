#version 330

in vec3 vBC;

out vec4 color;

void main()
{
	if(any(lessThan(vBC, vec3(0.02)))) {
		color = vec4(1,1,1,1);
	}
	else {
		color = vec4(0,0,0,0);
	}
}