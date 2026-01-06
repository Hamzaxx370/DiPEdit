// ============================================================================
// File: base.cpp
// 
// Purpose: Implements the base actor
// 
// ============================================================================

#include "actor.h"

#include "engine\core\engine.h"

cact_base::cact_base ( cact_base* p_parent, e_actid id ) {
	m_act_id = id;
	m_exec_flag = 0;

	act_speed = 1.0f;

	m_parent = p_parent;
	m_child = nullptr;
	m_sibling = nullptr;

	m_delete_next = nullptr;

	// Workaround for the root actor
	if ( cengine::get ( )->act_man ) {
		cengine::get ( )->act_man->regist_actor ( this );
	}

	if ( p_parent )
		p_parent->add_child ( this );
}

cact_base::~cact_base ( ) {
}

void cact_base::exec0 ( ) {

}

void cact_base::exec1 ( ) {

}

void cact_base::add_child ( cact_base* p_act ) {
	if ( !m_child ) {
		m_child = p_act;
		return;
	}

	cact_base* p_sibling = m_child;

	while ( p_sibling->m_sibling ) {
		p_sibling = p_sibling->m_sibling;
	}

	p_sibling->m_sibling = p_act;
}

