#include "engine.h"

#include "engine\sys\actor\actor.h"
#include "engine\sys\collision\collision.h"
#include "engine\sys\render\render.h"
#include "engine\sys\render\light.h"

cengine* cengine::instance = nullptr;

cengine::cengine ( ) {
	act_man = nullptr;
	render_man = nullptr;
	mesh_man = nullptr;
	tex_man = nullptr;
	light_man = nullptr;
	col_man = nullptr;
}

cengine::~cengine ( ) {
	delete act_man;
	delete render_man;
	delete mesh_man;
	delete tex_man;
	delete light_man;
	delete col_man;
}

void cengine::init ( ) {
	act_man = new cact_manager ( );
	render_man = new crender_manager ( "Game", 800, 600 );
	mesh_man = new cmesh_manager ( );
	tex_man = new ctex_manager ( );
	light_man = new clight_manager ( );
	col_man = new ccol_manager ( );
}

void cengine::frame_begin	 ( ) {
	render_man->exec0 ( );

	act_man->idle ( );
		
	render_man->draw ( );
	render_man->clear_draw ( );
}

int cengine::frame_end ( ) {
	render_man->exec1 ( );
	render_man->set_fps ( );
	return render_man->should_close ( );
}