// ============================================================================
// File: game.h
// 
// Purpose: Define the game module, without it the engine does nothing
// 
// ============================================================================

#ifndef __GAME_H_
#define __GAME_H_

#include <string>
#include "engine\sys\render\mesh.h"

// Hardcoded actor list
enum class e_actid : int {
	root,
	camera,
	dummy,
	test_particle,
	particle_manager,
	particle_start,
	particle_end = particle_start + 128, 
	num
};

// Entity types
#define ENTITY_PLAYER 0
#define ENTITY_ENEMY 1

struct sptcl_info {
	std::string name;
	int id;
};

//Data stuff
extern sptcl_info g_ptcl_info [ 697 ];

class cgame {
public:
	std::vector<sptcl_info> m_ptcl_info;
	std::vector<std::vector<cmesh_ref>> m_mesh_list;
	std::vector<std::vector<ctex_ref>> m_tex_list;
	bool m_is_y2;

	std::string m_particle_path;
	std::string m_particle_name;

	bool m_got_folder;
	bool m_particle_looped;


	std::string m_motion_path;
	std::string m_pmm_path;

	bool m_got_motion;
	bool m_motion_looped;

	cgame ( );
	~cgame ( );

	void init ( );
	void run ( );
	void populate_info ( );
	void draw_ptcl_tree ( );
	void draw_ptcl_data ( );
	void draw_pmm_data ( );

	static void create_game ( ) { 
		instance = new cgame ( );
	};
	static void destroy_game ( ) { 
		delete instance;
	};
	static cgame* get ( ) {
		return instance;
	}
private:
	static cgame* instance;
};

#endif