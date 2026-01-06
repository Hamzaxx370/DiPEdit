// ============================================================================
// File: manager.cpp
// 
// Purpose: Implements the collision manager
// 
// ============================================================================

#include "collision.h"
#include <iostream>

void ccol_manager::regist_col_mesh ( ccol_mesh* p_mesh ) {
	int i;
	for ( i = 0; i < MAX_COL; i++ ) {
		if ( !m_col_meshes [ i ] ) {
			m_col_meshes [ i ] = p_mesh;
			return;
		}
	}
}

void ccol_manager::regist_capsule ( ccol_capsule* p_capsule ) {
	int i;
	for ( i = 0; i < MAX_COL; i++ ) {
		if ( !m_capsules [ i ] ) {
			m_capsules [ i ] = p_capsule;
			return;
		}
	}
}

ccol_manager::ccol_manager ( ) {
	int i;
	for ( i = 0; i < MAX_COL; i++ ) {
		m_col_meshes [ i ] = NULL;
	}
	for ( i = 0; i < MAX_COL; i++ ) {
		m_capsules [ i ] = NULL;
	}
}

ccol_manager::~ccol_manager ( ) {
	clear_cols ( );
}

void ccol_manager::clear_cols ( ) {
	int i;
	for ( i = 0; i < MAX_COL; i++ ) {
		if ( m_col_meshes [ i ] ) {
			delete m_col_meshes [ i ];
		}
	}
	for ( i = 0; i < MAX_COL; i++ ) {
		if ( m_capsules [ i ] ) {
			delete m_capsules [ i ];
		}
	}
}

bool is_point_on_triangle ( const glm::vec3& p, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2 ) {
	glm::vec3 a = v1 - v0;
	glm::vec3 b = v2 - v0;
	glm::vec3 c = p - v0;

	float aa = glm::dot ( a, a );
	float ab = glm::dot ( a, b );
	float bb = glm::dot ( b, b );
	float ca = glm::dot ( c, a );
	float cb = glm::dot ( c, b );

	float d = aa * bb - ab * ab;
	float vd = bb * ca - ab * cb;
	float wd = aa * cb - ab * ca;

	return vd >= 0 && wd >= 0 && vd + wd <= d;
}

// This utility function is correct.
glm::vec3 closest_point_on_triangle ( const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c ) {
	glm::vec3 ab = b - a;
	glm::vec3 ac = c - a;
	glm::vec3 ap = p - a;

	float d1 = glm::dot ( ab, ap );
	float d2 = glm::dot ( ac, ap );
	if ( d1 <= 0.0f && d2 <= 0.0f ) return a;

	glm::vec3 bp = p - b;
	float d3 = glm::dot ( ab, bp );
	float d4 = glm::dot ( ac, bp );
	if ( d3 >= 0.0f && d4 <= d3 ) return b;

	float vc = d1 * d4 - d3 * d2;
	if ( vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f ) {
		float v = d1 / ( d1 - d3 );
		return a + v * ab;
	}

	glm::vec3 cp = p - c;
	float d5 = glm::dot ( ab, cp );
	float d6 = glm::dot ( ac, cp );
	if ( d6 >= 0.0f && d5 <= d6 ) return c;

	float vb = d5 * d2 - d1 * d6;
	if ( vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f ) {
		float w = d2 / ( d2 - d6 );
		return a + w * ac;
	}

	float va = d3 * d6 - d5 * d4;
	if ( va <= 0.0f && ( d4 - d3 ) >= 0.0f && ( d5 - d6 ) >= 0.0f ) {
		float w = ( d4 - d3 ) / ( ( d4 - d3 ) + ( d5 - d6 ) );
		return b + w * ( c - b );
	}

	float denom = 1.0f / ( va + vb + vc );
	float v = vb * denom;
	float w = vc * denom;
	return a + ab * v + ac * w;
}

// This utility function is correct.
glm::vec3 closest_point_on_line_segment ( glm::vec3 a, glm::vec3 b, glm::vec3 point ) {
	glm::vec3 ab = b - a;
	float ab_dot = glm::dot ( ab, ab );
	if ( ab_dot < 1e-6f ) {
		return a; // Line segment is just a point
	}
	// Project point onto the line defined by a and b
	float t = glm::dot ( point - a, ab ) / ab_dot;
	// Clamp t to [0, 1] to stay on the segment
	return a + glm::clamp ( t, 0.0f, 1.0f ) * ab;
}

