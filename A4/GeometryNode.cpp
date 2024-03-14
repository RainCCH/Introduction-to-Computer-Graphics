// Termm--Fall 2020

#include "GeometryNode.hpp"

using namespace std;
using namespace glm;

//---------------------------------------------------------------------------------------
GeometryNode::GeometryNode(
	const std::string & name, Primitive *prim, Material *mat )
	: SceneNode( name )
	, m_material( mat )
	, m_primitive( prim )
{
	m_nodeType = NodeType::GeometryNode;
}

void GeometryNode::setMaterial( Material *mat )
{
	// Obviously, there's a potential memory leak here.  A good solution
	// would be to use some kind of reference counting, as in the 
	// C++ shared_ptr.  But I'm going to punt on that problem here.
	// Why?  Two reasons:
	// (a) In practice we expect the scene to be constructed exactly
	//     once.  There's no reason to believe that materials will be
	//     repeatedly overwritten in a GeometryNode.
	// (b) A ray tracer is a program in which you compute once, and 
	//     throw away all your data.  A memory leak won't build up and
	//     crash the program.

	m_material = mat;
}

bool GeometryNode::hit(const Ray& ray, double t0, double t1, hit_record& record)
{
	// std::cout << "GeometryNode::hit function called" << std::endl;
	vec3 A = vec3(invtrans * vec4(ray.getOrigin(), 1.0f));
	vec3 B = vec3(invtrans * vec4(ray.getDirection(), 0.0f));

	Ray transformedRay(A, B);
	double closest_t = t1;
	hit_record closest_record;
	bool hitAnything = false;

    // Check intersection with the primitive
    if (m_primitive->hit(transformedRay, t0, closest_t, closest_record)) {
        hitAnything = true;
        closest_t = closest_record.t;  // Update closest_t
        closest_record.material = m_material;  // Set the material
    }
	// hit_record child_record;
    // for (auto child : children) {
    //     if (child->hit(transformedRay, t0, closest_t, child_record)) {
    //         if (child_record.t < closest_t) {
    //             closest_t = child_record.t;  // Update closest_t
    //             closest_record = child_record;  // Update the closest hit record
    //             hitAnything = true;
    //         }
    //     }
    // }

	// for(auto child: children){
	// 	// A = vec3(get_inverse() * vec4(transformedRay.getOrigin(), 1.0f));
	// 	// B = vec3(invtrans * vec4(ray.getDirection(), 0.0f));
	// 	bool hit_this = child->hit(transformedRay, t0, closest_t, tmp_record);
	// 	if (hit_this){
	// 		closest_t = tmp_record.t;
	// 		record = tmp_record;
	// 		hit = true;
	// 	}
	// }
	if(hitAnything){
		record = closest_record;  // Set the record to the closest hit
        record.p = vec3(get_transform() * vec4(record.p, 1.0f));  // Transform intersection point to world space
        vec4 normal4 = glm::transpose(invtrans) * vec4(record.normal, 0.0f);
        record.normal = normalize(vec3(normal4));
	}
	return hitAnything;
}
