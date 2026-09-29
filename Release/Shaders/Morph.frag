#version 140
 
uniform sampler2D tex0;

varying float fTilt;

void main()
{
	vec4 texture = texture2D(tex0, gl_TexCoord[0].st);

	float fK = fTilt;

	if (fTilt > 0.9)
	{	
		fK = (fK - 0.9) / 0.1;
		//						   val0(1-a)    +  val1*a      a
		gl_FragColor = vec4( mix( 1.1*texture.xyz, 1.0*texture.xyz, fK), texture.w);
	}
	else
	{	
		fK = fK/0.9;
		//						   val0(1-a)    +  val1*a      a
		gl_FragColor = vec4( mix( vec3(0.56, 0.48, 0.41), 1.1*texture.xyz, fK), texture.w);
	}
}
