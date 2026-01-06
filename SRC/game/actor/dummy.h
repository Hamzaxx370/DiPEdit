#pragma once

#include "particle.h"
#include "engine\actor\entity.h"
#include "game\core\pmm_data.h"

class cact_dummy : public cact_entity {
public:
	std::vector<ceffect_authoring> m_effects;
	std::vector<e_actid> m_particles;

	bool m_looped;

	cact_dummy ( cact_base* p_parent, e_actid actid, int mot_id );
	~cact_dummy ( );

	void exec0 ( ) override;
	void exec1 ( ) override;
};