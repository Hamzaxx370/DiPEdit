// ============================================================================
// File: light.h
// 
// Purpose: Defines the lighting system.
// 
// ============================================================================

#ifndef __LIGHT_H_
#define __LIGHT_H_

#include "defs.h"

#include <unordered_map>
#include <string>
#include <thread>
#include <chrono>

#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include "glm\glm\gtc\matrix_transform.hpp"
#include "glm\glm\gtc\type_ptr.hpp"

#define MAX_LIGHT 100
#define SHADOW_H 1024
#define SHADOW_W 1024

class clight {
public:
	std::string m_name;
	glm::vec3 m_pos;
	glm::vec3 m_dir;
	glm::vec3 m_color;
	float m_intensity;
	float m_radius;

	clight ( std::string lname, glm::vec3 pos, glm::vec3 dir, glm::vec3 color, float intensity, float radius ) :
		m_name ( lname ),
		m_pos ( pos ),
		m_dir ( dir ),
		m_color ( color ),
		m_intensity ( intensity ),
		m_radius ( radius ) {
	}
};

class clight_manager {
public:
	clight_manager ( );
	~clight_manager ( );

	/**
	* @brief Registers a light to the light manager
	* @param p_light Pointer to the light
	*/
	void regist_light ( clight* p_light );

	/**
	* @brief Clears all lights from the light manager
	*/
	void clear_lights ( );

	/**
	* @brief Uploads lights to a shader
	* @param shader The shader id to upload lights to
	*/
	void upload_lights ( int shader );
private:
	std::unordered_map <std::string, clight*> m_light_map;
};

#endif