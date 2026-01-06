#ifndef __FILE_H_
#define __FILE_H_


#include "defs.h"

#include "glm\glm\gtc\matrix_transform.hpp"
#include "glm\glm\gtc\type_ptr.hpp"

#include "engine\sys\render\render.h"
#include "game\core\pmm_data.h"

#include <iostream>
#include <vector>

class sparticle;

std::vector<cmesh_ref> read_ogre_mesh_file ( const char* szFileName, int type, const char* VSh, const char* FSh );
std::vector <cskel_bone> read_ogre_bones ( const char* filename );
std::vector<ctex_ref> read_ogre_tex_file ( const char* filename );
cskel_anim* read_ogre_motion ( const char* filename );
std::vector<ceffect_authoring> read_ogre_pmm ( const char* filename );
sparticle* load_particle_file ( std::string filename );
void write_particle_file ( const std::string& filename, sparticle* particle );
#endif