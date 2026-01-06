// ============================================================================
// File: map.cpp
// 
// Purpose: Implements the map actor
// 
// ============================================================================

#include "Map.h"

#include "engine\file\file.h"
#include "engine\core\engine.h"
#include "engine\sys\render\light.h"

cact_map::cact_map ( cact_base* p_parent, e_actid actid, const char* map_name ) : cact_base ( p_parent, actid ) {
	char szCol[32];
	char szMdl[32];

	sprintf ( szMdl, "Mdl\\%s.fbx", map_name );
	sprintf ( szCol, "Col\\%s.fbx", map_name );

	m_draw = new cdraw_normal ( szMdl );
	read_col_file ( szCol );

	cengine::get ( )->light_man->regist_light ( new clight ( "L0", glm::vec3 ( 0.0f, 4.0f, -4.0f ), glm::vec3 ( -1.0f, 0.0f, 0.0f ), glm::vec3 ( 0.8f ), 10.0f, 30.0f ) );
	cengine::get ( )->light_man->regist_light ( new clight ( "L1", glm::vec3 ( 0.0f, 4.0f, 4.0f ), glm::vec3 ( -1.0f, 0.0f, 0.0f ), glm::vec3 ( 0.8f ), 10.0f, 30.0f ) );
}

cact_map::~cact_map ( ) {
	delete m_draw;
}

void cact_map::exec0 ( ) {
}

void cact_map::exec1 ( ) {
	cengine::get ( )->render_man->regist_draw ( m_draw, e_draw_type::normal );
}