// ============================================================================
// File: root.cpp
// 
// Purpose: Implements the game actor
// 
// ============================================================================

#include "root.h"

#include "game\core\game.h"
#include "game\file\file.h"
#include "camera.h"
#include "dummy.h"
#include "engine\core\components.h"

// Just make the camera actor for the viewer
cact_game::cact_game ( cact_base* p_parent, e_actid actid ) : cact_base ( p_parent, actid ) {
	cact_camera* p_camera = new cact_camera ( this, e_actid::camera );
	p_camera->m_cam_view = e_cam_view::orbit;
	new cact_base ( this, e_actid::particle_manager );

	// Make a dummy entity
	//cact_entity* entity = new cact_dummy ( this, e_actid::dummy, 643 );
}

cact_game::~cact_game ( ) {
}

void cact_game::exec0 ( ) {

}

void cact_game::exec1 ( ) {

}