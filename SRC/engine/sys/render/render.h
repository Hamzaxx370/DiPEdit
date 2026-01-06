// ============================================================================
// File: render.h
// 
// Purpose: Defines the rendering system. currently only supports opengl
// 
// ============================================================================

#ifndef __RENDER_H_
#define __RENDER_H_

#include "defs.h"

#include <unordered_map>
#include <string>
#include <thread>
#include <chrono>

#include <glad\glad.h>
#include <GLFW\glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW\glfw3native.h>
#include "glm\glm\gtc\matrix_transform.hpp"
#include "glm\glm\gtc\type_ptr.hpp"

#include "mesh.h"

static inline void check_gl_err ( const char* szFile, int iLine ) {
	int iErr;
	while ( ( iErr = glGetError ( ) ) != GL_NO_ERROR ) {
		printf ( "Runtime Error: %x, File: %s, Line: %d\n", iErr, szFile, iLine );
	}
}

enum class e_draw_type : int {
	normal,
	particle,
	ui,
	num
};

// Rendered object class, exists for wanting to make completely
// unique drawing
class cdraw_base {
public:
	cdraw_base ( );
	virtual ~cdraw_base ( );

private:
	cdraw_base* m_next;

	/**
	* @brief The drawing function for a drawable
	*/
	virtual void draw ( );

	friend class crender_manager;
};

// Draw class with basic features
class cdraw_normal : public cdraw_base {
public:
	glm::mat4 m_mtx;
	std::vector < cmesh_ref > m_meshes;
	cdraw_normal ( const char* map_name );
	cdraw_normal ( ) {
		m_mtx = glm::mat4 ( 1.0f );
	};
	~cdraw_normal ( );

	/**
	* @brief The drawing function for a basic drawable that
	* draws its registered meshes
	*/
	void draw ( ) override;
};

class crender_manager {
public:
	crender_manager ( const char* title, int width, int height );
	virtual ~crender_manager ( ) {
		glfwDestroyWindow ( m_window );
		glfwTerminate ( );
	};

	/**
	* @brief Gets input, calculates delta, and clears screen
	*/
	virtual void exec0 ( );

	/**
	* @brief Sets width and swaps buffers
	*/
	virtual void exec1 ( );

	/**
	* @brief Registers a drawable for this frame
	* @param p_draw Pointer to the drawable to register
	* @param type The type of drawable to register it as
	*/
	virtual void regist_draw ( cdraw_base* p_draw, e_draw_type type );

	/**
	* @brief Checks if the window should close
	*/
	virtual bool should_close ( );

	/**
	* @brief Checks if the window is in focus
	*/
	virtual bool is_window_focus ( );

	/**
	* @brief Sets the cursor lock
	* @param val GLFW enum value for the property
	*/
	virtual void set_cursor_lock ( int val );

	/**
	* @brief Gets cursor pos
	* @param x Output variable for x pos
	* @param y Output variable for y pos
	*/
	virtual void get_cursor_pos ( double* x, double* y );

	/**
	* @brief Sets cursor pos
	* @param x New pos x
	* @param y New pos y
	*/
	virtual void set_cursor_pos ( double x, double y );

	/**
	* @brief Checks if a key is held in this frame
	* @param val GLFW enum for the key
	*/
	virtual bool is_key_held ( int val );

	/**
	* @brief Checks if a key was just pressed in this frame
	* @param val GLFW enum for the key
	*/
	virtual bool is_key_just_pressed ( int val );

	/**
	* @brief Checks if a key was just released in this frame
	* @param val GLFW enum for the key
	*/
	virtual bool is_key_just_released ( int dwVal );

	/**
	* @brief Checks if a mouse key is held in this frame
	* @param val GLFW enum for the key
	*/
	virtual bool is_mouse_key_held ( int val );

	/**
	* @brief Sets screen offsets (Left)
	* @param x x offset
	* @param y y offset
	*/
	virtual void set_scr_offset ( int x, int y ) { 
		m_scr_x = x;
		m_scr_y = y;
	};

	/**
	* @brief Sets screen offsets (Right)
	* @param x x offset
	* @param y y offset
	*/
	virtual void set_scr_offset_r ( int x, int y ) {
		m_scr_x_r = x;
		m_scr_y_r = y;
	};

	/**
	* @brief Gets this frame's delta
	*/
	virtual float get_delta ( ) {
		return m_delta;
	};

	/**
	* @brief Locks frame rate, -1.0f means unlocked
	* @param fps The max frame rate
	*/
	virtual void lock_fps ( float fps ) {
		m_fps = fps;
	};

	/**
	* @brief  Gets screen size
	* @param width Dst for width
	* @param height Dst for height
	*/
	virtual void get_scr_size ( int* width, int* height ) {
		*width = m_width;
		*height = m_height;
	};

	/**
	* @brief  Gets screen offsets
	* @param y Dst for y	
	* @param x Dst for x
	*/
	virtual void get_scr_offset ( int* x, int* y ) {
		*x = m_scr_x;
		*y = m_scr_y;
	};

	virtual void set_window_title ( std::string title, bool debug_fps = false ) {
		m_title = title;
		m_debug_fps = debug_fps;
	}

	virtual GLFWwindow* get_window ( ) {
		return m_window;
	}
	
	virtual HWND get_win32_window ( ) {
		return glfwGetWin32Window ( m_window );
	}

	/**
	* @brief Draws all registered drawables this frame based on their type
	* and sets up special rendering states
	*/
	virtual void draw ( );

	/**
	* @brief Clear this frame's drawables
	*/
	virtual void clear_draw ( );

	/**
	* @brief Set viewport and clear screen
	*/
	virtual void set_viewport ( );

	/**
	* @brief Sets fps speed
	*/
	virtual void set_fps ( );
private:
	// Window
	GLFWwindow* m_window;
	std::string m_title;
	int m_width;
	int m_height;
	float m_title_timer;

	bool m_debug_fps;

	// Viewport
	int m_scr_x;
	int m_scr_y;
	int m_scr_x_r;
	int m_scr_y_r;
	unsigned char m_color [ 4 ];

	// Timing
	float m_delta;
	float m_delta_last;
	float m_fps;

	// Input
	bool m_key_down [ GLFW_KEY_LAST ];
	bool m_key_prev [ GLFW_KEY_LAST ];

	// Drawing
	cdraw_base* m_draw_tbl [ ( int ) e_draw_type::num ];
};

extern glm::mat4 g_view;
extern glm::mat4 g_proj;

#endif