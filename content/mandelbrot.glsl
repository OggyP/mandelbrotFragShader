#version 400

uniform vec2 centerOfScreen;
uniform float distancePerPixel;
uniform vec2 resolution;

#define cx_add(a, b) vec2(a.x + b.x, a.y + b.y)

// Complex power
// Let z = r(cos θ + i sin θ)
// Then z^n = r^n (cos nθ + i sin nθ)
vec2 cx_pow(vec2 a, float n) {
	float angle = atan(a.y, a.x);
	float r = length(a);
	float real = pow(r, n) * cos(n * angle);
	float im = pow(r, n) * sin(n * angle);
	return vec2(real, im);
}

////////////////////////////////////////////////////////////
// end Complex Number math by DonKarlssonSan
////////////////////////////////////////////////////////////

int iterationsToPastTwo(vec2 c) {
	vec2 z = c;
	for(int i = 0; i < 100; i++) {

		z = cx_add(cx_pow(z, 2), c);
		if(pow(z.x, 2) + pow(z.y, 2) > 4.0)
			return i;
	}
	return -1;
}

vec2 screenToComplex(vec2 screenPos) {
	return vec2((screenPos.x - resolution.x / 2) * distancePerPixel + centerOfScreen.x, (screenPos.y - resolution.y / 2) * distancePerPixel + centerOfScreen.y);
}

void main(void) {
	vec2 complexPos = screenToComplex(gl_FragCoord.xy);
	int iterations = iterationsToPastTwo(complexPos);
	if(iterations == -1) {
		gl_FragColor = vec4(0, 0, 0, 1);
	} else {
		if(iterations <= 50)
			gl_FragColor = mix(vec4(0.017, 0.029, 0.283, 1.0), vec4(0.483, 0.506, 1.0, 1.0), (iterations / 50.0));
		else
			gl_FragColor = mix(vec4(0.483, 0.506, 1.0, 1.0), vec4(0.054, 0.019, 0.260, 1.0), ((iterations - 50) / 50.0));
	}

}