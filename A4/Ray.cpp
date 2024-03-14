
#include "Ray.hpp"

using namespace std;
using namespace glm;

Ray::Ray() {}

Ray::Ray(const glm::vec3 &a, const glm::vec3 &b) { origin = a; direction = b; }

vec3 Ray::getOrigin() const { return origin; }

vec3 Ray::getDirection() const { return direction; }

vec3 Ray::At(float t) const { return origin + t*direction; }