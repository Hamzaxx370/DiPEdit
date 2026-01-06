// ============================================================================
// File: entity.h
// 
// Purpose: Defines the entity system, containing the entity cdraw_base class, entity
// components, and entity controllers and their managers.
// An entity controller (derived from cact_entity_ctrl) is created along side an
// entity (cact_entity) and initializes its components, as well as controls and
// manages the entity each frame.
// The entity is just the game object, without a controller it freezes.
// The components are systems the entity can use, initialized as nullptr and only 
// created if the controller needs them.
// 
// ============================================================================

#ifndef __ENTITY_H_
#define __ENTITY_H_

#include "engine\sys\actor\actor.h"

#include "glm\glm\gtc\matrix_transform.hpp"
#include "glm\glm\gtc\type_ptr.hpp"


// Forward declarations
class cmesh_buffer;
class ccomponent_physics;
class ccomponent_anim;
class cdraw_normal;

// Entity data and state
class cact_entity : public cact_base {
public:
	cdraw_normal* m_draw;
	int m_type_id;

	// Components
	ccomponent_anim* m_animator;
	ccomponent_physics* m_physics;

	cact_entity ( cact_base* p_parent, e_actid actid );
	~cact_entity ( );

	void exec0 ( ) override;
	void exec1 ( ) override;
};

// Entity controller and ai
class cact_entity_ctrl : public cact_base {
public:
	// Entity ID
	e_actid m_entity_id;

	cact_entity_ctrl ( cact_base* p_parent, e_actid actid, e_actid entity_id );
	~cact_entity_ctrl ( );

	void exec0 ( ) override;
	void exec1 ( ) override;
};

class cact_entity_manager : public cact_base {
public:
	cact_entity_manager ( cact_base* p_parent, e_actid actid );
	~cact_entity_manager ( );

	void exec0 ( ) override;
	void exec1 ( ) override;

	e_actid get_closest_entity ( glm::vec3 point, e_actid pass = ( e_actid ) -1 );
	glm::vec3 get_closest_entity_pos ( glm::vec3 point, e_actid pass = ( e_actid ) -1 );
};

class cact_entity_ctrl_manager : public cact_base {
public:
	cact_entity_ctrl_manager ( cact_base* p_parent, e_actid actid );
	~cact_entity_ctrl_manager ( );

	void exec0 ( ) override;
	void exec1 ( ) override;
};

#endif