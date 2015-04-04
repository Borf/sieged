uniform sampler2D s_texture;

varying vec2 texcoord;
varying vec3 normal;
varying vec3 pos;

uniform vec4 colorMult;

#define M_PI 3.1415926535897932384626433832795

void main()
{
	float diffuse = dot(normalize(normal), normalize(vec3(0.5,1,0.5)));
	diffuse = clamp(diffuse, 0.0,1.0);

	float light = 0.5 * diffuse + 0.5;

	vec4 color = texture2D(s_texture, texcoord);

	gl_FragColor = vec4(colorMult.rgb * color.rgb * light, colorMult.a * color.a);
}