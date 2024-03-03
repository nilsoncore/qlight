#version 450

layout (location = 0) in vec3 in_fragment_color;

layout (location = 0) out vec4 out_color;

void main() {
/*
	Vignette

	// Normalized pixel coordinates (from 0 to 1)
	vec2 uv = fragCoord / iResolution.xy;

	vec4 background = texture(iChannel0, uv);
	float vignette_horizontal = ( 4.0 * uv.x * ( 1.0 - uv.x ) );
	float vignette_vertical = ( 4.0 * uv.y * ( 1.0 - uv.y ) );
	float vignette = vignette_horizontal * vignette_vertical;
	float factor = pow(iTime, exp(iTime * iTime));
	vignette = pow(vignette, 1.0 / 5.0 * factor);

	out_color = vec4(in_fragment_color.rgb * vignette, 1.0);
*/

	out_color = vec4(in_fragment_color.rgb, 1.0);
}
