#pragma once

#include <glm/glm.hpp>

class Ray {
    public:
        Ray();
        Ray(const glm::vec3 &a, const glm::vec3 &b);
        glm::vec3 getOrigin() const;
        glm::vec3 getDirection() const;
        glm::vec3 At(float t) const;

    private:
        glm::vec3 origin;
        glm::vec3 direction;
};