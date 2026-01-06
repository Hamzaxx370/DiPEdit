#ifndef __FILE_MESH_H_
#define __FILE_MESH_H_

#include "defs.h"

#include <iostream>
#include <vector>

#include "engine\sys\render\render.h"

std::vector<cmesh_ref> read_mesh_file ( const char* szFileName, const char* VSh, const char* FSh );
void read_col_file ( const char* szFileName );

#endif