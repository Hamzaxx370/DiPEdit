#pragma once

#include "glm\glm\gtc\matrix_transform.hpp"
#include "glm\glm\gtc\type_ptr.hpp"

class ceffect_authoring {
public:
	float m_start;
	float m_end;
	float m_speed;
	int m_bone_idx;
	int id;
	glm::vec3 m_xyz;
	glm::vec3 m_normal;
	glm::vec3 m_tmp0;
	glm::vec3 m_tmp1;
	bool m_copy_pos;
	bool m_copy_rot;
};