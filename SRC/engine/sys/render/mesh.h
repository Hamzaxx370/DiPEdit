// ============================================================================
// File: mesh.h
// 
// Purpose: Defines the mesh structs.
// 
// ============================================================================

#ifndef __MESH_H_
#define __MESH_H_

#include "defs.h"

#include <unordered_map>
#include <string>
#include <thread>
#include <chrono>

#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include "glm\glm\gtc\matrix_transform.hpp"
#include "glm\glm\gtc\type_ptr.hpp"

#include "tex.h"
#include "skel.h"

#define MAX_MDL 1024

// Bit flags for exec_flag
enum class e_mesh_attr {
	none = 0,
	normals = 1 << 0,
	weights = 1 << 1,
	colors = 1 << 2,
	tex = 1 << 3,
};


// Mesh data that can be used by c_draw classes
class cmesh_buffer {
public:
	std::string m_name;

	// Data
	glm::mat4 m_mdl;

	glm::vec3* m_verts;
	unsigned int m_vert_num;
	glm::vec3* m_norms;
	glm::vec4* m_weights;
	glm::vec4* m_vert_colors;
	glm::vec2* m_uvs;

	int* m_faces;
	unsigned int m_face_num;

	std::vector<cskel_bone> m_bones;

	std::vector < ctex_ref > m_used_tex;

	cmesh_buffer ( );
	~cmesh_buffer ( );

	/**
	* @brief Initializes the opengl shaders and ids for a mesh
	* @param vert The vertex shader path
	* @param frag The fragment shader path
	*/
	void set_shaders ( const char* vert, const char* frag );

	/**
	* @brief Initializes opengl buffers for the mesh, by uploading
	* data based on the mesh attributes flag
	*/
	void init_buffers ( );

	/**
	* @brief Draw function for the mesh, draws the mesh based
	* on the mesh attributes flag
	*/
	void draw ( );

	/**
	* @brief Sets a mesh data flag.
	* @param flag The flag to set.
	*/
	void set_flag ( e_mesh_attr flag ) { 
		m_attr |= ( int ) flag;
	};

	/**
	* @brief Checks if a flag is set.
	* @param flag The flag to check.
	*/
	bool check_flag ( e_mesh_attr flag ) {
		return ( m_attr & ( int ) flag );
	};

private:
	// Feature flags
	int m_attr;

	// OpenGL stuff
	unsigned int m_vao;
	unsigned int m_vbo;
	unsigned int m_normal_vbo;
	unsigned int m_weight_vbo;
	unsigned int m_color_vbo;
	unsigned int m_uv_vbo;
	unsigned int m_ebo;
	unsigned int m_shader;
};

class cmesh_ref {
public:
	std::string m_name;

	cmesh_buffer* get ( );

	cmesh_ref ( std::string name ) : m_name ( name ) { };
};

class cmesh_manager {
public:
	cmesh_manager ( );
	~cmesh_manager ( );

	/**
	* @brief Gets a mesh by name
	* @param name The mesh name
	*/
	cmesh_buffer* get_mesh ( std::string name );

	/**
	* @brief Registers a mesh in the mesh manager
	* @param p_buffer Pointer to the mesh
	*/
	void regist_mesh ( cmesh_buffer* p_buffer );

	/**
	* @brief Clears all registered meshes
	*/
	void clear_mesh ( );

private:
	std::unordered_map <std::string, cmesh_buffer*> m_mesh_map;
};

extern glm::mat4 g_view;
extern glm::mat4 g_proj;

#endif