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
    vec4 blackMult = step(vec4(BlackBorder),coords);

    // the black diagonal
    if (abs((1.0-coords.x) - coords.y) < BlackBorder) blackMult *= Diagonal;

    // color ramp in interior
    vec4 interpolator = smoothstep(vec4(InteriorRange.x),vec4(InteriorRange.y),coords);

    vec4 interpolatedColor = mix(TileColor0,
				 TileColor1,
				 interpolator.x * interpolator.y * interpolator.z * interpolator.w);

    // overlay black border over interior color
    gl_FragColor = interpolatedColor * blackMult.x * blackMult.y * blackMult.z * blackMult.w;
}
