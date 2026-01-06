// ============================================================================
// File: Draw.cpp
// 
// Purpose: Implements cdraw_base
// 
// ============================================================================

#include "engine\core\engine.h"
#include "engine\file\file.h"
#include "engine\sys\render\render.h"

cdraw_base::cdraw_base () {
	m_next = nullptr;
}

cdraw_base::~cdraw_base () {
}

void cdraw_base::draw () {
}

cdraw_normal::cdraw_normal ( const char* map_name ) {
	m_meshes = read_mesh_file ( map_name, "Shaders\\vertex_general.glsl", "Shaders\\fragment_general.glsl" );

	m_mtx = glm::mat4 ( 1.0f );
}

cdraw_normal::~cdraw_normal ( ) {
	m_meshes.clear ( );
}

void cdraw_normal::draw ( ) {
	for ( auto& mesh : m_meshes ) {
		cmesh_buffer* pMBuf = mesh.get ( );
		for ( auto& tex : pMBuf->m_used_tex ) {
			ctex_buffer* t = tex.get ( );
			t->m_base_uv = glm::vec2 ( 0.0f );
			t->m_base_color = glm::vec4 ( 1.0f );
		}
		pMBuf->m_mdl = m_mtx;
		pMBuf->draw ( );
	}
}
