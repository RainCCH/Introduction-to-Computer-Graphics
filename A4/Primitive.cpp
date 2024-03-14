// Termm--Fall 2020

#include "Primitive.hpp"
#include <iostream>

#define EPSILON 1e-4

using namespace std;

Primitive::~Primitive()
{
}

Sphere::~Sphere()
{
}

bool Sphere::hit(const Ray& ray, double t0, double t1, hit_record& record)
{
    NonhierSphere sphere(glm::vec3(0, 0, 0), 1);
    return sphere.hit(ray, t0, t1, record);
}

Cube::~Cube()
{
}

bool Cube::hit(const Ray& ray, double t0, double t1, hit_record& record)
{
    NonhierBox box(glm::vec3(0, 0, 0), 1);
    return box.hit(ray, t0, t1, record);
}

NonhierSphere::~NonhierSphere()
{
}

NonhierBox::~NonhierBox()
{
}

bool NonhierSphere::hit(const Ray& ray, double t0, double t1, hit_record& record)
{
    // std::cout << "NonhierSphere::hit" << "\n";
    glm::vec3 L = ray.getOrigin() - m_pos;
    double a = glm::dot(ray.getDirection(), ray.getDirection());
    double b = 2.0f * glm::dot(L, ray.getDirection());
    double c = glm::dot(L, L) - m_radius * m_radius;
    // std::cout << "a: " << a << " b: " << b << " c: " << c << "\n";
    double discriminant = b * b - 4 * a * c;
    // std::cout << "discriminant: " << discriminant << "\n";
    if (discriminant < 0) {
        return false;
    }
    double temp_t = (-b - sqrt(discriminant)) / (2 * a);
    if (temp_t < t0 || temp_t > t1) {
        temp_t = (-b + sqrt(discriminant)) / (2 * a);
        if (temp_t < t0 || temp_t > t1) {
            return false;
        }
    }
    // std::cout << "NonhierSphere::hit" << "\n";
    record.t = temp_t;
    record.p = ray.At(record.t);
    record.normal = glm::normalize(record.p - m_pos);
    return true;
}

bool NonhierBox::hit(const Ray& ray, double t0, double t1, hit_record& record)
{
    glm::vec3 p = m_pos - ray.getOrigin();
    glm::vec3 q = m_pos + glm::vec3(m_size, m_size, m_size) - ray.getOrigin();

    double txmin = p.x / ray.getDirection().x;
    double txmax = q.x / ray.getDirection().x;
    double tymin = p.y / ray.getDirection().y;
    double tymax = q.y / ray.getDirection().y;
    double tzmin = p.z / ray.getDirection().z;
    double tzmax = q.z / ray.getDirection().z;

    double tmin = glm::max(glm::max(glm::min(txmin, txmax), glm::min(tymin, tymax)), glm::min(tzmin, tzmax));
    double tmax = glm::min(glm::min(glm::max(txmin, txmax), glm::max(tymin, tymax)), glm::max(tzmin, tzmax));

    if (tmin > tmax || tmax < t0 || tmin > t1) {
        return false;
    }

    record.t = tmin;
    glm::vec3 point = ray.At(record.t);

    if (fabs(point.x - m_pos.x) < EPSILON) {
        record.normal = glm::vec3(-1, 0, 0);
    } else if (fabs(point.x - (m_pos.x + m_size)) < EPSILON) {
        record.normal = glm::vec3(1, 0, 0);
    } else if (fabs(point.y - m_pos.y) < EPSILON) {
        record.normal = glm::vec3(0, -1, 0);
    } else if (fabs(point.y - (m_pos.y + m_size)) < EPSILON) {
        record.normal = glm::vec3(0, 1, 0);
    } else if (fabs(point.z - m_pos.z) < EPSILON) {
        record.normal = glm::vec3(0, 0, -1);
    } else if (fabs(point.z - (m_pos.z + m_size)) < EPSILON) {
        record.normal = glm::vec3(0, 0, 1);
    }
    return true;
}
