// ============================================================================
// File: components.h
// 
// Purpose: Defines the components entities can use
// 
// ============================================================================

#ifndef __COMPONENT_H_
#define __COMPONENT_H_

#include "glm\glm\gtc\matrix_transform.hpp"
#include "glm\glm\gtc\type_ptr.hpp"

#include "engine\sys\render\render.h"

// Forward declarations
class ccol_capsule;

class ccomponent_anim {
public:
	std::vector <cskel_bone> m_bones;
	cskel_anim* m_anim;

	float m_frame_counter;
	bool m_looped;

	ccomponent_anim ( );
	~ccomponent_anim ( );

	void exec ( );
};

class ccomponent_physics {
public:
	// Movement
	float m_max_speed;
	float m_accel;
	float m_deccel;
	float m_gravity;
	bool m_grounded;
	// Collision
	ccol_capsule* m_capsule;

	// misc
	glm::quat m_rot;

	ccomponent_physics ( );
	~ccomponent_physics ( );

	void exec ( );
};

#endif