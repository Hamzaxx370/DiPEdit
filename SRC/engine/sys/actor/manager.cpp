// ============================================================================
// File: manager.cpp
// 
// Purpose: Implements the actor manager
// 
// ============================================================================

#include "actor.h"

#include "engine\core\engine.h"

cact_manager::cact_manager ( ) {
	m_root_act = nullptr;
	m_delete_list = nullptr;

	m_act_tbl = nullptr;
	m_act_tbl_num = 0;
}

cact_manager::~cact_manager ( ) {
	int i;

	for ( i = 0; i < m_act_tbl_num; i++ ) {
		if ( m_act_tbl [ i ] )
			delete m_act_tbl [ i ];
	}

	delete [ ] m_act_tbl;
}

// Game loop
int cact_manager::idle ( ) {
	// Do two execs, for actors to be synced with each other 
	act_exec0 ( m_root_act );
	act_exec1 ( m_root_act );

	// Clean up finished actors
	while ( m_delete_list ) {
		cact_base* p_next = m_delete_list->m_delete_next;
		delete_actor ( m_delete_list );
		m_delete_list = p_next;
	}

	return 0;
}

// Just creates the game actor
void cact_manager::init ( e_actid act_num ) {
	m_act_tbl_num = ( int ) act_num;
	m_act_tbl = new cact_base* [ m_act_tbl_num ];
	for ( int i = 0; i < m_act_tbl_num; i++ ) {
		m_act_tbl [ i ] = nullptr;
	}
}

void cact_manager::regist_actor ( cact_base* p_act ) {
	if ( !m_act_tbl ) return;
	m_act_tbl [ ( int ) p_act->m_act_id ] = p_act;
}

cact_base* cact_manager::get_actor ( e_actid actid ) {
	return m_act_tbl [ ( int ) actid ];
}

e_actid cact_manager::get_free_id ( e_actid st, e_actid en ) {
	int i;
	for ( i = ( int ) st; i < ( int ) en; i++ ) {
		if ( !m_act_tbl [ i ] )
			return ( e_actid ) i;
	}
	return ( e_actid ) -1;
}

void cact_manager::delete_actor ( cact_base* p_act ) {
	if ( !p_act )
		return;

	// Relink actors
	if ( p_act->m_parent ) {
		if ( p_act->m_parent->m_child == p_act )
			p_act->m_parent->m_child = p_act->m_sibling;
		else {
			cact_base* p_prev = p_act->m_parent->m_child;
			cact_base* p_curr = p_act->m_parent->m_child->m_sibling;

			while ( p_curr ) {
				if ( p_curr == p_act ) {
					p_prev->m_sibling = p_curr->m_sibling;
					break;
				}
				p_prev = p_curr;
				p_curr = p_curr->m_sibling;
			}
		}
	}

	// Recursive child deletion
	cact_base* p_child = p_act->m_child;
	while ( p_child ) {
		cact_base* p_next = p_child->m_sibling;
		delete_actor ( p_child );
		p_child = p_next;
	}

	// Remove ActTbl reference
	m_act_tbl [ ( int ) p_act->m_act_id ] = nullptr;
	delete p_act;
}

// Do first exec for actors
void cact_manager::act_exec0 ( cact_base* p_act ) {
	if ( !p_act )
		return;

	if ( !( p_act->m_exec_flag & ( int ) e_act_exec::pause ) )
		p_act->exec0 ( );

	cact_base* p_child = p_act->m_child;

	while ( p_child ) {
		act_exec0 ( p_child );
		p_child = p_child->m_sibling;
	}
}

// Do second exec and add finished actors for deletion
void cact_manager::act_exec1 ( cact_base* p_act ) {
	if ( !p_act )
		return;

	if ( !( p_act->m_exec_flag & ( int ) e_act_exec::pause ) )
		p_act->exec1 ( );

	if ( ( p_act->m_exec_flag & ( int ) e_act_exec::done ) ) {
		add_delete ( p_act );
		return;
	}

	cact_base* p_child = p_act->m_child;

	while ( p_child ) {
		act_exec1 ( p_child );
		p_child = p_child->m_sibling;
	}
}

// Add an actor to the deletion list
void cact_manager::add_delete ( cact_base* pAct ) {
	if ( !m_delete_list ) {
		m_delete_list = pAct;
		return;
	}

	cact_base* p_next = m_delete_list;
	while ( p_next->m_delete_next ) {
		p_next = p_next->m_delete_next;
	}

	p_next->m_delete_next = pAct;
}