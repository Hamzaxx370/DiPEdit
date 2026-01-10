#pragma once

#include "glm\glm\gtc\matrix_transform.hpp"
#include "glm\glm\gtc\type_ptr.hpp"

#include <vector>

class ceffect_authoring {
public:
	int m_play_type;
	float m_start;
	float m_end;
	float m_speed;
	int m_bone_idx;
	int m_type;
	int m_id;
	glm::vec3 m_xyz;
	glm::vec3 m_normal;
	glm::vec3 m_tmp0;
	glm::vec3 m_tmp1;
	int m_tmp0_int;
	int m_tmp1_int;
};

class cpmm_property {
public:
	int m_type;
	float m_start;
	float m_end;
	int m_tmp0;
	int m_tmp1;
	int m_tmp2;
	int m_tmp3;
	int m_tmp4;
};

class cpmm_data {
public:
	std::vector<cpmm_property> m_properties;
	std::vector<ceffect_authoring> m_effects;
};