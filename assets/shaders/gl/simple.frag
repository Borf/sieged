varying vec4 color;
varying vec3 normal;
varying vec3 pos;

uniform vec4 colorMult;

#define M_PI 3.1415926535897932384626433832795

void main()
{
	float diffuse = dot(normalize(normal), normalize(vec3(0.5,1,0.5)));
	diffuse = clamp(diffuse, 0.0,1.0);

	float light = 0.5 * diffuse + 0.5;

	float grid = 1;
	grid *= pow(cos((pos.x-0.5) * M_PI * 2) * 0.5 + 0.5, 0.5);
//	grid *= pow(cos((pos.y) * M_PI * 2) * 0.5 + 0.5, 0.5);
	grid *= pow(cos((pos.z-0.5) * M_PI * 2) * 0.5 + 0.5, 0.5);
	light = light * clamp(grid+0.5f, 0, 1);

	gl_FragColor = vec4(colorMult.rgb * color.rgb * light, colorMult.a * color.a);
}