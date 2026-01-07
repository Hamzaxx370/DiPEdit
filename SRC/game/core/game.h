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
	particle_manager,
	particle_start,
	particle_end = particle_start + 128, 
	num
};

// Entity types
#define ENTITY_PLAYER 0
#define ENTITY_ENEMY 1

class cgame {
public:
	std::vector<std::vector<cmesh_ref>> m_mesh_list;
	std::vector<std::vector<ctex_ref>> m_tex_list;
	std::string m_particle_path;
	std::string m_particle_name;
	bool m_got_folder;
	bool m_is_y2;

	bool m_looped;


	std::string m_motion_path;
	int m_motion_num;
	bool m_got_mot_folder;

	cgame ( );
	~cgame ( );

	void init ( );
	void run ( );
	void draw_ptcl_tree ( );
	void draw_ptcl_data ( );
	void draw_mot_tree ( );
	void draw_mot_data ( );

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