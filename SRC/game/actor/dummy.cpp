#include "dummy.h"
#include "engine\core\components.h"
#include "engine\core\engine.h"

#include "game\core\game.h"
#include "game\file\file.h"
#include "game\sys\ogre\particle\particle.h"
#include "authoring_common.h"

cact_dummy::cact_dummy ( cact_base* p_parent, e_actid actid ) : cact_entity ( p_parent, actid ) {
	m_animator = new ccomponent_anim ( );

	std::string mesh_name;
	if ( cgame::get ( )->m_is_y2 ) {
		mesh_name = "kiryu_y2";
	}
	else
	{
		mesh_name = "kiryu_y1";
	}

	m_draw = new cdraw_normal ( );
	m_draw->m_meshes = read_ogre_mesh_file ( ( "default\\" + mesh_name + ".ome" ).c_str ( ), "Shaders\\vertex_general.glsl", "Shaders\\fragment_general.glsl" );

	for ( auto& ref : m_draw->m_meshes ) {
		cmesh_buffer* mesh = ref.get ( );
		mesh->m_used_tex = read_ogre_tex_file ( ( "default\\" + mesh_name + ".txb" ).c_str ( ) );
	}

	m_animator->m_bones = read_ogre_bones ( ( "default\\" + mesh_name + ".ome" ).c_str ( ) );

	m_pause_motion = false;
}

cact_dummy::~cact_dummy ( ) {
	clear_particles ( );
}

void cact_dummy::exec0 ( ) {
	cact_entity::exec0 ( );
}

void cact_dummy::exec1 ( ) {
	if ( !m_animator->m_anim ) return;

	m_animator->m_paused = m_pause_motion;

	cact_entity::exec1 ( );

	update_particle_logic ( );
}

void cact_dummy::clear_particles ( ) {
	for ( auto& id : m_particles ) {
		if ( id == ( e_actid ) -1 )
			continue;

		cact_effect* p = ( cact_effect* ) cengine::get ( )->act_man->get_actor ( id );

		if ( !p )
			continue;

		p->set_exec_flag ( e_act_exec::done );
		p->set_exec_flag ( e_act_exec::pause );
	}

	m_particles.clear ( );
	m_particles.resize ( m_pmm_data.m_effects.size ( ), ( e_actid ) -1 );
}

void cact_dummy::reload_motion ( std::string mot_name, std::string pmm_name, bool reload_all, float start ) {

	if ( reload_all ) {
		m_animator->m_anim = read_ogre_motion ( mot_name.c_str ( ) );
		m_pmm_data = read_ogre_pmm ( pmm_name.c_str ( ) );
		m_animator->m_frame_counter = 0.0f;
	}

	clear_particles ( );

	set_time ( start );
}


void cact_dummy::update_particle_logic ( ) {
	float now = m_animator->m_frame_counter;

	for ( int i = 0; i < m_pmm_data.m_effects.size ( ); i++ ) {
		auto& effect = m_pmm_data.m_effects [ i ];
		if ( effect.m_type != 1 || effect.m_id == 0 )
			continue;

		if ( i >= m_particles.size ( ) ) continue;

		e_actid id = m_particles [ i ];

		if ( id == ( e_actid ) -1 ) {
			if ( now >= effect.m_start && now <= effect.m_end ) {
				m_particles [i ] = create_effect ( effect, this );
			}
		}
		else {
			cact_effect* e = ( cact_effect* ) cengine::get ( )->act_man->get_actor ( id );
			if ( e ) {
				e->m_pause_exec = m_pause_motion;

				update_effect ( effect, this, id );
			}
		}
	}
}

void cact_dummy::set_time ( float target_time ) {
	clear_particles ( );
	m_animator->m_frame_counter = 0.0f;

	const float step_size = 1.0f / 10.0f;
	float current_sim_time = 0.0f;

	while ( current_sim_time < target_time ) {

		float dt = step_size;
		if ( current_sim_time + dt > target_time ) {
			dt = target_time - current_sim_time;
		}
		current_sim_time += dt;

		m_animator->m_frame_counter = current_sim_time;
		m_animator->exec ( );

		update_particle_logic ( );

		for ( auto& id : m_particles ) {
			if ( id == ( e_actid ) -1 ) continue;

			cact_effect* p = ( cact_effect* ) cengine::get ( )->act_man->get_actor ( id );
			if ( p ) {
				p->skip_time ( dt );
			}
		}
	}
}