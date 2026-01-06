// ============================================================================
// File: map.h
// 
// Purpose: Defines the map actor, which manages the current map loaded by the 
// game
// 
// ============================================================================

#ifndef __MAP_H_
#define __MAP_H_

#include "engine\sys\actor\actor.h"

// Forward declarations
class cdraw_normal;

class cact_map : public cact_base {
public:
	cdraw_normal* m_draw;
	cact_map ( cact_base* p_parent, e_actid actid, const char* map_name );
	~cact_map ( );

	void exec0 ( ) override;
	void exec1 ( ) override;
};

#endif