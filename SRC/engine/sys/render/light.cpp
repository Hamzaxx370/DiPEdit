// ============================================================================
// File: Render.cpp
// 
// Purpose: Implements the rendering system
// 
// ============================================================================

#include "light.h"

#include "engine\core\engine.h"

#include <string>
#include <fstream>
#include <iostream>

clight_manager::clight_manager ( ) {

	/*
	glGenFramebuffers ( 1, &dwDepthFBO );

	glGenTextures ( 1, &dwDepthMap );
	glBindTexture ( GL_TEXTURE_2D, dwDepthMap );
	
	glTexImage2D ( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_W, SHADOW_H, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
	
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	glBindFramebuffer ( GL_FRAMEBUFFER, dwDepthFBO );
	glFramebufferTexture2D ( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dwDepthMap, 0 );
	glDrawBuffer ( GL_NONE );
	glReadBuffer ( GL_NONE );
	glBindFramebuffer ( GL_FRAMEBUFFER, 0 );
	*/
}

clight_manager::~clight_manager ( ) {
	clear_lights ( );
}

void clight_manager::regist_light ( clight* p_light ) {
	if ( !m_light_map [ p_light->m_name ] ) {
		m_light_map [ p_light->m_name ] = p_light;
	}
	else {
		delete p_light;
	}
}

void clight_manager::clear_lights ( ) {
	for ( auto& light : m_light_map ) {
		delete light.second;
	}
	m_light_map.clear ( );
}

void clight_manager::upload_lights ( int shader ) {
	int dwLightCount = 0;

	for ( auto& light_var : m_light_map ) {
		clight* light = light_var.second;

		char path [ 32 ] = "Lights[99]";
		sprintf_s ( path, "Lights[%d]", dwLightCount );

		glUniform3fv ( glGetUniformLocation ( shader, ( std::string ( path ) + ".pos" ).c_str ( ) ), 1, &light->m_pos [ 0 ] );
		glUniform3fv ( glGetUniformLocation ( shader, ( std::string ( path ) + ".color" ).c_str ( ) ), 1, &light->m_color [ 0 ] );
		glUniform3fv ( glGetUniformLocation ( shader, ( std::string ( path ) + ".dir" ).c_str ( ) ), 1, &light->m_dir [ 0 ] );
		glUniform1f ( glGetUniformLocation ( shader, ( std::string ( path ) + ".intensity" ).c_str ( ) ), light->m_intensity );
		glUniform1f ( glGetUniformLocation ( shader, ( std::string ( path ) + ".radius" ).c_str ( ) ), light->m_radius );

		dwLightCount++;
	}
	glUniform1i ( glGetUniformLocation ( shader, "LightCount" ), dwLightCount );
	
}