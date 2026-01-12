#include "authoring_common.h"

#include "game\core\game.h"
#include "game\file\file.h"
#include "engine\core\engine.h"
#include "engine\sys\render\render.h"

#include "engine\actor\entity.h"
#include "engine\core\components.h"

#include <iostream>

cact_effect::cact_effect ( cact_base* p_parent, e_actid actid ) : cact_base ( p_parent, actid ) {
	m_pause_exec = false;
};

cact_effect::~cact_effect ( ) {

}

void cact_effect::create_blank ( ) {

}

void cact_effect::create_with_param ( ceffect_authoring effect ) {

}

void cact_effect::skip_time ( float start ) {

}


cact_particle::cact_particle ( cact_base* p_parent, e_actid actid, std::string ptcl_name ) : cact_effect ( p_parent, actid ) {
	m_ptcl_name = ptcl_name;
	m_particle_data = load_particle_file ( m_ptcl_name.c_str ( ) );
	m_entity_id = ( e_actid ) -1;
}

cact_particle::~cact_particle ( ) {
	if ( m_particle_data ) {
		delete m_particle_data;
		m_particle_data = nullptr;
	}

	for ( auto& p : m_particles ) {
		p->release ( );
		delete p;
	}
	m_particles.clear ( );
}

void cact_particle::exec0 ( ) {
	if ( m_particles.empty ( ) ) return;

	bool finished = true;

	for ( auto& particle : m_particles ) {
		if ( !particle->m_finished )
			finished = false;
	}

	if ( finished ) {
		for ( auto& p : m_particles ) {
			p->release ( );
			delete p;
		}
		m_particles.clear ( );
	};
}

void cact_particle::exec1 ( ) {
	if ( m_particles.empty ( ) ) return;

	for ( auto& particle : m_particles ) {
		if ( particle->m_finished )
			continue;
		if ( !m_pause_exec )
			particle->execute ( cengine::get ( )->render_man->get_delta ( ) / PTCL_FRAME_SPD );
		particle->draw ( );
	}
}

void cact_particle::create_blank ( ) {
	if ( !m_particles.empty ( ) ) {
		for ( auto& p : m_particles ) {
			p->release ( );
			delete p;
			p = nullptr;
		}
		m_particles.clear ( );
	}
	sparticle_data data = { glm::vec3 ( 0.0f ), glm::vec3 ( 0.0f, 0.0f, -1.0f ), glm::vec3 ( 0.0f,1.0f,0.0f ) };

	for ( auto& param : m_particle_data->m_particle_params ) {
		cparticle* particle = new cparticle ( );
		if ( !particle->initialize ( param, data, glm::vec3 ( 0.0f ) ) ) {
			particle->release ( );
			delete particle;
			continue;
		}
		particle->set_attach_mtx ( glm::mat4 ( 1.0f ) );

		m_particles.push_back ( particle );
	}
}
void cact_particle::create_with_param ( ceffect_authoring effect ) {
	if ( !m_particles.empty ( ) ) {
		for ( auto& p : m_particles ) {
			p->release ( );
			delete p;
			p = nullptr;
		}
		m_particles.clear ( );
	};

	glm::vec3 f0 = effect.m_normal;

	glm::vec3 f1 = effect.m_tmp0;
	//f1 = glm::vec3 ( f1.z, f1.y, f1.x );

	sparticle_data data = { effect.m_xyz, f0, f1 };

	for ( auto& param : m_particle_data->m_particle_params ) {
		cparticle* particle = new cparticle ( );
		if ( !particle->initialize ( param, data, glm::vec3 ( 0.0f ) ) ) {
			particle->release ( );
			delete particle;
			continue;
		}
		particle->set_attach_mtx ( glm::mat4 ( 1.0f ) );
		if ( effect.m_tmp1.z == 1.0f ) {
			particle->set_emitter_flag ( e_emitter_flag::follow );
		}
		particle->set_speed ( effect.m_speed );

		m_particles.push_back ( particle );
	}
}

void cact_particle::skip_time ( float start ) {
	for ( auto& particle : m_particles ) {
		particle->execute ( start );
	}
}

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

cact_track::cact_track ( cact_base* p_parent, e_actid actid ) : cact_effect ( p_parent, actid ) {
}

cact_track::~cact_track ( ) {
}

void cact_track::exec0 ( ) {

}

void cact_track::exec1 ( ) {

}

void cact_track::create_blank ( ) {

}

void cact_track::create_with_param ( ceffect_authoring effect ) {

}

void cact_track::skip_time ( float start ) {

}

e_actid create_effect ( ceffect_authoring effect, cact_entity* entity ) {
	switch ( effect.m_type ) {
	default:
		return ( e_actid ) -1;
	case 1:
		if ( effect.m_id > 0 ) {
			std::string ptcl_name = get_ptcl_from_id ( effect.m_id );

			cact_particle* p = new cact_particle (
				cengine::get ( )->act_man->get_actor ( e_actid::particle_manager ),
				cengine::get ( )->act_man->get_free_id ( e_actid::particle_start, e_actid::particle_end ),
				cgame::get ( )->m_particle_path + "\\" + ptcl_name + ".ptcl"
			);

			p->create_with_param ( effect );

			for ( auto& ptcl : p->m_particles ) {
				ptcl->set_particle_data ( get_bone_data ( effect, entity->m_animator ) );
			}

			return p->m_act_id;
		}

		break;
	}
}

void update_effect ( ceffect_authoring effect, cact_entity* entity, e_actid id ) {

	cact_effect* eff_act = ( cact_effect* ) cengine::get ( )->act_man->get_actor ( id );

	switch ( effect.m_type ) {
	default:
		return;
	case 1:
		if ( effect.m_id > 0 ) {
			cact_particle* p = ( cact_particle* ) eff_act;
			if ( ( effect.m_tmp1_int & 0x00000001 ) != 0 ) {
				for ( auto& particle : p->m_particles ) {
					particle->set_particle_data (
						get_bone_data ( effect, entity->m_animator )
					);
				}
			}
		}

		break;
	}
}