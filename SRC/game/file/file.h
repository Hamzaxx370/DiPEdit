#ifndef __FILE_H_
#define __FILE_H_


#include "defs.h"

#include "glm\glm\gtc\matrix_transform.hpp"
#include "glm\glm\gtc\type_ptr.hpp"

#include "engine\sys\render\render.h"
#include "game\sys\ogre\pmm\pmm_data.h"
#include "game\sys\ogre\yact\yact_ogre.h"
#include "game\sys\ogre\yact\yact_ogre2.h"

#include <iostream>
#include <vector>

struct sparticle;
class cbinary_helper;

std::vector<cmesh_ref> read_ogre_mesh_file ( const char* szFileName, const char* VSh, const char* FSh, cbinary_helper* curr_helper = nullptr );
std::vector <cskel_bone> read_ogre_bones ( const char* filename, cbinary_helper* curr_helper = nullptr );
std::vector<ctex_ref> read_ogre_tex_file ( const char* filename, cbinary_helper* curr_helper = nullptr );
cskel_anim* read_ogre_motion ( const char* filename, cbinary_helper* curr_helper = nullptr );
cpmm_data read_ogre_pmm ( const char* filename );
void write_ogre_pmm ( const char* filename, cpmm_data data );
void get_particle_info ( std::string filename, std::string& name, int& id );
sparticle* load_particle_file ( std::string filename );
void write_particle_file ( const std::string& filename, sparticle* particle );
yact_ogre::syact_play_data read_ogre_yact_play_data ( std::string filename );
yact_ogre::syact_data read_ogre_yact_data ( std::string filename );

#endif