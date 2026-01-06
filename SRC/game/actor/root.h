// ============================================================================
// File: root.h
// 
// Purpose: Defines the game (root) actor, which just creates the actors the game
// needs, once its done the game runs
// 
// ============================================================================

#ifndef __ROOT_H_
#define __ROOT_H_

#include "engine\sys\actor\actor.h"

class cact_game : public cact_base {
public:
	cact_game ( cact_base* pParent, e_actid actid );
	~cact_game ( );

	void exec0 ( ) override;
	void exec1 ( ) override;
};

#endif