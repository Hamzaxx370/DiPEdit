// ============================================================================
// File: Camera.h
// 
// Purpose: Defines the camera actor
// 
// ============================================================================

#ifndef __CAMERA_H_
#define __CAMERA_H_

#include "engine\sys\actor\actor.h"

#include "glm\glm\gtc\matrix_transform.hpp"
#include "glm\glm\gtc\type_ptr.hpp"

enum class e_cam_view {
	orbit,
	fp,
	free
};

class cact_camera : public cact_base {
public:
	e_cam_view m_cam_view;
	glm::vec3 m_pos;
	glm::vec3 m_offset;
	glm::vec3 m_target;
	glm::vec3 m_lookat;
	float m_angle;
	float m_fov;
	float m_yaw = 0.0f;
	float m_pitch = 0.0f;
	float m_sensitivity = 0.009f;
	double m_last_mouse_x = 0.0;
	double m_last_mouse_y = 0.0;
	bool m_orbiting = false;

	glm::vec3 m_up;
	glm::vec3 m_front;

	cact_camera ( cact_base* p_parent, e_actid actid );
	~cact_camera ( );

	void exec0 ( ) override;
	void exec1 ( ) override;

	void handle_mouse_input ( );
	void set_target ( glm::vec3 v );
};

#endif