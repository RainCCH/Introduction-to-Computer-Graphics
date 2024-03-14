// Termm--Fall 2020

#pragma once

#include <glm/glm.hpp>
#include "Ray.hpp"
#include "Material.hpp"

class hit_record {
  public:
    double t;
    glm::vec3 p;
    glm::vec3 normal;
    Material *material;
};

class Primitive {
public:
  virtual ~Primitive();
  virtual bool hit(const Ray& ray, double t0, double t1, hit_record& record) = 0;
};

class Sphere : public Primitive {
public:
  virtual ~Sphere();
  bool hit(const Ray& ray, double t0, double t1, hit_record& record);
};

class Cube : public Primitive {
public:
  virtual ~Cube();
  bool hit(const Ray& ray, double t0, double t1, hit_record& record);
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3& pos, double radius)
    : m_pos(pos), m_radius(radius)
  {
  }
  virtual ~NonhierSphere();
  bool hit(const Ray& ray, double t0, double t1, hit_record& record);

private:
  glm::vec3 m_pos;
  double m_radius;
};

class NonhierBox : public Primitive {
public:
  NonhierBox(const glm::vec3& pos, double size)
    : m_pos(pos), m_size(size)
  {
  }
  NonhierBox(){};
  void setPos(const glm::vec3& pos){
    m_pos = pos;
  };
  void setSize(double size){
    m_size = size;
  };
  
  virtual ~NonhierBox();
  bool hit(const Ray& ray, double t0, double t1, hit_record& record);

private:
  glm::vec3 m_pos;
  double m_size;
};
