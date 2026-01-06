// ============================================================================
// File: mesh.cpp
// 
// Purpose: Implements the mesh system
// 
// ============================================================================

#include "mesh.h"

#include "engine\core\engine.h"

#include <string>
#include <fstream>
#include <iostream>

#include "light.h"

static float s_fColor [ 4 ] = { 1.0, 1.0, 1.0, 1.0 };

std::string LoadShaderSource ( const char* path )
{
	std::ifstream in ( path, std::ios::binary );
	if ( in )
	{
		std::string contents;
		in.seekg ( 0, std::ios::end );
		contents.resize ( in.tellg ( ) );
		in.seekg ( 0, std::ios::beg );
		in.read ( &contents [ 0 ], contents.size ( ) );
		return contents;
	};
	std::cout << "No workie\n";
	return "";
}

cmesh_buffer::cmesh_buffer ( ) {
	m_name = "NULL";

	m_attr = 0;

	m_vao = 0;
	m_vbo = 0;
	m_normal_vbo = 0;
	m_weight_vbo = 0;
	m_uv_vbo = 0;
	m_color_vbo = 0;
	m_ebo = 0;
	m_shader = 0;

	m_mdl = glm::mat4 ( 1.0 );
	m_verts = NULL;
	m_faces = NULL;
	m_norms = NULL;
	m_weights = NULL;
	m_vert_colors = NULL;
	m_uvs = NULL;

	m_vert_num = 0;
	m_face_num = 0;
}

cmesh_buffer::~cmesh_buffer ( ) {
	if ( m_verts )
		delete [ ] m_verts;
	if ( m_faces )
		delete [ ] m_faces;
	if ( m_norms )
		delete [ ] m_norms;
	if ( m_weights )
		delete [ ] m_weights;
	if ( m_vert_colors )
		delete [ ] m_vert_colors;
	if ( m_uvs )
		delete [ ] m_uvs;

	glDeleteProgram ( m_shader );

	// Vertex array object
	glDeleteVertexArrays ( 1, &m_vao );

	// Vertex buffer
	glDeleteBuffers ( 1, &m_vbo );

	// Face buffer
	glDeleteBuffers ( 1, &m_ebo );

	// Extra features
	if ( check_flag ( e_mesh_attr::normals ) ) {
		glDeleteBuffers ( 1, &m_normal_vbo );
	}
	if ( check_flag ( e_mesh_attr::weights ) ) {
		glDeleteBuffers ( 1, &m_weight_vbo );
	}
	if ( check_flag ( e_mesh_attr::colors ) ) {
		glDeleteBuffers ( 1, &m_color_vbo );
	}
	if ( check_flag ( e_mesh_attr::tex ) ) {
		glDeleteBuffers ( 1, &m_uv_vbo );
	}
}

void cmesh_buffer::set_shaders ( const char* szVert, const char* szFrag ) {
	// Initialize Shaders
	int VertexShaderTest = glCreateShader ( GL_VERTEX_SHADER );
	std::string vertex_source = LoadShaderSource ( szVert );
	const char* source = vertex_source.c_str ( );
	glShaderSource ( VertexShaderTest, 1, &source, NULL );
	glCompileShader ( VertexShaderTest );

	// Check errors
	int iSuccess;
	glGetShaderiv ( VertexShaderTest, GL_COMPILE_STATUS, &iSuccess );
	if ( !iSuccess ) {
		char cLog [ 1024 ];
		glGetShaderInfoLog ( VertexShaderTest, 1024, NULL, cLog );
		printf ( "Compile Shader Error: %s\n", cLog );
	}

	int FragmentShaderTest = glCreateShader ( GL_FRAGMENT_SHADER );
	std::string fragment_source = LoadShaderSource ( szFrag );
	source = fragment_source.c_str ( );
	glShaderSource ( FragmentShaderTest, 1, &source, NULL );
	glCompileShader ( FragmentShaderTest );

	// Check errors
	glGetShaderiv ( FragmentShaderTest, GL_COMPILE_STATUS, &iSuccess );
	if ( !iSuccess ) {
		char cLog [ 1024 ];
		glGetShaderInfoLog ( FragmentShaderTest, 1024, NULL, cLog );
		printf ( "Compile Shader Error: %s\n", cLog );
	}

	m_shader = glCreateProgram ( );
	glAttachShader ( m_shader, VertexShaderTest );
	glAttachShader ( m_shader, FragmentShaderTest );
	glLinkProgram ( m_shader );

	// Clean up
	glDeleteShader ( VertexShaderTest );
	glDeleteShader ( FragmentShaderTest );
}

