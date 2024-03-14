# pragma once

#include <glm/glm.hpp>
#include <list>

#include "Light.hpp"
#include "Material.hpp"
#include "PhongMaterial.hpp"
#include "Image.hpp"
#include "SceneNode.hpp"
#include "Ray.hpp"

class Camera {
    public:
        Camera(const glm::vec3& eye, const glm::vec3& view, const glm::vec3& up, double fovy, double aspect);
        ~Camera();
        glm::vec3 phongShading(const glm::vec3& p, const glm::vec3& n, const std::list<Light *> & lights, const Material *material, const glm::vec3 & ambient, SceneNode * root);
        glm::vec3 debug_shadow(const glm::vec3& p, const glm::vec3& n, const std::list<Light *> & lights, const Material *material, const glm::vec3 & ambient, SceneNode * root);
        // Ray get_ray(double x, double y, size_t w, size_t h);
        void render(Image & image, const std::list<Light *> & lights, SceneNode * root, const glm::vec3 & ambient);
    private:
        glm::vec3 m_eye;
        glm::vec3 m_view;
        glm::vec3 m_up;
        double m_fovy;
        double m_aspect;
        glm::vec3 m_u;
        glm::vec3 m_v;
        glm::vec3 m_w;
        glm::vec3 lowerleftcorner;
};