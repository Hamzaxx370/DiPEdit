// ============================================================================
// File: entity.cpp
// 
// Purpose: Implements the entity system
// 
// ============================================================================

#include "entity.h"
#include "engine\core\components.h"

#include "engine\core\engine.h"
#include "engine\sys\render\render.h"
#include "engine\sys\collision\collision.h"

cact_entity::cact_entity ( cact_base* p_parent, e_actid actid ) : cact_base ( p_parent, actid ) {
	m_draw = nullptr;
	m_physics = nullptr;
	m_animator = nullptr;
	m_type_id = -1;
}

cact_entity::~cact_entity ( ) {
	if ( m_physics )
		delete m_physics;
	if ( m_animator )
		delete m_animator;
	if ( m_draw )
		delete m_draw;
}

void cact_entity::exec0 ( ) {
}

void cact_entity::exec1 ( ) {
	if ( m_physics )
		m_physics->exec ( );

	if ( m_animator )
		m_animator->exec ( );

	if ( m_draw ) {
		if ( m_animator ) {
			for ( auto& ref : m_draw->m_meshes ) {
				ref.get ( )->m_bones = m_animator->m_bones;
			}
		}
		cengine::get ( )->render_man->regist_draw ( m_draw, e_draw_type::normal );
	}
}

cact_entity_ctrl::cact_entity_ctrl ( cact_base* p_parent, e_actid actid, e_actid _entity_id ) : cact_base ( p_parent, actid ) {
	m_entity_id = _entity_id;
}

cact_entity_ctrl::~cact_entity_ctrl ( ) {

}

void cact_entity_ctrl::exec0 ( ) {
}

void cact_entity_ctrl::exec1 ( ) {
}


cact_entity_manager::cact_entity_manager ( cact_base* p_parent, e_actid actid ) : cact_base ( p_parent, actid ) {

}

cact_entity_manager::~cact_entity_manager ( ) {

}

void cact_entity_manager::exec0 ( ) {
}

void cact_entity_manager::exec1 ( ) {
}

e_actid cact_entity_manager::get_closest_entity ( glm::vec3 point, e_actid pass ) {
	std::vector <glm::vec3> positions;
	cact_entity* current = ( cact_entity* ) m_child;
	float diff = FLT_MAX;
	e_actid closest = ( e_actid ) - 1;
	while ( current ) {
		if ( current->m_physics && current->m_act_id != pass ) {
			glm::vec3 pos = current->m_physics->m_capsule->m_pos;
			float cur_diff = glm::distance ( point, pos );
			if ( cur_diff < diff ) {
				diff = cur_diff;
				closest = current->m_act_id;
			}
		}
		current = ( cact_entity* ) current->m_sibling;
	}
	return closest;
};

glm::vec3 cact_entity_manager::get_closest_entity_pos ( glm::vec3 point, e_actid pass ) {
	std::vector <glm::vec3> positions;
	cact_entity* current = ( cact_entity* ) m_child;
	float diff = FLT_MAX;
	glm::vec3 closest = glm::vec3(0.0f);
	while ( current ) {
		if ( current->m_physics && current->m_act_id != pass ) {
			glm::vec3 pos = current->m_physics->m_capsule->m_pos;
			float cur_diff = glm::distance ( point, pos );
			if ( cur_diff < diff ) {
				diff = cur_diff;
				closest = pos;
			}
		}
		current = ( cact_entity* ) current->m_sibling;
	}
	return closest;
};

cact_entity_ctrl_manager::cact_entity_ctrl_manager ( cact_base* p_parent, e_actid actid ) : cact_base ( p_parent, actid ) {
}

cact_entity_ctrl_manager::~cact_entity_ctrl_manager ( ) {

}

void cact_entity_ctrl_manager::exec0 ( ) {
}

void cact_entity_ctrl_manager::exec1 ( ) {
}