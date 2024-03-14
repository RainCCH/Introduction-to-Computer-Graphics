// Termm--Fall 2020

#pragma once

#include "SceneNode.hpp"
#include "Primitive.hpp"
#include "Material.hpp"

class GeometryNode : public SceneNode {
public:
	GeometryNode( const std::string & name, Primitive *prim, 
		Material *mat = nullptr );

	void setMaterial( Material *material );
	bool hit(const Ray& ray, double t0, double t1, hit_record& record);

	Material *m_material;
	Primitive *m_primitive;
};
