#include "Camera.hpp"
#include <random>

Camera::Camera(const glm::vec3& eye, const glm::vec3& view, const glm::vec3& up, double fovy, double aspect)
    : m_eye(eye), m_view(view), m_up(up), m_fovy(fovy), m_aspect(aspect)
{
    // back side of the camera
    m_w = glm::normalize(eye - view);
    // right side of the camera
    m_u = glm::normalize(glm::cross(up, m_w));
    // top side of the camera
    m_v = glm::cross(m_w, m_u);
}

Camera::~Camera()
{
}

glm::vec3 Camera::phongShading(const glm::vec3& p, const glm::vec3& n, const std::list<Light *> & lights, const Material *material, const glm::vec3 & ambient, SceneNode * root)
{
    glm::vec3 color = glm::vec3(0.0, 0.0, 0.0);
    PhongMaterial phongMaterial = *(dynamic_cast<const PhongMaterial *>(material));
    glm::vec3 ka = phongMaterial.get_kd();
    glm::vec3 kd = phongMaterial.get_kd();
    glm::vec3 ks = phongMaterial.get_ks();
    double shininess = phongMaterial.get_shininess();
    color += ka*ambient;
    for (auto light : lights) {
        glm::vec3 p_adjusted = p + 0.001f * (light->position - p);
        Ray shadow_ray(p_adjusted, light->position - p_adjusted);
        hit_record shadow_record;
        // color += ka * light->colour;
        if(root->hit(shadow_ray, 0.001, glm::length(light->position-p_adjusted), shadow_record)){
            continue;
        }
        glm::vec3 l = glm::normalize(light->position - p);
        glm::vec3 v = glm::normalize(m_eye - p);
        glm::vec3 h = glm::normalize(l + v);
        float distance = glm::length(light->position - p);
        float attenuation = 1.0 / (light->falloff[0] + light->falloff[1] * distance + light->falloff[2] * distance * distance);
        // glm::vec3 ambient = ka * light->colour;
        glm::vec3 diffuse = kd * light->colour * std::max(0.0f, glm::dot(n, l));
        glm::vec3 specular = ks * light->colour * glm::pow(std::max(0.0f, glm::dot(n, h)), (float)shininess) * attenuation;
        color += (diffuse + specular);
        // std::cout << "color: " << color.x << " " << color.y << " " << color.z << std::endl;
    }
    return color;
}

glm::vec3 Camera::debug_shadow(const glm::vec3& p, const glm::vec3& n, const std::list<Light *> & lights, const Material *material, const glm::vec3 & ambient, SceneNode * root){
    for(auto light: lights){
        glm::vec3 p_adjusted = p + 0.001f * n;
        Ray shadow_ray(p_adjusted, glm::normalize(light->position - p_adjusted));
        hit_record shadow_record;
        if(root->hit(shadow_ray, 0.001, glm::length(light->position-p_adjusted), shadow_record)){
            return glm::vec3(0.0, 0.0, 0.0);
        }
    }
    return glm::vec3(1.0, 1.0, 1.0);
}

double random_double(){
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

void Camera::render(Image & image, const std::list<Light *> & lights, SceneNode * root, const glm::vec3 & ambient)
{
    const int samples_per_pixel = 25;
    size_t h = image.height();
    size_t w = image.width();
    float d = (float)h / (2.0f * tan(glm::radians(m_fovy / 2.0f)));
    glm::vec3 lowerleftcorner = m_eye - d * m_w - (float)w / 2.0f * m_u - (float)h / 2.0f * m_v;
    for (uint y = 0; y < h; ++y) {
        for (uint x = 0; x < w; ++x) {
            image(x, y, 0) = 0.0;
            image(x, y, 1) = 0.0;
            image(x, y, 2) = 0.0;
            // Following code is for no anti-aliasing without random sampling
            // hit_record record;
            // glm::vec3 direction = lowerleftcorner + (float)x * m_u + (float)y * m_v - m_eye;
            // Ray ray(m_eye, direction);
            // if (root->hit(ray, 0.001, 100000000, record)) {
            //     glm::vec3 color = phongShading(record.p, record.normal, lights, record.material);
            //     image(x, y, 0) = color.x;
            //     image(x, y, 1) = color.y;
            //     image(x, y, 2) = color.z;
            // }

            // Following code is for anti-aliasing with random sampling
            Ray ray(m_eye, lowerleftcorner + (float)x * m_u + (float)y * m_v - m_eye);
            hit_record record;
            if(root->hit(ray, 0.001, std::numeric_limits<float>::max(), record)){
                // std::cout << "Something hit!" << std::endl;
                glm::vec3 color_sum(0.0f);
                for(int i = 0; i < samples_per_pixel; i++){
                    // hit_record record;
                    auto u = x + random_double();
                    auto v = y + random_double();
                    glm::vec3 direction = lowerleftcorner + (float)u * m_u + (float)v * m_v - m_eye;
                    Ray ray(m_eye, glm::normalize(direction));
                    // std::cout << "Something hit!" << std::endl;
                    glm::vec3 color = phongShading(record.p, record.normal, lights, record.material, ambient, root);
                    color_sum += color;
                } 
                // std::cout << "color: " << image(x, y, 0) << " " << image(x, y, 1) << " " << image(x, y, 2) << std::endl;
                image(x, y, 0) = color_sum.x / (float)samples_per_pixel;
                image(x, y, 1) = color_sum.y / (float)samples_per_pixel;
                image(x, y, 2) = color_sum.z / (float)samples_per_pixel;
            }
            else{
                // background color
                glm::vec3 unitDirection = normalize(ray.getDirection());
                float t = (normalize(unitDirection).x + 0.5);
                glm::vec3 bg_color = t * glm::vec3(0.2, 0.2, 0.8);
                image(x, y, 0) = bg_color.x;
                image(x, y, 1) = bg_color.y;
                image(x, y, 2) = bg_color.z;
            }
        }
    }
}