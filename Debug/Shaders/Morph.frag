#version 140
 
uniform sampler2D tex0;

varying float fTilt;

void main()
{
	vec4 texture = texture2D(tex0, gl_TexCoord[0].st);
	gl_FragColor = mix( vec4(1.0, 1.0, 1.0, texture.w),
                        texture,
		fTilt*(fTilt)
                      );
}
