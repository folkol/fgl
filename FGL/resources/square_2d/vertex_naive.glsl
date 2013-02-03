#version 110

uniform float timer;
attribute vec4 position;

varying vec2 texcoord;
varying float fade_factor;

void main()
{
	// Transforms that map worldspace to project space is 
	// called projection matrix
	mat3 otho_projection = mat3(
        vec3(3.0/4.0, 0.0, 0.0),
        vec3(    0.0, 1.0, 0.0),
        vec3(    0.0, 0.0, 1.0)
    );
	mat3 otho_projection_with_scaled_down_z_axis = mat3(
        vec3(3.0/4.0, 0.0, 0.0),
        vec3(    0.0, 1.0, 0.0),
        vec3(    0.0, 0.0, 0.5)
    );
	mat4 projection = mat4(
        vec4(3.0/4.0, 0.0, 0.0, 0.0),
        vec4(    0.0, 1.0, 0.0, 0.0),
        vec4(    0.0, 0.0, 0.5, 0.5),
		vec4(    0.0, 0.0, 0.0, 0.5)
    );

	// Transforms to orient and place objects in world space
	// Are called model-view matrices
    mat4 rotation = mat4(
        vec4(1.0,         0.0,         0.0, 0.0),
        vec4(0.0,  cos(timer),  sin(timer), 0.0),
        vec4(0.0, -sin(timer),  cos(timer), 0.0),
        vec4(0.0,         0.0,         0.0, 1.0)
    );
	 mat4 scale = mat4(
        vec4(4.0/3.0, 0.0, 0.0, 0.0),
        vec4(    0.0, 1.0, 0.0, 0.0),
        vec4(    0.0, 0.0, 1.0, 0.0),
        vec4(    0.0, 0.0, 0.0, 1.0)
    );

    gl_Position = projection * rotation * scale * position;
    texcoord = position.xy * vec2(0.5) + vec2(0.5);
	fade_factor = sin(timer) * 0.5 + 0.5;
}