// ============================================================================
// File: game.h
// 
// Purpose: Define the game module, without it the engine does nothing
// 
// ============================================================================

#ifndef __GAME_H_
#define __GAME_H_

#include <string>

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
	std::string m_particle_path;
	std::string m_particle_name;
	bool m_got_folder;
	bool m_is_y2;

	bool m_looped;

	cgame ( );
	~cgame ( );

	void init ( );
	void run ( );
	void draw_ptcl_tree ( );
	void draw_ptcl_data ( );

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