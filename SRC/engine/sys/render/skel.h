// ============================================================================
// File: Render.h
// 
// Purpose: Defines the skeleton animator and structs
// 
// ============================================================================

#ifndef __SKEL_H_
#define __SKEL_H_

#include "defs.h"

#include <unordered_map>
#include <string>
#include <thread>
#include <chrono>

#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include "glm\glm\gtc\matrix_transform.hpp"
#include "glm\glm\gtc\type_ptr.hpp"

class cskel_pos_key {
public:
	float m_time;
	glm::vec3 m_pos;
	cskel_pos_key ( );
};


class cskel_rot_key {
public:
	float m_time;
	glm::quat m_rot;
	cskel_rot_key ( );
};

class cskel_bone_anim {
public:
	std::vector<cskel_pos_key> m_pos_keys;
	std::vector<cskel_rot_key> m_rot_keys;
};

class cskel_anim {
public:
	float m_frame_num;
	float m_frame_rate;
	std::vector<cskel_bone_anim> m_bone_anims;

	cskel_anim ( );
};

class cskel_bone {
public:
	glm::vec3 m_rest_pos;
	glm::vec3 m_anim_pos;
	glm::quat m_rest_rot;
	glm::quat m_anim_rot;
	glm::mat4 m_rest;
	glm::mat4 m_final;

	int m_index;
	int m_parent;
	int m_child;
	int m_sibling;

	cskel_bone ( );
	~cskel_bone ( );
};

class cskel_animator {
public:
	/**
	 * @brief Updates a mesh's bone matrices based on an animation and time.
	 * @param p_mesh Pointer to the mesh containing the skeleton.
	 * @param p_anim Pointer to the animation data.
	 * @param time The current time in seconds.
	 */
	static void skel_update_animation ( std::vector<cskel_bone>& bones, const cskel_anim* p_anim, float time );

	/**
	 * @brief Recursively updates global matrices based on local animation data.
	 * @param p_mesh The mesh buffer.
	 * @param bone_idx The index of the bone to update.
	 * @param parent_transform The calculated global transform of the parent.
	 */
	static void skel_update_hierarchy ( std::vector<cskel_bone>& bones, const cskel_anim* p_anim, float time, int bone_idx, const glm::vec3& local_pos, const glm::quat& local_rot );

};

#endif