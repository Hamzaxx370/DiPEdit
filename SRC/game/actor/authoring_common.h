#ifndef  __PARTICLE_H__
#define __PARTICLE_H__

#include "engine\sys\actor\actor.h"

#include "game\sys\ogre\pmm\pmm_data.h"
#include "game\sys\ogre\particle\particle.h"

#include <vector>
#include <string>

// Forward declarations
class cparticle;
struct sparticle;
class cdraw_normal;

class cact_effect : public cact_base {
public:

	bool m_pause_exec;

	cact_effect ( cact_base* p_parent, e_actid actid );
	~cact_effect ( );

	virtual void create_blank ( );
	virtual void create_with_param ( ceffect_authoring effect );

	virtual void skip_time ( float start );
};

class cact_particle : public cact_effect {
public:
	sparticle* m_particle_data;
	std::vector<cparticle*> m_particles;

	std::string m_ptcl_name;

	e_actid m_entity_id;

	cact_particle ( cact_base* p_parent, e_actid actid, std::string ptcl_name );
	~cact_particle ( );

	void exec0 ( ) override;
	void exec1 ( ) override;

	void create_blank ( ) override;
	void create_with_param ( ceffect_authoring effect ) override;

	void skip_time ( float start ) override;
};

class cact_track : public cact_effect {
public:
	cact_track ( cact_base* p_parent, e_actid actid );
	~cact_track ( );

	void exec0 ( ) override;
	void exec1 ( ) override;

	void create_blank ( ) override;
	void create_with_param ( ceffect_authoring effect ) override;

	void skip_time ( float start ) override;
};


sparticle_data get_bone_data ( ceffect_authoring effect, class ccomponent_anim* animator );
e_actid create_effect ( ceffect_authoring effect, class cact_entity* entity );
void update_effect ( ceffect_authoring effect, class cact_entity* entity, e_actid id );

#endif // ! __PARTICLE_H__