void cmesh_buffer::init_buffers ( ) {
	// Vertex array object
	glGenVertexArrays ( 1, &m_vao );
	glBindVertexArray ( m_vao );

	// Vertex buffer
	glGenBuffers ( 1, &m_vbo );
	glBindBuffer ( GL_ARRAY_BUFFER, m_vbo );
	glBufferData ( GL_ARRAY_BUFFER, m_vert_num * sizeof ( glm::vec3 ), m_verts, GL_STATIC_DRAW );
	glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof ( float ), ( void* ) 0 );
	glEnableVertexAttribArray ( 0 );

	// Face buffer
	glGenBuffers ( 1, &m_ebo );
	glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, m_ebo );
	glBufferData ( GL_ELEMENT_ARRAY_BUFFER, m_face_num * sizeof ( int ), m_faces, GL_STATIC_DRAW );

	// Extra features
	if ( check_flag ( e_mesh_attr::normals ) ) {
		glGenBuffers ( 1, &m_normal_vbo );
		glBindBuffer ( GL_ARRAY_BUFFER, m_normal_vbo );
		glBufferData ( GL_ARRAY_BUFFER, m_vert_num * sizeof ( glm::vec3 ), m_norms, GL_STATIC_DRAW );
		glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof ( float ), ( void* ) 0 );
		glEnableVertexAttribArray ( 1 );

	}

	if ( check_flag ( e_mesh_attr::weights ) ) {
		glGenBuffers ( 1, &m_weight_vbo );
		glBindBuffer ( GL_ARRAY_BUFFER, m_weight_vbo );
		glBufferData ( GL_ARRAY_BUFFER, m_vert_num * sizeof ( glm::vec4 ), m_weights, GL_STATIC_DRAW );
		glVertexAttribPointer ( 2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof ( float ), ( void* ) 0 );
		glEnableVertexAttribArray ( 2 );
	}

	if ( check_flag ( e_mesh_attr::colors ) ) {
		glGenBuffers ( 1, &m_color_vbo );
		glBindBuffer ( GL_ARRAY_BUFFER, m_color_vbo );
		glBufferData ( GL_ARRAY_BUFFER, m_vert_num * sizeof ( glm::vec4 ), m_vert_colors, GL_STATIC_DRAW );
		glVertexAttribPointer ( 4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof ( float ), ( void* ) 0 );
		glEnableVertexAttribArray ( 4 );
	}

	if ( check_flag ( e_mesh_attr::tex ) ) {
		glGenBuffers ( 1, &m_uv_vbo );
		glBindBuffer ( GL_ARRAY_BUFFER, m_uv_vbo );
		glBufferData ( GL_ARRAY_BUFFER, m_vert_num * sizeof ( glm::vec2 ), m_uvs, GL_STATIC_DRAW );
		glVertexAttribPointer ( 3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof ( float ), ( void* ) 0 );
		glEnableVertexAttribArray ( 3 );
	}

	glBindBuffer ( GL_ARRAY_BUFFER, 0 );
	glBindVertexArray ( 0 );
}

void cmesh_buffer::draw ( ) {
	glUseProgram ( m_shader );

	if ( check_flag ( e_mesh_attr::tex ) ) {
		int t = 0;
		for ( auto& tex : m_used_tex ) {
			tex.get ( )->draw ( m_shader, t );
			t++;
		}
	}

	glUniformMatrix4fv ( glGetUniformLocation ( m_shader, "model" ), 1, GL_FALSE, glm::value_ptr ( m_mdl ) );

	glUniformMatrix4fv ( glGetUniformLocation ( m_shader, "view" ), 1, GL_FALSE, glm::value_ptr ( g_view ) );

	glUniformMatrix4fv ( glGetUniformLocation ( m_shader, "proj" ), 1, GL_FALSE, glm::value_ptr ( g_proj ) );

	glUniform1i ( glGetUniformLocation ( m_shader, "LightCount" ), 0 );
	cengine::get ( )->light_man->upload_lights ( m_shader );

	glUniform1i ( glGetUniformLocation ( m_shader, "hasNormals" ), check_flag ( e_mesh_attr::normals ) != 0 );
	glUniform1i ( glGetUniformLocation ( m_shader, "hasWeights" ), check_flag ( e_mesh_attr::weights ) != 0 );
	glUniform1i ( glGetUniformLocation ( m_shader, "hasUVs" ), check_flag ( e_mesh_attr::tex ) != 0 );
	glUniform1i ( glGetUniformLocation ( m_shader, "hasColors" ), check_flag ( e_mesh_attr::colors ) != 0 );

	if ( check_flag ( e_mesh_attr::weights ) ) {
		int b = 0;
		glm::mat4 finals [ 100 ];

		for ( auto& bone : m_bones ) {
			finals [ b ] = m_bones [ b ].m_final;
			b++;
		}

		glUniformMatrix4fv ( glGetUniformLocation ( m_shader, "finalBonesMatrices" ), b, GL_FALSE, glm::value_ptr ( finals [ 0 ] ) );

	}

	//Bind arrays and render them
	glBindVertexArray ( m_vao );
	glDrawElements ( GL_TRIANGLE_STRIP, m_face_num, GL_UNSIGNED_INT, 0 );
	glBindVertexArray ( 0 );
}

cmesh_buffer* cmesh_ref::get ( ) {
	return cengine::get ( )->mesh_man->get_mesh ( m_name );
}

cmesh_manager::cmesh_manager ( ) {
}

cmesh_manager::~cmesh_manager ( ) {
	clear_mesh ( );
}

cmesh_buffer* cmesh_manager::get_mesh ( std::string name ) {
	auto it = m_mesh_map.find ( name );
	if ( it != m_mesh_map.end ( ) ) {
		return it->second;
	}
	return nullptr;
}

void cmesh_manager::regist_mesh ( cmesh_buffer* p_buffer ) {
	auto it = m_mesh_map.find ( p_buffer->m_name );

	if ( it == m_mesh_map.end ( ) ) {
		m_mesh_map.emplace ( p_buffer->m_name, p_buffer );
	}
	else {
		delete p_buffer;
	}
}

void cmesh_manager::clear_mesh ( ) {
	for ( auto& mesh : m_mesh_map ) {
		if ( mesh.second )
			delete mesh.second;
	}
	m_mesh_map.clear ( );
}