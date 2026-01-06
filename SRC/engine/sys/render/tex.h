// ============================================================================
// File: tex.h
// 
// Purpose: Defines the texture structs.
// 
// ============================================================================

#ifndef __TEX_H_
#define __TEX_H_

#include "defs.h"

#include <unordered_map>
#include <string>
#include <thread>
#include <chrono>

#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include "glm\glm\gtc\matrix_transform.hpp"
#include "glm\glm\gtc\type_ptr.hpp"

#define MAX_TEX 1024

class ctex_buffer {
public:
	std::string m_name;
	// OpenGL stuff
	unsigned int m_tex;

	// Data
	int m_width;
	int m_height;
	unsigned char* m_buffer;

	glm::vec2 m_base_uv;
	glm::vec2 m_uv_scale;
	glm::vec4 m_base_color;

	ctex_buffer ( );
	~ctex_buffer ( );

	/**
	* @brief Initializes opengl buffers
	*/
	void init_buffer ( );

	/**
	* @brief Drawing function for the texture
	*/
	void draw ( int shader, int index );
};

class ctex_ref {
public:
	std::string m_name;

	ctex_buffer* get ( );

	ctex_ref ( std::string name ) : m_name ( name ) { };
};

class ctex_manager {
public:
	std::unordered_map <std::string, ctex_buffer*> m_tex_map;

	ctex_manager ( );
	~ctex_manager ( );

	/**
	* @brief Gets texture by name
	* @param name The texture name
	*/
	ctex_buffer* get_tex ( std::string name );

	/**
	* @brief Registers a texture to the texture manager
	* @param p_buffer Pointer to the texture
	*/
	void regist_tex ( ctex_buffer* p_buffer );

	/**
	* @brief Clears all registed textures
	*/
	void clear_tex ( );
};

#endif