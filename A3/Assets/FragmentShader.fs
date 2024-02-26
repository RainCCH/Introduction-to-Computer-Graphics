#version 330

uniform bool picking;

struct Material {
    vec3 kd;
    vec3 ks;
    float shininess;
};
uniform Material material;

// Ambient light intensity for each RGB component.
uniform vec3 ambientIntensity;

in vec3 vcolour;

out vec4 fragColour;

void main() {
	fragColour = vec4(vcolour,1.);
}
