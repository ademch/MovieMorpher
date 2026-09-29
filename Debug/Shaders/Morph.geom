#version 150

uniform float fShadow;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3  objPos[];
in vec3  eyePos[];

out float fTilt;


vec3 VecRotateAroundAxis(vec3 v, vec3 axis, float angle)
{
    axis = normalize(axis);

    float c = cos(angle);
    float s = sin(angle);

    return v * c +
           cross(axis, v) * s +
           axis * dot(axis, v) * (1.0 - c);
}

void main()
{
	if (fShadow > 0.0)
	{
		vec2 A0 = objPos[0].xy;
		vec2 B0 = objPos[1].xy;
		vec2 C0 = objPos[2].xy;

		// Deformed triangle
		vec2 A1 = eyePos[0].xy;
		vec2 B1 = eyePos[1].xy;
		vec2 C1 = eyePos[2].xy;

		//vec2 A0 = vec2(0.0, 1.0);
		//vec2 B0 = vec2(0.5, 0.5);
		//vec2 C0 = vec2(0.0, 0.0);
		//
		//// Deformed triangle
		//vec2 A1 = vec2(0.0, 1.0);
		//vec2 B1 = vec2(1.0, 0.5);
		//vec2 C1 = vec2(0.5, 0.0);


		// Triangle edges
		vec2 e0x = B0 - A0;
		vec2 e0y = C0 - A0;

		vec2 e1x = B1 - A1;
		vec2 e1y = C1 - A1;


		// Construct matrices
		mat2 E0 = mat2( e0x, e0y );
		mat2 E1 = mat2( e1x, e1y );


		// Jacobian
		mat2 J = E1 * inverse(E0);

		// J*J^T
		mat2 M = transpose(J) * J;


		// Eigenvalues
		float a = M[0][0];
		float b = M[0][1];
		float c = M[1][1];

		float tr = a + c;

		float d = sqrt(max(0.0, (a-c)*(a-c) + 4.0*b*b));

		float lambdaMin = 0.5 * (tr - d);
		float lambdaMax = 0.5 * (tr + d);

		// Singular values
		float sigmaMin = sqrt(lambdaMin);
		float sigmaMax = sqrt(lambdaMax);

		// Principal directions
		vec2 principalDirMin;
		vec2 principalDirMax;


		if (abs(b) > 1e-6)
		{
			principalDirMin = normalize(vec2(lambdaMin-c, b));
			principalDirMax = normalize(vec2(lambdaMax-c, b));
		}
		else
		{
			principalDirMin = (a < c) ? vec2(1.0,0.0) : vec2(0.0,1.0);
			principalDirMax = (a > c) ? vec2(1.0,0.0) : vec2(0.0,1.0);
		}

		vec3 N = vec3(0.0, 0.0, 1.0);

		vec3 axis = vec3(principalDirMin, 0.0);

		float r = clamp(sigmaMin/max(sigmaMax,1e-6), 0.0,1.0);
		float theta = acos(r);

		vec3 tiltedNormal = VecRotateAroundAxis(N, axis, theta);

		if (determinant(J) < 0.0f)
			tiltedNormal.z *= -1.0f;

		fTilt = dot(tiltedNormal, vec3(0,0,1));
		fTilt = fTilt*fTilt;
	}
	else
		fTilt = 1.0;
	
	for (int i = 0; i < 3; i++)
    {
        gl_Position    = gl_in[i].gl_Position;
		gl_TexCoord[0] = gl_in[i].gl_TexCoord[0];

        EmitVertex();
    }

    EndPrimitive();
}
