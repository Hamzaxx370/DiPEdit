#include "particle.h"

#include "game\file\file.h"
#include "game\core\particle.h"
#include "engine\core\engine.h"
#include "engine\sys\render\render.h"

#include <iostream>

cact_particle::cact_particle ( cact_base* p_parent, e_actid actid, std::string ptcl_name ) : cact_base ( p_parent, actid ) {
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
		particle->execute ( );
		particle->draw ( );
	}
}

void cact_particle::create_blank ( ) {
	if ( !m_particles.empty() ) {
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
void cact_particle::create_with_param ( ceffect_authoring effect, e_actid entity_id ) {
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
		m_particles.push_back ( particle );
	}
}