void ccol_manager::col_check ( ccol_capsule& capsule, std::vector<ccol_info>& cols ) {
	capsule.calculate_points ( );

	// We define the two spheres of the capsule
	glm::vec3 sphere_centers [ 2 ] = { capsule.m_point0, capsule.m_point1 };

	for ( int col_num = 0; col_num < MAX_COL; col_num++ ) {
		ccol_mesh* p_mesh = m_col_meshes [ col_num ];
		if ( !p_mesh ) continue;

		for ( unsigned int face_num = 0; face_num < p_mesh->m_face_num / 3; face_num++ ) {
			glm::vec3 v0 = p_mesh->m_verts [ p_mesh->m_faces [ face_num * 3 + 0 ] ];
			glm::vec3 v1 = p_mesh->m_verts [ p_mesh->m_faces [ face_num * 3 + 1 ] ];
			glm::vec3 v2 = p_mesh->m_verts [ p_mesh->m_faces [ face_num * 3 + 2 ] ];

			// Check both spheres of the capsule
			for ( int i = 0; i < 2; i++ ) {
				glm::vec3 center = sphere_centers [ i ];

				// KEY FIX: Use closest_point_on_triangle instead of plane projection
				glm::vec3 closest = closest_point_on_triangle ( center, v0, v1, v2 );

				glm::vec3 diff = center - closest;
				float dist_sq = glm::dot ( diff, diff );

				// Check if sphere intersects the closest point
				if ( dist_sq < ( capsule.m_radius * capsule.m_radius ) ) {
					float dist = glm::sqrt ( dist_sq );

					ccol_info info;
					info.m_collided = true;
					info.m_hit = closest;

					// Handle case where center is exactly on the triangle
					if ( dist < 1e-6f ) {
						// Fallback to face normal
						info.m_normal = glm::normalize ( glm::cross ( v1 - v0, v2 - v0 ) );
						info.m_depth = capsule.m_radius;
					}
					else {
						info.m_normal = diff / dist;
						info.m_depth = capsule.m_radius - dist;
					}

					cols.push_back ( info );
				}
			}
		}
	}
}

bool ccol_manager::ray_vs_triangle ( const glm::vec3& ray_origin, const glm::vec3& ray_dir,
	const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
	float& out_t ) {
	const float EPSILON = 0.0000001f;

	glm::vec3 edge1 = v1 - v0;
	glm::vec3 edge2 = v2 - v0;
	glm::vec3 h = glm::cross ( ray_dir, edge2 );
	float a = glm::dot ( edge1, h );

	if ( a > -EPSILON && a < EPSILON )
		return false;

	float f = 1.0f / a;
	glm::vec3 s = ray_origin - v0;
	float u = f * glm::dot ( s, h );

	if ( u < 0.0f || u > 1.0f )
		return false;

	glm::vec3 q = glm::cross ( s, edge1 );
	float v = f * glm::dot ( ray_dir, q );

	if ( v < 0.0f || u + v > 1.0f )
		return false;

	float t = f * glm::dot ( edge2, q );

	if ( t > EPSILON ) {
		out_t = t;
		return true;
	}
	return false;
}

// The main raycast function using ccol_info
bool ccol_manager::ray_cast ( const glm::vec3& start, const glm::vec3& dir, float max_dist, ccol_info* p_out_info ) {

	// Reset output info
	p_out_info->m_collided = false;
	p_out_info->m_distance = max_dist;
	p_out_info->m_type = e_col_type::none;

	bool hit_found = false;
	float closest_dist = max_dist;
	glm::vec3 closest_normal ( 0.0f, 1.0f, 0.0f );

	for ( int i = 0; i < MAX_COL; i++ ) {
		ccol_mesh* p_mesh = m_col_meshes [ i ];
		if ( !p_mesh ) continue;

		for ( unsigned int face_num = 0; face_num < p_mesh->m_face_num / 3; face_num++ ) {
			glm::vec3 v0 = p_mesh->m_verts [ p_mesh->m_faces [ face_num * 3 + 0 ] ];
			glm::vec3 v1 = p_mesh->m_verts [ p_mesh->m_faces [ face_num * 3 + 1 ] ];
			glm::vec3 v2 = p_mesh->m_verts [ p_mesh->m_faces [ face_num * 3 + 2 ] ];

			float t = 0.0f;
			if ( ray_vs_triangle ( start, dir, v0, v1, v2, t ) ) {
				if ( t < closest_dist ) {
					closest_dist = t;
					hit_found = true;
					// Calculate normal for this triangle
					closest_normal = glm::normalize ( glm::cross ( v1 - v0, v2 - v0 ) );
				}
			}
		}
	}

	if ( hit_found ) {
		p_out_info->m_collided = true;
		p_out_info->m_distance = closest_dist;
		p_out_info->m_hit = start + ( dir * closest_dist );
		p_out_info->m_normal = closest_normal;
		p_out_info->m_type = e_col_type::none;
		return true;
	}

	return false;
}