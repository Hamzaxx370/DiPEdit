// ============================================================================
// File: tex.cpp
// 
// Purpose: Implements the texture system
// 
// ============================================================================

#include "tex.h"

#include "engine\core\engine.h"

#include <string>
#include <fstream>
#include <iostream>

ctex_buffer::ctex_buffer ( ) {
	m_name = "NULL";

	m_tex = 0;

	m_width = 0;
	m_height = 0;

	m_buffer = NULL;

	m_base_uv = glm::vec2 ( 0.0f );
	m_uv_scale = glm::vec2 ( 1.0f );
	m_base_color = glm::vec4 ( 0.0f );
}

ctex_buffer::~ctex_buffer ( ) {
	if ( m_buffer ) {
		delete [ ] m_buffer;
	}

	glDeleteTextures ( 1, &m_tex );
}

void ctex_buffer::init_buffer ( ) {
	glGenTextures ( 1, &m_tex );

	glActiveTexture ( GL_TEXTURE0 );
	glBindTexture ( GL_TEXTURE_2D, m_tex );

	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 );

	glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_buffer );
	//GenerateMipmap(GL_TEXTURE_2D);

	glBindTexture ( GL_TEXTURE_2D, 0 );
}

void ctex_buffer::draw ( int dwShader, int dwIndex ) {
	glActiveTexture ( GL_TEXTURE0 + dwIndex );
	glBindTexture ( GL_TEXTURE_2D, m_tex );

	std::string Name = "tex[" + std::to_string ( dwIndex ) + "]";
	int loc = glGetUniformLocation ( dwShader, Name.c_str ( ) );
	glUniform1i ( loc, dwIndex );

	loc = glGetUniformLocation ( dwShader, "uvBase" );
	glUniform2f ( loc, m_base_uv.x, m_base_uv.y );
	loc = glGetUniformLocation ( dwShader, "uvScale" );
	glUniform2f ( loc, m_uv_scale.x, m_uv_scale.y );
	loc = glGetUniformLocation ( dwShader, "colorBase" );
	glUniform4f ( loc, m_base_color.r, m_base_color.g, m_base_color.b, m_base_color.a );
}

ctex_buffer* ctex_ref::get ( ) {
	return cengine::get ( )->tex_man->get_tex ( m_name );
}

ctex_manager::ctex_manager ( ) {
}
ctex_manager::~ctex_manager ( ) {
	for ( auto& tex : m_tex_map ) {
		delete tex.second;
	}
	m_tex_map.clear ( );
}

ctex_buffer* ctex_manager::get_tex ( std::string name ) {
	auto it = m_tex_map.find ( name );
	if ( it != m_tex_map.end ( ) ) {
		return it->second;
	}
	return nullptr;
}

void ctex_manager::regist_tex ( ctex_buffer* p_buffer ) {
	auto it = m_tex_map.find ( p_buffer->m_name );

	if ( it == m_tex_map.end ( ) ) {
		m_tex_map.emplace ( p_buffer->m_name, p_buffer );
	}
	else {
		delete p_buffer;
	}
}

void ctex_manager::clear_tex ( ) {
	for ( auto& tex : m_tex_map ) {
		if ( tex.second )
			delete tex.second;
	}
	m_tex_map.clear ( );
}