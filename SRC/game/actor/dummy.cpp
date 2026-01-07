#include "dummy.h"
#include "engine\core\components.h"
#include "engine\core\engine.h"

#include "game\core\game.h"
#include "game\file\file.h"
#include "game\core\particle.h"
#include "particle.h"

glm::mat4 get_bone_mtx ( ceffect_authoring effect, ccomponent_anim* animator ) {
	glm::vec3 final_pos ( 0.0f );
	glm::quat final_rot ( 1.0f, 0.0f, 0.0f, 0.0f ); // Identity

	// Get the bone index for this particle
	int b = effect.m_bone_idx;
	if ( b >= 0 ) {
		const cskel_bone& bone = animator->m_bones [ b ];

		if ( effect.m_copy_pos ) {
			final_pos = bone.m_anim_pos;
		}
		if ( effect.m_copy_rot ) {
			final_rot = bone.m_anim_rot;
		}
	}

	// Build final matrix and assign to particle
	return glm::translate ( glm::mat4 ( 1.0f ), final_pos ) * glm::mat4_cast ( final_rot );
}

sparticle_data get_bone_data ( ceffect_authoring effect, ccomponent_anim* animator ) {
	sparticle_data data;
	data.m_pos = glm::vec3 ( 0.0f );
	data.m_normal = glm::vec3 ( 0.0f );
	data.m_tmp0 = glm::vec3 ( 0.0f );

	glm::vec3 e_norm = effect.m_normal;
	glm::vec3 e_tmp0 = effect.m_tmp0;
	glm::vec3 e_pos = effect.m_xyz;

	int b = effect.m_bone_idx;
	if ( b >= 0 ) {
		const cskel_bone& bone = animator->m_bones [ b ];

		if ( effect.m_copy_pos ) {
			data.m_pos = bone.m_anim_pos;
		}
		if ( effect.m_copy_rot ) {
			// Calculate vectors in Bone Space (Inverted Z)
			data.m_normal = bone.m_anim_rot * glm::vec3 ( 0.0f, 0.0f, -1.0f );
			data.m_tmp0 = bone.m_anim_rot * glm::vec3 ( 0.0f, 1.0f, 0.0f );
		}

		e_pos.z = -e_pos.z;
		e_norm.z = -e_norm.z;
		e_tmp0.z = -e_tmp0.z;

		data.m_pos += e_pos;
		data.m_normal += e_norm;
		data.m_tmp0 += e_tmp0;
	}

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
			cact_particle* p = new cact_particle ( this, cengine::get ( )->act_man->get_free_id ( e_actid::particle_start, e_actid::particle_end ), cgame::get()->m_particle_path + "\\" + get_ptcl_from_id ( m_effects [ i ].id ) + ".ptcl" );
			m_particles [ i ] = p->m_act_id;
			p->create_with_param ( m_effects [ i ], m_act_id );
			for ( auto& ptcl : p->m_particles ) {
				ptcl->set_particle_data ( get_bone_data ( m_effects [ i ], m_animator ) );
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
			particle->set_particle_data ( get_bone_data ( m_effects [ idx ], m_animator ) );
		}

		idx++;
	}
}
