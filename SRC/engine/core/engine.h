// ============================================================================
// File: engine.h
// 
// ============================================================================

#ifndef __ENGINE_H_
#define __ENGINE_H_

#include "defs.h"

// Forward declarations
class cact_manager;
class crender_manager;
class cmesh_manager;
class ctex_manager;
class clight_manager;
class ccol_manager;

class cengine {
public:
	cact_manager* act_man;
	crender_manager* render_man;
	cmesh_manager* mesh_man;
	ctex_manager* tex_man;
	clight_manager* light_man;
	ccol_manager* col_man;


	cengine ( );
	~cengine ( );
	
	static void create_engine ( ) {
		instance = new cengine ( );
	}
	static void destroy_engine ( ) {
		delete instance;

	}
	void init ( );
	void frame_begin ( );
	int frame_end ( );

	static cengine* get ( ) { 
		return instance;
	};

private:
	static cengine* instance;
};

#endif