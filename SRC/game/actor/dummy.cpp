#include "dummy.h"
#include "engine\core\components.h"
#include "engine\core\engine.h"

#include "game\core\game.h"
#include "game\file\file.h"
#include "game\core\particle.h"
#include "particle.h"

sparticle_data get_bone_data ( ceffect_authoring effect, ccomponent_anim* animator, bool force ) {
	sparticle_data data;

	data.m_pos = effect.m_xyz;
	data.m_normal = effect.m_normal;
	data.m_tmp0 = effect.m_tmp0;

	bool parent = ( effect.m_tmp1_int & 0x00000001 ) != 0;
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
	if ( bone_idx >= 0 && ( parent || force ) ) {
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

	data.m_normal = -data.m_normal;
	data.m_tmp0 = -data.m_tmp0;

	return data;
}

cact_dummy::cact_dummy ( cact_base* p_parent, e_actid actid, int mot_id ) : cact_entity ( p_parent, actid ) {
	m_animator = new ccomponent_anim ( );
	m_draw = new cdraw_normal ( );
	m_draw->m_meshes = read_ogre_mesh_file ( "Default\\KiryuBody.OME", 0, "Shaders\\vertex_general.glsl", "Shaders\\fragment_general.glsl" );
	for ( auto& ref : m_draw->m_meshes ) {
		cmesh_buffer* mesh = ref.get ( );
		mesh->m_used_tex = read_ogre_tex_file ( "Default\\KiryuBody.TXB" );
	}

	m_animator->m_bones = read_ogre_bones ( "Default\\KiryuBody.OME" );
	m_animator->m_anim = read_ogre_motion ( ( cgame::get ( )->m_motion_path + "\\0-" + std::to_string ( mot_id ) + ".dat" ).c_str ( ) );
	m_effects = read_ogre_pmm ( ( cgame::get ( )->m_motion_path + "\\1-" + std::to_string ( mot_id ) + ".dat" ).c_str ( ) );;

	m_looped = false;

	m_particles.resize ( m_effects.size ( ), ( e_actid ) -1 );
}

cact_dummy::~cact_dummy ( ) {
	for ( auto& id : m_particles ) {
		if ( id == ( e_actid ) -1 )
			continue;

		cact_particle* p = ( cact_particle* ) cengine::get ( )->act_man->get_actor ( id );

		if ( !p )
			continue;

		p->set_exec_flag ( e_act_exec::done );
		p->set_exec_flag ( e_act_exec::pause );
	}
}

void cact_dummy::exec0 ( ) {
	cact_entity::exec0 ( );
}

void cact_dummy::exec1 ( ) {
    if ( m_animator->m_frame_counter >= m_animator->m_anim->m_frame_num && !m_looped ) {
        set_exec_flag ( e_act_exec::done );
        set_exec_flag ( e_act_exec::pause );
        return;
    }

    cact_entity::exec1 ( );

	for ( int i = 0; i < m_effects.size ( ); i++ ) {
		if ( m_animator->m_frame_counter >= m_effects [ i ].m_start && 
			m_animator->m_frame_counter <= m_effects [ i ].m_end && 
			m_particles [ i ] == ( e_actid ) -1 ) {
			cact_particle* p = new cact_particle ( cengine::get ( )->act_man->get_actor ( e_actid::particle_manager ),
				cengine::get ( )->act_man->get_free_id ( e_actid::particle_start, e_actid::particle_end ),
				cgame::get ( )->m_particle_path + "\\" + get_ptcl_from_id ( m_effects [ i ].id ) + ".ptcl"
			);
			m_particles [ i ] = p->m_act_id;
			p->create_with_param ( m_effects [ i ], m_act_id );
			for ( auto& ptcl : p->m_particles ) {
				ptcl->set_particle_data ( get_bone_data ( m_effects [ i ], m_animator, true ) );
			}
		}
	}

	int idx = 0;
	for ( auto& id : m_particles ) {
		if ( id == ( e_actid ) -1 ) {
			idx++;
			continue;
		}
		cact_particle* p = ( cact_particle* ) cengine::get ( )->act_man->get_actor ( id );

		if ( !p ) {
			idx++;
			continue;
		}

		for ( auto& particle : p->m_particles ) {
			particle->set_particle_data ( 
				get_bone_data ( 
					m_effects [ idx ], 
					m_animator, 
					glm::epsilonEqual (m_animator->m_frame_counter,m_effects[idx].m_start,2.0f ) 
				) 
			);
		}

		idx++;
	}
}
