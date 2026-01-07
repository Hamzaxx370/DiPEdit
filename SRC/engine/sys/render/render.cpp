// ============================================================================
// File: render.cpp
// 
// Purpose: Implements the render manager
// 
// ============================================================================

#include "render.h"

#include "engine\core\engine.h"

static unsigned char s_cColor [ 4 ] = { 32,32,32,255 };
glm::mat4 g_view = glm::mat4 ( 1.0 );
glm::mat4 g_proj = glm::mat4 ( 1.0 );

#include <string>
#include <fstream>
#include <iostream>

crender_manager::crender_manager ( const char* title, int width, int height ) {
	// Simple OpenGL initialization
	m_title = title;
	m_width = width;
	m_height = height;
	m_scr_x = 0;
	m_scr_y = 0;
	m_scr_x_r = 0;
	m_scr_y_r = 0;
	m_title_timer = 0.0f;

	memcpy ( m_color, s_cColor, sizeof ( m_color ) );

	m_delta = 0.0f;
	m_delta_last = 0.0f;
	m_fps = -1.0f;

	int i;
	for ( i = 0; i < GLFW_KEY_LAST; i++ ) {
		m_key_down [ i ] = false;
		m_key_prev [ i ] = false;
	}

	clear_draw ( );

	glfwInit ( );
	glfwWindowHint ( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint ( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint ( GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE );

	// Create window
	m_window = glfwCreateWindow ( width, height, m_title.c_str ( ), NULL, NULL );
	glfwMakeContextCurrent ( m_window );
	//glfwSwapInterval ( 1 );
	gladLoadGL ( );

	glDisable ( GL_CULL_FACE );

	glfwSetInputMode ( m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE );
}

void crender_manager::exec0 ( ) {
	// Get inputs
	glfwPollEvents ( );

	m_title_timer += m_delta;

	// Update input
	int i;
	for ( i = 0; i < GLFW_KEY_LAST; i++ ) {
		m_key_prev [ i ] = m_key_down [ i ];
	}

	for ( i = 0; i < GLFW_KEY_LAST; i++ ) {
		m_key_down [ i ] = ( glfwGetKey ( m_window, i ) == GLFW_PRESS );
	}
	// Clear screen
	set_viewport ( );

	if ( !m_debug_fps ) {
		glfwSetWindowTitle ( m_window, m_title.c_str ( ) );
		return;
	}
	// Update title
	if ( m_title_timer >= 1.0f ) {
		char cTitle [ 32 ];
		sprintf ( cTitle, "%s    FPS: %f", m_title.c_str ( ), 1.0f / m_delta );
		glfwSetWindowTitle ( m_window, cTitle );
		m_title_timer = 0.0f;
	}
}

void crender_manager::exec1 ( ) {
	glfwGetFramebufferSize ( m_window, &m_width, &m_height );
	glfwSwapBuffers ( m_window );
	m_width -= m_scr_x + m_scr_x_r;
	m_height -= m_scr_y + m_scr_y_r;
}

void crender_manager::regist_draw ( cdraw_base* p_draw, e_draw_type type ) {
	p_draw->m_next = nullptr;
	if ( !m_draw_tbl [ ( int ) type ] ) {
		m_draw_tbl [ ( int ) type ] = p_draw;
		return;
	}
	cdraw_base* p_curr = m_draw_tbl [ ( int ) type ];
	while ( p_curr->m_next ) {
		p_curr = p_curr->m_next;
	}
	p_curr->m_next = p_draw;
}

void crender_manager::draw ( ) {
	for ( int i = 0; i < ( int ) e_draw_type::num; i++ ) {
		// Set up special rendering types
		switch ( ( e_draw_type ) i ) {
		case e_draw_type::normal: {
			glEnable ( GL_DEPTH_TEST );
			glEnable ( GL_BLEND );
			glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			break;
		}
		case e_draw_type::particle: {
			glEnable ( GL_BLEND );
			glBlendFunc ( GL_SRC_ALPHA, GL_ONE );
			glBlendEquation ( GL_FUNC_ADD );
			glDepthMask ( GL_FALSE );
			break;
		}
		case e_draw_type::ui: {
			break;
		}
		}

		cdraw_base* p_curr = m_draw_tbl [ i ];
		while ( p_curr ) {
			p_curr->draw ( );
			p_curr = p_curr->m_next;
		}

		// End special rendering types
		switch ( ( e_draw_type ) i ) {
		case e_draw_type::normal: {
			;
			break;
		}
		case e_draw_type::particle: {
			glDepthMask ( GL_TRUE );
			break;
		}
		case e_draw_type::ui: {
			break;
		}
		}
	}
}

void crender_manager::clear_draw ( ) {
	for ( int i = 0; i < ( int ) e_draw_type::num; i++ ) {
		m_draw_tbl [ i ] = nullptr;
	}
}

void crender_manager::set_viewport ( ) {
	glViewport ( m_scr_x, m_scr_y, m_width, m_height );
	glClearColor ( ( float ) m_color [ 0 ] / 255.0f, ( float ) m_color [ 1 ] / 255.0f, ( float ) m_color [ 2 ] / 255.0f, ( float ) m_color [ 3 ] / 255.0f );
	glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

bool crender_manager::should_close ( ) {
	return glfwWindowShouldClose ( m_window );
}

bool crender_manager::is_window_focus ( ) {
	return glfwGetWindowAttrib ( m_window, GLFW_FOCUSED );
}

void crender_manager::set_fps ( ) {
	if ( m_fps == -1 ) {
		float d = static_cast< float >( glfwGetTime ( ) );
		m_delta = d - m_delta_last;
		m_delta_last = d;
		return;
	}

	float target = 1.0f / m_fps;

	float current = static_cast< float >( glfwGetTime ( ) );
	float frame = current - m_delta_last;

	if ( frame < target ) {
		float sleep = target - frame;
		std::this_thread::sleep_for ( std::chrono::duration<float> ( sleep ) );
	}

	m_delta_last = static_cast< float > ( glfwGetTime ( ) );
	m_delta = target;
}

void crender_manager::set_cursor_lock ( int val ) {
	glfwSetInputMode ( m_window, GLFW_CURSOR, val );
}

void crender_manager::get_cursor_pos ( double* dX, double* dY ) {
	glfwGetCursorPos ( m_window, dX, dY );
}

void crender_manager::set_cursor_pos ( double dX, double dY ) {
	glfwSetCursorPos ( m_window, dX, dY );
}

bool crender_manager::is_key_held ( int dwVal ) {
	return m_key_down [ dwVal ];
}

bool crender_manager::is_key_just_pressed ( int dwVal ) {
	return ( m_key_down [ dwVal ] && !m_key_prev [ dwVal ] );
}
bool crender_manager::is_key_just_released ( int dwVal ) {
	return ( !m_key_down [ dwVal ] && m_key_prev [ dwVal ] );
}

bool crender_manager::is_mouse_key_held ( int val ) {
	return glfwGetMouseButton ( m_window, val ) == GLFW_PRESS;
}