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

	// Transforms to orient and place objects in world space
	// Are called model-view matrices
    mat3 rotation = mat3(
        vec3(1.0,         0.0,         0.0),
        vec3(0.0,  cos(timer),  sin(timer)),
        vec3(0.0, -sin(timer),  cos(timer))
    );
	mat3 object_scale = mat3(
        vec3(4.0/3.0, 0.0, 0.0),
        vec3(    0.0, 1.0, 0.0),
        vec3(    0.0, 0.0, 1.0)
    );

	vec3 projected_position = otho_projection_with_scaled_down_z_axis * rotation * object_scale * position.xyz;
	float perspective_factor = projected_position.z * 0.5 + 1.0;

    gl_Position = vec4(projected_position/perspective_factor, 1.0);

    texcoord = position.xy * vec2(0.5) + vec2(0.5);
	fade_factor = sin(timer) * 0.5 + 0.5;
}