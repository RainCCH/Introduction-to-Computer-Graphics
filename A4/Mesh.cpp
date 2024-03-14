// Termm--Fall 2020

#include <iostream>
#include <fstream>

#include <glm/ext.hpp>

// #include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"

#define EPSILON 1e-4

Mesh::Mesh( const std::string& fname )
	: m_vertices()
	, m_faces()
{
	std::string code;
	double vx, vy, vz;
	size_t s1, s2, s3;
	std::string full_path = "Assets/" + fname;

	std::ifstream ifs( full_path.c_str() );
	if( !ifs ) {
		ifs = std::ifstream( fname.c_str() );
		if(!ifs){
			std::cerr << "Could not open " << fname << std::endl;
			return;
		}
	}
	while( ifs >> code ) {
		// std::cout << "code: " << code << std::endl;
		if( code == "v" ) {
			ifs >> vx >> vy >> vz;
			// std::cout << "v " << vx << " " << vy << " " << vz << std::endl;
			m_vertices.push_back( glm::vec3( vx, vy, vz ) );
		} else if( code == "f" ) {
			ifs >> s1 >> s2 >> s3;
			// std::cout << "f " << s1 << " " << s2 << " " << s3 << std::endl;
			m_faces.push_back( Triangle( s1 - 1, s2 - 1, s3 - 1 ) );
		}
	}

	if(m_vertices.size() > 0){
		glm::vec3 min = m_vertices[0];
		glm::vec3 max = m_vertices[0];
		for(const glm::vec3 &v : m_vertices){
			if(v.x < min.x){
				min.x = v.x;
			}
			if(v.y < min.y){
				min.y = v.y;
			}
			if(v.z < min.z){
				min.z = v.z;
			}
			if(v.x > max.x){
				max.x = v.x;
			}
			if(v.y > max.y){
				max.y = v.y;
			}
			if(v.z > max.z){
				max.z = v.z;
			}
		}
		m_bounding_box = NonhierBox(min, glm::length(max - min));
	}
}

Mesh::~Mesh()
{
}

bool Mesh::hit(const Ray& ray, double t0, double t1, hit_record& record)
{
	// std::cout << "Mesh::hit" << "\n";
	// std::cout << "m_face's size: " << m_faces.size() << std::endl;
	// Axis-aligned bounding box
	if(!m_bounding_box.hit(ray, t0, t1, record)){
		// std::cout << "bounding box miss" << std::endl;
		return false;
	}
	#ifdef RENDER_BOUNDING_VOLUMES
	return true;
	#endif

	double the_t = std::numeric_limits<double>::max();
	bool hit = false;
	for(const Triangle &tri : m_faces) {
		// hit |= hitTriangle(ray, t0, t1, record, tri);
		// std::cout << "triangle" << std::endl;
		const glm::vec3 &v0 = m_vertices[tri.v1];
		const glm::vec3 &v1 = m_vertices[tri.v2];
		const glm::vec3 &v2 = m_vertices[tri.v3];
		glm::vec3 e1 = v1 - v0;
		glm::vec3 e2 = v2 - v0;
		glm::vec3 h = glm::cross(ray.getDirection(), e2);
		double a = glm::dot(e1, h);
		// std::cout << "a: " << a << std::endl;
		if (a > -EPSILON && a < EPSILON) {
			continue;
		}
		double f = 1.0 / a;
		glm::vec3 s = ray.getOrigin() - v0;
		double u = f * glm::dot(s, h);
		// std::cout << "u: " << u << std::endl;
		if (u < 0.0 || u > 1.0) {
			continue;
		}
		glm::vec3 q = glm::cross(s, e1);
		double v = f * glm::dot(ray.getDirection(), q);
		// std::cout << "u: " << u << " v: " << v << std::endl;
		if (v < 0.0 || u + v > 1.0) {
			continue;
		}
		double t = f * glm::dot(e2, q);
		// std::cout << "t: " << t << std::endl;
		if(t < t0 || t > t1) {
			continue;
		}
		// std::cout << "hit" << std::endl;
		if(t < the_t){
			the_t = t;
			record.t = t;
			record.p = ray.At(record.t);
			record.normal = glm::normalize(glm::cross(e1, e2));
		}
	}
	if(the_t < std::numeric_limits<double>::max()){
		return true;
	}
	return false;
	// return hit;
}

std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
{
  out << "mesh {";
  /*
  
  for( size_t idx = 0; idx < mesh.m_verts.size(); ++idx ) {
  	const MeshVertex& v = mesh.m_verts[idx];
  	out << glm::to_string( v.m_position );
	if( mesh.m_have_norm ) {
  	  out << " / " << glm::to_string( v.m_normal );
	}
	if( mesh.m_have_uv ) {
  	  out << " / " << glm::to_string( v.m_uv );
	}
  }

*/
  out << "}";
  return out;
}

void Mesh::setPos(const glm::vec3& pos){
  m_bounding_box.setPos(pos);
}

void Mesh::setSize(double size){
  m_bounding_box.setSize(size);
}