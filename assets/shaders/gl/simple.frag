#version 330 core

uniform sampler2D s_texture;
uniform sampler2DShadow s_shadowmap;
uniform vec4 colorMult;
uniform float buildFactor;
uniform vec2 location;
uniform float shadowFac;
uniform vec3 lightDirection;

varying vec2 texcoord;
varying vec3 normal;
varying vec3 pos;
varying vec4 shadowPos;
varying vec3 LightDirection_cameraspace;


#define M_PI 3.1415926535897932384626433832795
vec2 poissonDisk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);
  
// Permutation polynomial: (34x^2 + x) mod 289
vec3 permute(vec3 x) {
  return mod((34.0 * x + 1.0) * x, 289.0);
}

// Cellular noise, returning F1 and F2 in a vec2.
// Standard 3x3 search window for good F1 and F2 values
vec2 cellular(vec2 P) {
#define K 0.142857142857 // 1/7
#define Ko 0.428571428571 // 3/7
#define jitter 1.0 // Less gives more regular pattern
	vec2 Pi = mod(floor(P), 289.0);
 	vec2 Pf = fract(P);
	vec3 oi = vec3(-1.0, 0.0, 1.0);
	vec3 of = vec3(-0.5, 0.5, 1.5);
	vec3 px = permute(Pi.x + oi);
	vec3 p = permute(px.x + Pi.y + oi); // p11, p12, p13
	vec3 ox = fract(p*K) - Ko;
	vec3 oy = mod(floor(p*K),7.0)*K - Ko;
	vec3 dx = Pf.x + 0.5 + jitter*ox;
	vec3 dy = Pf.y - of + jitter*oy;
	vec3 d1 = dx * dx + dy * dy; // d11, d12 and d13, squared
	p = permute(px.y + Pi.y + oi); // p21, p22, p23
	ox = fract(p*K) - Ko;
	oy = mod(floor(p*K),7.0)*K - Ko;
	dx = Pf.x - 0.5 + jitter*ox;
	dy = Pf.y - of + jitter*oy;
	vec3 d2 = dx * dx + dy * dy; // d21, d22 and d23, squared
	p = permute(px.z + Pi.y + oi); // p31, p32, p33
	ox = fract(p*K) - Ko;
	oy = mod(floor(p*K),7.0)*K - Ko;
	dx = Pf.x - 1.5 + jitter*ox;
	dy = Pf.y - of + jitter*oy;
	vec3 d3 = dx * dx + dy * dy; // d31, d32 and d33, squared
	// Sort out the two smallest distances (F1, F2)
	vec3 d1a = min(d1, d2);
	d2 = max(d1, d2); // Swap to keep candidates for F2
	d2 = min(d2, d3); // neither F1 nor F2 are now in d3
	d1 = min(d1a, d2); // F1 is now in d1
	d2 = max(d1a, d2); // Swap to keep candidates for F2
	d1.xy = (d1.x < d1.y) ? d1.xy : d1.yx; // Swap if smaller
	d1.xz = (d1.x < d1.z) ? d1.xz : d1.zx; // F1 is in d1.x
	d1.yz = min(d1.yz, d2.yz); // F2 is now not in d2.yz
	d1.y = min(d1.y, d1.z); // nor in  d1.z
	d1.y = min(d1.y, d2.x); // F2 is in d1.y, we're done.
	return sqrt(d1.xy);
}


void main()
{
	float noiseFac = clamp(abs(cellular(texcoord + location)), 0, 1);
	//float noiseFac = clamp(abs(snoise(vec3(texcoord,0.0))), 0.0, 1.0);
	if(noiseFac > buildFactor)
		discard;//color.rgb = vec3(0,0,0);

	// Normal of the computed fragment, in camera space
	vec3 n = normalize( normal);
	// Direction of the light (from the fragment to the light)
	vec3 l = normalize( LightDirection_cameraspace );
	// Cosine of the angle between the normal and the light direction, 
	// clamped above 0
	//  - light is at the vertical of the triangle -> 1
	//  - light is perpendiular to the triangle -> 0
	//  - light is behind the triangle -> 0
	float cosTheta = clamp( dot( n,l ), 0,1 );



	float bias = 0.001*tan(acos(cosTheta)); // cosTheta is dot( n,l ), clamped between 0 and 1
	bias = clamp(abs(bias), 0,0.01);

	bias = 0.0001;



	float visibility = 1.0;

	if(shadowFac > 0.1)
	{
		for (int i=0;i<4;i++){
			// use either :
			//  - Always the same samples.
			//    Gives a fixed pattern in the shadow, but no noise
			int index = i;
			//  - A random sample, based on the pixel's screen location. 
			//    No banding, but the shadow moves with the camera, which looks weird.
			// int index = int(16.0*random(gl_FragCoord.xyy, i))%16;
			//  - A random sample, based on the pixel's position in world space.
			//    The position is rounded to the millimeter to avoid too much aliasing
			//int index = int(16.0*random(floor(pos.xyz*1000.0), i))%16;
		
			// being fully in the shadow will eat up 4*0.2 = 0.8
			// 0.2 potentially remain, which is quite dark.
			visibility -= 0.15*(1.0-texture( s_shadowmap, vec3(shadowPos.xy + poissonDisk[index]/4000.0,  (shadowPos.z-bias)/shadowPos.w) ));
		}
	}

	float diffuse = dot(normalize(normal), normalize(lightDirection));
	diffuse = clamp(diffuse, 0.0,1.0);

	//diffuse = pow(diffuse, 0.25);

	float light = 0.5 * diffuse + 0.5;

	vec4 color = texture2D(s_texture, texcoord);




	light = min(light, visibility);


	gl_FragColor = vec4(colorMult.rgb * color.rgb * light, colorMult.a * color.a);
}