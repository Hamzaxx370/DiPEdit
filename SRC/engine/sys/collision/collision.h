// ============================================================================
// File: collision.h
// 
// Purpose: Defines the collision system class, which is used for collision maths
// 
// ============================================================================

#ifndef __COLLISION_H_
#define __COLLISION_H_

#include "defs.h"

#include "glm\glm\gtc\matrix_transform.hpp"
#include "glm\glm\gtc\type_ptr.hpp"
#include <vector>


#define MAX_COL 1024

enum class e_col_type : int {
	none,
};

enum class e_col_priority : int {
	low,
	mid,
	high
};

class ccol_info {
public:
	e_col_type m_type;
	e_col_priority m_priority;
	
	bool m_collided;
	
	float m_distance;
	glm::vec3 m_hit;
	glm::vec3 m_normal;

	float m_time;
	float m_depth;

	ccol_info ( ) {
		m_type = e_col_type::none;
		m_priority = e_col_priority::low;

		m_collided = false;
		
		m_distance = -1.0f;
		
		m_hit = glm::vec3 ( 0.0 );
		m_normal = glm::vec3 ( 0.0f );

		m_time = 1.0f;
		m_depth = 0.0f;
	}
};

class ccol_capsule {
public:
	glm::vec3 m_pos;
	glm::vec3 m_vel;
	float m_radius;
	float m_height;
	glm::vec3 m_point0;
	glm::vec3 m_point1;

	ccol_capsule ( );
	void calculate_points ( );
};

class ccol_mesh {
public:
	glm::vec3* m_verts;
	unsigned int m_vert_num;

	int* m_faces;
	unsigned int m_face_num;

	ccol_mesh ( ) { };
	~ccol_mesh ( );
};

class ccol_manager {
public:
	ccol_manager ( );
	~ccol_manager ( );

	/**
	* @brief Register collision mesh in the collision list
	* @param p_mesh Mesh to register
	*/
	void regist_col_mesh ( ccol_mesh* p_mesh );

	/**
	* @brief Register collision capsule in the collision list
	* @param p_capsule Capsule to register
	*/
	void regist_capsule ( ccol_capsule* p_capsule );

	/**
	* @brief Clear the collision list
	*/
	void clear_cols ( );

	/**
	* @brief Check collisions for a capsule against all registered meshes
	* @param capsule capsule to check
	* @param cols vector to store collision info results
	*/
	void col_check ( ccol_capsule& capsule, std::vector<ccol_info>& cols );

	/**
	* @brief Ray-cast check against a triangle
	* @param ray_origin The position from which the ray is cast
	* @param ray_dir The direction to cast to
	* @param v0 Vertex 0 of the triangle
	* @param v1 Vertex 1 of the triangle
	* @param v2 Vertex 2 of the triangle
	* @param out_t Output variable for the distance to the hit point
	*/
	bool ray_vs_triangle ( const glm::vec3& ray_origin, const glm::vec3& ray_dir,
		const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
		float& out_t );

	/**
	* @brief Ray-cast check against all collision meshes
	* @param start The position from which the ray is cast
	* @param dir The direction to cast to
	* @param max_dist The maximum distance for the ray cast
	* @param p_out_info Output variable for the collision info
	*/
	bool ray_cast ( const glm::vec3& start, const glm::vec3& dir, float max_dist, ccol_info* p_out_info );
private:
	ccol_mesh* m_col_meshes [ MAX_COL ];
	ccol_capsule* m_capsules [ MAX_COL ];
};

#endif