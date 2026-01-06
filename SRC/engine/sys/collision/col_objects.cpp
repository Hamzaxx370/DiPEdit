// ============================================================================
// File: col_objects.cpp
// 
// Purpose: Implements the collision objects
// 
// ============================================================================

#include "collision.h"
#include <iostream>

ccol_mesh::~ccol_mesh ( ) {
	if ( m_verts )
		delete [ ] m_verts;
	if ( m_faces )
		delete [ ] m_faces;
}

ccol_capsule::ccol_capsule ( ) {
	m_pos = glm::vec3 ( 0.0f );
	m_vel = glm::vec3 ( 0.0f );
	m_radius = 0.0f;
	m_height = 0.0f;
	m_point0 = glm::vec3 ( 0.0f );
	m_point1 = glm::vec3 ( 0.0f );
};

void ccol_capsule::calculate_points ( ) {
	m_point0 = m_pos + glm::vec3 ( 0.0f, m_radius, 0.0f );
	m_point1 = m_pos + glm::vec3 ( 0.0f, ( m_height - m_radius ), 0.0f );
}