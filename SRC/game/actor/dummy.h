#pragma once

#include "engine\actor\entity.h"
#include "game\sys\ogre\pmm\pmm_data.h"

#include <string>

class cact_dummy : public cact_entity {
public:
	std::string m_mot_name;
	std::string m_pmm_name;
	cpmm_data m_pmm_data;

	std::vector<e_actid> m_particles;

	bool m_pause_motion;

	cact_dummy ( cact_base* p_parent, e_actid actid );
	~cact_dummy ( );

	void exec0 ( ) override;
	void exec1 ( ) override;

	void clear_particles ( );
	void reload_motion ( std::string mot_name, std::string pmm_name, bool reload_all = false, float start = 0.0f );

	void update_particle_logic ( );
	void set_time ( float target_time );
};