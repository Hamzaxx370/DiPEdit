#include "dummy.h"
#include "engine\core\components.h"
#include "engine\core\engine.h"

#include "game\core\game.h"
#include "game\file\file.h"
#include "game\core\particle.h"
#include "particle.h"

sparticle_data get_bone_data ( ceffect_authoring effect, ccomponent_anim* animator ) {
	sparticle_data data;

	data.m_pos = effect.m_xyz;
	data.m_normal = effect.m_normal;
	data.m_tmp0 = effect.m_tmp0;

	bool motion = ( effect.m_tmp1_int & 0x00000100 ) != 0;

	unsigned int orient_flags;
	float tmp1_z = effect.m_tmp1.z;
	if ( tmp1_z >= 2147483600.0f ) {
		orient_flags = ( unsigned int ) ( ( int ) ( tmp1_z - 2147483600.0f ) + 0x80000000 );
	}
	else {
		orient_flags = ( unsigned int ) tmp1_z;
	}
	bool orient = ( orient_flags & 0x1 ) != 0;

	int bone_idx = effect.m_bone_idx;
	if ( bone_idx >= 0 ) {
		const cskel_bone& bone = animator->m_bones [ bone_idx ];
		glm::mat4 bone_mtx = ( bone.m_final * bone.m_rest );

		data.m_pos = glm::vec3 ( bone_mtx * glm::vec4 ( data.m_pos, 1.0f ) );

		if ( motion ) {
			glm::mat3 normal_mtx = glm::transpose ( glm::inverse ( glm::mat3 ( bone_mtx ) ) );
			data.m_normal = normal_mtx * data.m_normal;
		}

		glm::mat3 rot_mtx = glm::mat3 ( bone_mtx );
		data.m_tmp0 = rot_mtx * data.m_tmp0;
	}

	return data;
}

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
	m_draw->m_meshes = read_ogre_mesh_file ( ( "default\\" + mesh_name + ".ome" ).c_str ( ), 0, "Shaders\\vertex_general.glsl", "Shaders\\fragment_general.glsl" );

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

		cact_particle* p = ( cact_particle* ) cengine::get ( )->act_man->get_actor ( id );

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

				std::string ptcl_name = get_ptcl_from_id ( effect.m_id );

				cact_particle* p = new cact_particle (
					cengine::get ( )->act_man->get_actor ( e_actid::particle_manager ),
					cengine::get ( )->act_man->get_free_id ( e_actid::particle_start, e_actid::particle_end ),
					cgame::get ( )->m_particle_path + "\\" + ptcl_name + ".ptcl"
				);

				m_particles [ i ] = p->m_act_id;

				p->create_with_param ( effect, m_act_id );

				for ( auto& ptcl : p->m_particles ) {
					ptcl->set_particle_data ( get_bone_data ( effect, m_animator ) );
				}
			}
		}
		else {
			cact_particle* p = ( cact_particle* ) cengine::get ( )->act_man->get_actor ( id );
			if ( p ) {
				p->m_pause_particle_exec = m_pause_motion;

				if ( now > effect.m_end || now < effect.m_start ) {
				}

				if ( ( effect.m_tmp1_int & 0x00000001 ) != 0 ) {
					for ( auto& particle : p->m_particles ) {
						particle->set_particle_data (
							get_bone_data ( effect, m_animator )
						);
					}
				}
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

			cact_particle* p = ( cact_particle* ) cengine::get ( )->act_man->get_actor ( id );
			if ( p ) {
				p->skip_time ( dt );
			}
		}
	}
}