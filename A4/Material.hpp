// Termm--Fall 2020

#pragma once

enum MaterialType {
  PHONG
};

class Material {
public:
  Material(MaterialType type);
  virtual ~Material();
  MaterialType type;

protected:
  Material();
};
