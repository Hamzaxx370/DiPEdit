#ifndef  __PARTICLE_H__
#define __PARTICLE_H__

#include "engine\sys\actor\actor.h"
#include "game\core\pmm_data.h"

#include <vector>
#include <string>

// Forward declarations
class cparticle;
struct sparticle;
class cdraw_normal;

class cact_particle : public cact_base {
public:
	sparticle* m_particle_data;
	std::vector<cparticle*> m_particles;

	std::string m_ptcl_name;

	e_actid m_entity_id;

	cact_particle ( cact_base* p_parent, e_actid actid, std::string ptcl_name );
	~cact_particle ( );

	void exec0 ( ) override;
	void exec1 ( ) override;

	void create_blank ( );
	void create_with_param ( ceffect_authoring effect, e_actid entity_id );
};

#endif // ! __PARTICLE_H__
