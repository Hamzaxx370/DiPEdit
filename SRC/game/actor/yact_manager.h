#pragma once

#include "engine\sys\actor\actor.h"

#include "game\sys\ogre\yact\yact_ogre.h"

#include <string>

class cact_yact_manager : public cact_base {
public:
	yact_ogre::syact_play_data m_yact_play_data;

	cact_yact_manager ( cact_base* p_parent, e_actid actid );
	~cact_yact_manager ( );

	void exec0 ( ) override;
	void exec1 ( ) override;

	void reset ( );
	void start_yact ( );
	void release_yact ( );
};