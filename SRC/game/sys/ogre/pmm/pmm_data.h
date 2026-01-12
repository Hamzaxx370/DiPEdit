#pragma once

#include "game\sys\ogre\effect_authoring\effect_authoring.h"

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