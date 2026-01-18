#include "yact_manager.h"

#include "game\core\game.h"
#include "game\file\file.h"

cact_yact_manager::cact_yact_manager ( cact_base* p_parent, e_actid actid ) : cact_base ( p_parent, actid ) {
	reset ( );
}

cact_yact_manager::~cact_yact_manager ( ) {

}

void cact_yact_manager::exec0 ( ) {

}

void cact_yact_manager::exec1 ( ) {

}

void cact_yact_manager::reset ( ) {
	for ( auto& info : m_yact_play_data.m_common ) {
		if ( info.m_player )
			delete info.m_player;
	}
	for ( auto& info : m_yact_play_data.m_range ) {
		if ( info.m_player )
			delete info.m_player;
	}

	m_yact_play_data.m_common.clear ( );
	m_yact_play_data.m_range.clear ( );

	m_yact_play_data = read_ogre_yact_play_data ( cgame::get ( )->m_yact_path + "\\" + "YActPlayData.bin" );
}

void cact_yact_manager::start_yact ( ) {

}

void cact_yact_manager::release_yact ( ) {

}