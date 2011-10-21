// -*- C++ -*- automatisch in C++mode wechseln (emacs)

uniform vec4  TileColor0;
uniform vec4  TileColor1;
uniform float Diagonal;
uniform float BlackBorder;
uniform vec2  InteriorRange;

void main (void)
{
    vec4 coords;
    coords.xy = gl_TexCoord[0].xy;
    coords.zw = 1.0 - gl_TexCoord[0].xy;

    // the black border
    vec4 blackMult = smoothstep(vec4(0.0),vec4(0.04), coords);

    // the black diagonal
    float d = 0.04;
    if (abs((1.0-coords.x) - coords.y) <d) blackMult *= 1.0+5.0*(abs((1.0-coords.x) - coords.y)-d);

    // color ramp in interior
    vec4 interpolator = smoothstep(vec4(InteriorRange.x),vec4(InteriorRange.y),coords);

    vec4 interpolatedColor = mix(TileColor0,
				 TileColor1,
				 interpolator.x * interpolator.y * interpolator.z * interpolator.w);

    // overlay black border over interior color
	vec4 tmp = interpolatedColor * blackMult.x * blackMult.y * blackMult.z * blackMult.w;
    gl_FragColor =  vec4(tmp.x,tmp.y,tmp.z,1);
	//gl_FragColor = blackMult;

	//gl_FragColor = vec4(0,0,0,1);
}
