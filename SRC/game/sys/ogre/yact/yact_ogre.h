#pragma once


#include "glm\glm\gtc\matrix_transform.hpp"
#include "glm\glm\gtc\type_ptr.hpp"
#include "glm\glm\gtx\norm.hpp"

#include <vector>

#include "game\sys\ogre\effect_authoring\effect_authoring.h"
#include "engine\sys\render\mesh.h"

namespace yact_ogre {

	struct syact_motion {
		int m_unk;
		float m_start_frame;
		float m_end_frame;
		float m_speed;
		int m_index;
		int m_unk1;
		int m_unk2;
		int m_unk3;
	};

	struct syact_camera {
		int m_unk;
		int m_effect_inx;
		int m_effect_num;
		std::vector<syact_motion> m_motions;
	};

	struct syact_player {
		int m_cond_flag;
		int m_human_status;
		int m_arm_flag;
		int m_arm_category;
		int m_boot_flag;
		int m_unk;
		int m_end_status;

		int m_angle;
		int m_arc;

		int max_health;
		int min_health;

		int m_effect_inx;
		int m_effect_num;
		std::vector<syact_motion> m_motions;
	};

	struct syact_enemy {
		int m_category;
		int m_category_mask;
		int m_cond_flag;
		int m_human_status;
		int m_arm_flag;
		int m_arm_category;
		int m_boot_flag; // 0x1 Use Matrix / Vertex distance check
							  // 0x20 Skip some checks

		int m_unk0;
		int m_end_status;
		int m_unk1;
		int m_unk2;
		int max_health;
		int m_unk_char0;
		int min_health;
		int m_unk_char1;

		int m_effect_inx;
		int m_effect_num;
		std::vector<syact_motion> m_motions;
	};

	// Mostly unk
	struct syact_object {
		int m_unk1;
		int m_unk2;
		int m_unk3;
		int m_stgobj_index;
		int m_condition;
		int m_matrix_unk; // idk
		int m_unk4;
		int m_unk5;
		int m_unk6;
		int m_unk7;
		int m_unk8;
		int m_unk9;

		int m_effect_inx;
		int m_effect_num;
		std::vector<syact_motion> m_motions;
	};

	struct syact_model {
		int m_mdl_id;
		int m_tex_id;
		int m_parent_id;
		int m_unk;

		glm::vec3 m_position;
		glm::vec3 m_rotation;
		glm::vec3 m_scale;

		int m_effect_inx;
		int m_effect_num;
		std::vector<syact_motion> m_motions;
	};

	struct syact_arm {
		unsigned char m_unk1 [ 16 ];
		unsigned char m_unk2 [ 32 ];

		int m_effect_inx;
		int m_effect_num;
		std::vector<syact_motion> m_motions;
	};

	struct syact_info {
		unsigned char m_unk_data [ 64 ];

		int m_file_id;

		std::vector<syact_camera> m_cameras;
		
		syact_player* m_player;

		std::vector<syact_enemy> m_enemies;
		std::vector<syact_object> m_objects;
		std::vector<syact_model> m_models;
		std::vector<syact_arm> m_arms;
	};

	struct syact_play_data {
		std::vector<syact_info> m_common;
		std::vector<syact_info> m_range;
	};

	struct smtb_pos_key {
		glm::vec3 m_pos;
		float m_time;
	};

	struct smtb_quat_key {
		glm::quat m_rot;
		float m_time;
	};

	struct smtb_fov_key {
		float m_fov;
		float m_time;
	};

	struct smtb_motion {
		float m_frame_num;
		std::vector<smtb_pos_key> m_pos_keys;
		std::vector<smtb_quat_key> m_rot_keys;
		std::vector<smtb_fov_key> m_fov_keys;
	};

	struct syact_data_chunk {
		unsigned char m_data [ 32 ];
	};

	struct syact_data {
		std::vector<syact_data_chunk> m_chunks1;
		std::vector<syact_data_chunk> m_chunks2;
		std::vector<ceffect_authoring> m_effects;
		std::vector<cmesh_ref> m_meshes;
		std::vector<ctex_ref> m_textures;
		std::vector<smtb_motion> m_motions;
		std::vector<cskel_anim> m_anims;
	};

}