// ============================================================================
// File: Camera.cpp
// 
// Purpose: Implements the camera actor
// 
// ============================================================================

#include "Camera.h"

#include "engine\core\engine.h"
#include "engine\sys\render\render.h"

cact_camera::cact_camera ( cact_base* p_parent, e_actid actid ) : cact_base ( p_parent, actid ) {
	m_cam_view = e_cam_view::orbit;
	m_pos = glm::vec3 ( 0.0f, 30.0f, -50.0f );
	m_offset = glm::vec3 ( 0.0f, 30.0f, -50.0f );
	m_target = glm::vec3 ( 0.0, 0.0, 0.0 );
	m_lookat = glm::vec3 ( 0.0f );

	m_angle = 0.0;
	m_fov = 45.0f;

	m_up = glm::vec3 ( 0.0f, 1.0f, 0.0f );
	m_front = glm::vec3 ( 0.0f, 0.0f, -1.0f );
}

cact_camera::~cact_camera ( ) {

}

void cact_camera::exec0 ( ) {
}

void cact_camera::exec1 ( ) {
	int width, height;
	float delta = cengine::get ( )->render_man->get_delta ( );
	cengine::get ( )->render_man->get_scr_size ( &width, &height );
	switch ( m_cam_view ) {
	case e_cam_view::orbit: {
		handle_mouse_input ( ); // Update yaw/pitch from mouse

		// Compute rotated offset
		glm::vec3 offset;
		offset.x = m_offset.z * cos ( m_pitch ) * sin ( m_yaw );
		offset.y = m_offset.z * sin ( m_pitch );
		offset.z = m_offset.z * cos ( m_pitch ) * cos ( m_yaw );

		m_pos = m_lookat + offset;
		m_target = m_lookat;

		g_view = glm::lookAt ( m_pos, m_target, m_up );

		// Zoom with keyboard or mouse wheel (optional)
		if ( cengine::get ( )->render_man->is_key_held ( GLFW_KEY_UP ) )
			m_offset.z += 50.0f * delta;
		if ( cengine::get ( )->render_man->is_key_held ( GLFW_KEY_DOWN ) )
			m_offset.z -= 50.0f * delta;

		break;
	}
	case e_cam_view::fp: {
		g_view = glm::lookAt ( m_pos , m_target, m_up );
		break;
	}
	}

	// Finalize matrices
	if ( ( float ) width == 0 || ( float ) height == 0 )
		g_proj = glm::perspective ( glm::radians ( m_fov ), ( float ) ( 600.0 / 800.0 ), 0.1f, 10000.0f );
	else
		g_proj = glm::perspective ( glm::radians ( m_fov ), ( float ) width / ( float ) height, 0.1f, 10000.0f );
}

// Called every frame
void cact_camera::handle_mouse_input ( ) {
	auto render = cengine::get ( )->render_man;

	if ( render->is_mouse_key_held ( GLFW_MOUSE_BUTTON_RIGHT ) ) {
		if ( !m_orbiting ) {
			// First frame of right click
			m_orbiting = true;
			render->get_cursor_pos ( &m_last_mouse_x, &m_last_mouse_y );
		}
		else {
			// Compute mouse delta
			double mouse_x, mouse_y;
			render->get_cursor_pos ( &mouse_x, &mouse_y );
			double dx = mouse_x - m_last_mouse_x;
			double dy = mouse_y - m_last_mouse_y;

			m_last_mouse_x = mouse_x;
			m_last_mouse_y = mouse_y;

			m_yaw -= float ( dx ) * m_sensitivity;
			m_pitch -= float ( dy ) * m_sensitivity;

			// Clamp pitch to avoid flipping
			if ( m_pitch > glm::radians ( 89.0f ) ) m_pitch = glm::radians ( 89.0f );
			if ( m_pitch < glm::radians ( -89.0f ) ) m_pitch = glm::radians ( -89.0f );
		}
	}
	else {
		m_orbiting = false;
	}
}

void cact_camera::set_target ( glm::vec3 v ) {
	m_target = v;
	m_lookat = v;
}