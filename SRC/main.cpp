// ============================================================================
// File: main.cpp
// 
// Purpose: Implements the application's starting point
// 
// ============================================================================

#include <iostream>
#include "engine\core\engine.h"
#include "game\core\game.h"

int main ( )
{
	cengine::create_engine ( );
	cgame::create_game ( );

	cengine::get ( )->init ( );
	cgame::get ( )->init ( );

	while ( true ) {
		cengine::get ( )->frame_begin ( );
		
		cgame::get ( )->run ( );

		int done = cengine::get ( )->frame_end ( );

		if ( done ) break;
	}

	cengine::destroy_engine ( );
	cgame::destroy_game ( );
}