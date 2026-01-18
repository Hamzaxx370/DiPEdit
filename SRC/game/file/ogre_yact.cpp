#include "file.h"

#include "engine\file\binary_helper.h"

std::vector<yact_ogre::syact_motion> read_motions ( cbinary_helper& bin_help, int mot_ptr, int mot_count ) {
	int returnpos = bin_help.get_pos ( );

	std::vector<yact_ogre::syact_motion> mot_list;
	bin_help.seek ( mot_ptr );
	for ( int i = 0; i < mot_count; i++ ) {
		yact_ogre::syact_motion motion;
		motion.m_unk = bin_help.read_int ( );
		motion.m_start_frame = bin_help.read_float ( );
		motion.m_end_frame = bin_help.read_float ( );
		motion.m_speed = bin_help.read_float ( );
		motion.m_index = bin_help.read_int ( );
		motion.m_unk1 = bin_help.read_int ( );
		motion.m_unk2 = bin_help.read_int ( );
		motion.m_unk3 = bin_help.read_int ( );
		mot_list.push_back ( motion );
	}

	bin_help.seek ( returnpos );
	return mot_list;
}

std::vector<yact_ogre::syact_camera> read_cameras ( cbinary_helper& bin_help, int cam_ptr, int cam_count ) {
	std::vector<yact_ogre::syact_camera> cam_list;
	bin_help.seek ( cam_ptr );
	for ( int i = 0; i < cam_count; i++ ) {
		yact_ogre::syact_camera camera;
		camera.m_unk = bin_help.read_int ( );
		camera.m_effect_inx = bin_help.read_int ( );
		camera.m_effect_num = bin_help.read_int ( );
		int mot_ptr = bin_help.read_int ( );
		int mot_count = bin_help.read_int ( );

		bin_help.read_int ( );
		bin_help.read_int ( );
		bin_help.read_int ( );

		camera.m_motions = read_motions ( bin_help, mot_ptr, mot_count );

		cam_list.push_back ( camera );
	}
	return cam_list;
}

yact_ogre::syact_player* read_player ( cbinary_helper& bin_help, int plyr_ptr ) {

	if ( plyr_ptr == 0 )
		return nullptr;

	yact_ogre::syact_player* player = new yact_ogre::syact_player ( );

	bin_help.seek ( plyr_ptr );
	player->m_cond_flag = bin_help.read_int ( );
	player->m_human_status = bin_help.read_int ( );
	player->m_arm_flag = bin_help.read_int ( );
	player->m_arm_category = bin_help.read_int ( );
	player->m_boot_flag = bin_help.read_int ( );
	player->m_unk = bin_help.read_int ( );
	player->m_end_status = bin_help.read_int ( );
	bin_help.read_int ( );
	bin_help.read_int ( );
	player->m_effect_inx = bin_help.read_int ( );
	player->m_effect_num = bin_help.read_int ( );
	player->m_angle = bin_help.read_int ( );
	player->m_arc = bin_help.read_int ( );
	int mot_ptr = bin_help.read_int ( );
	int mot_count = bin_help.read_int ( );
	player->max_health = bin_help.read_uchar ( );
	player->min_health = bin_help.read_uchar ( );

	bin_help.read_uchar ( );
	bin_help.read_uchar ( );
	
	player->m_motions = read_motions ( bin_help, mot_ptr, mot_count );
	return player;
}

std::vector<yact_ogre::syact_enemy> read_enemies ( cbinary_helper& bin_help, int ene_ptr, int ene_count ) {
	std::vector<yact_ogre::syact_enemy> ene_list;
	bin_help.seek ( ene_ptr );
	for ( int i = 0; i < ene_count; i++ ) {
		yact_ogre::syact_enemy enemy;
		enemy.m_category = bin_help.read_int ( );
		enemy.m_category_mask = bin_help.read_int ( );
		enemy.m_cond_flag = bin_help.read_int ( );
		enemy.m_human_status = bin_help.read_int ( );
		enemy.m_arm_flag = bin_help.read_int ( );
		enemy.m_arm_category = bin_help.read_int ( );
		enemy.m_boot_flag = bin_help.read_int ( );
		enemy.m_unk0 = bin_help.read_int ( );
		enemy.m_end_status = bin_help.read_int ( );
		enemy.m_unk1 = bin_help.read_int ( );
		enemy.m_unk2 = bin_help.read_int ( );
		enemy.m_effect_inx = bin_help.read_int ( );
		enemy.m_effect_num = bin_help.read_int ( );
		int mot_ptr = bin_help.read_int ( );
		int mot_count = bin_help.read_int ( );
		enemy.max_health = bin_help.read_uchar ( );
		enemy.m_unk_char0 = bin_help.read_uchar ( );
		enemy.min_health = bin_help.read_uchar ( );
		enemy.m_unk_char1 = bin_help.read_uchar ( );

		enemy.m_motions = read_motions ( bin_help, mot_ptr, mot_count );
		ene_list.push_back ( enemy );
	}
	return ene_list;
}

std::vector<yact_ogre::syact_object> read_objects ( cbinary_helper& bin_help, int obj_ptr, int obj_count ) {
	std::vector<yact_ogre::syact_object> obj_list;
	bin_help.seek ( obj_ptr );
	for ( int i = 0; i < obj_count; i++ ) {
		yact_ogre::syact_object object;
		object.m_unk1 = bin_help.read_int ( );
		object.m_unk2 = bin_help.read_int ( );
		object.m_unk3 = bin_help.read_int ( );
		object.m_stgobj_index = bin_help.read_int ( );
		object.m_condition = bin_help.read_int ( );
		object.m_matrix_unk = bin_help.read_int ( );

		object.m_effect_inx = bin_help.read_int ( );
		object.m_effect_num = bin_help.read_int ( );

		object.m_unk4 = bin_help.read_int ( );

		int mot_ptr = bin_help.read_int ( );
		int mot_count = bin_help.read_int ( );

		object.m_unk5 = bin_help.read_int ( );
		object.m_unk6 = bin_help.read_int ( );
		object.m_unk7 = bin_help.read_int ( );
		object.m_unk8 = bin_help.read_int ( );
		object.m_unk9 = bin_help.read_int ( );
		
		object.m_motions = read_motions ( bin_help, mot_ptr, mot_count );
		obj_list.push_back ( object );
	}
	return obj_list;
}

std::vector<yact_ogre::syact_model> read_models ( cbinary_helper& bin_help, int mdl_ptr, int mdl_count ) {
	std::vector<yact_ogre::syact_model> mdl_list;
	bin_help.seek ( mdl_ptr );
	for ( int i = 0; i < mdl_count; i++ ) {
		yact_ogre::syact_model model;
		model.m_mdl_id = bin_help.read_int ( );
		model.m_tex_id = bin_help.read_int ( );
		model.m_parent_id = bin_help.read_int ( );
		
		model.m_effect_inx = bin_help.read_int ( );
		model.m_effect_num = bin_help.read_int ( );
		int mot_ptr = bin_help.read_int ( );
		int mot_count = bin_help.read_int ( );

		model.m_unk = bin_help.read_int ( );
		model.m_position.x = bin_help.read_float ( );
		model.m_position.y = bin_help.read_float ( );
		model.m_position.z = bin_help.read_float ( );
		bin_help.read_int ( );

		model.m_rotation.x = bin_help.read_int ( );
		model.m_rotation.y = bin_help.read_int ( );
		model.m_rotation.z = bin_help.read_int ( );
		bin_help.read_int ( );

		model.m_scale.x = bin_help.read_float ( );
		model.m_scale.y = bin_help.read_float ( );
		model.m_scale.z = bin_help.read_float ( );
		bin_help.read_int ( );

		model.m_motions = read_motions ( bin_help, mot_ptr, mot_count );
		mdl_list.push_back ( model );
	}
	return mdl_list;
}

std::vector<yact_ogre::syact_arm> read_arms ( cbinary_helper& bin_help, int arm_ptr, int arm_count ) {
	std::vector<yact_ogre::syact_arm> arm_list;
	bin_help.seek ( arm_ptr );
	for ( int i = 0; i < arm_count; i++ ) {
		yact_ogre::syact_arm arm;
		bin_help.read_uchars_fixed ( arm.m_unk1, 16 );
		
		arm.m_effect_inx = bin_help.read_int ( );
		arm.m_effect_num = bin_help.read_int ( );
		int mot_ptr = bin_help.read_int ( );
		int mot_count = bin_help.read_int ( );

		bin_help.read_uchars_fixed ( arm.m_unk2, 32 );
		
		arm.m_motions = read_motions ( bin_help, mot_ptr, mot_count );
		arm_list.push_back ( arm );
	}
	return arm_list;
}

std::vector<yact_ogre::syact_info> read_category ( cbinary_helper& bin_help, int info_ptr, int info_count ) {
	bin_help.set_begin ( 0 );
	std::vector<yact_ogre::syact_info> info_list;
	bin_help.seek ( info_ptr );
	for ( int i = 0; i < info_count; i++ ) {
		yact_ogre::syact_info info;
		bin_help.set_begin ( bin_help.get_pos ( ) + bin_help.m_begin );

		bin_help.read_uchars_fixed ( info.m_unk_data, 64 );
		info.m_file_id = bin_help.read_int ( );
		int size = bin_help.read_int ( );
		int cam_ptr = bin_help.read_int ( );
		int cam_cnt = bin_help.read_int ( );
		int plyr_ptr = bin_help.read_int ( );
		int ene_ptr = bin_help.read_int ( );
		int ene_cnt = bin_help.read_int ( );
		int obj_ptr = bin_help.read_int ( );
		int obj_cnt = bin_help.read_int ( );
		int mdl_ptr = bin_help.read_int ( );
		int mdl_cnt = bin_help.read_int ( );
		int arm_ptr = bin_help.read_int ( );
		int arm_cnt = bin_help.read_int ( );

		info.m_cameras = read_cameras ( bin_help, cam_ptr, cam_cnt );
		info.m_player = read_player ( bin_help, plyr_ptr );
		info.m_enemies = read_enemies ( bin_help, ene_ptr, ene_cnt );
		info.m_objects = read_objects ( bin_help, obj_ptr, obj_cnt );
		info.m_models = read_models ( bin_help, mdl_ptr, mdl_cnt );
		info.m_arms = read_arms ( bin_help, arm_ptr, arm_cnt );

		bin_help.seek ( size );

		info_list.push_back ( info );
	}
	return info_list;
}

yact_ogre::syact_play_data read_ogre_yact_play_data ( std::string filename ) {
	cbinary_helper bin_help = cbinary_helper ( filename.c_str ( ) );

	yact_ogre::syact_play_data play_data;

	bin_help.seek ( 16 );
	int cmn_ptr = bin_help.read_int ( );

	bin_help.seek ( 32 );
	int rng_ptr = bin_help.read_int ( );

	bin_help.seek ( cmn_ptr );

	int cmn_yact_ptr = bin_help.read_int ( );
	int cmn_yact_count = bin_help.read_int ( );

	bin_help.seek ( rng_ptr );

	int rng_yact_ptr = bin_help.read_int ( );
	int rng_yact_count = bin_help.read_int ( );

	play_data.m_common = read_category ( bin_help, cmn_ptr + cmn_yact_ptr, cmn_yact_count );
	play_data.m_range = read_category ( bin_help, rng_ptr + rng_yact_ptr, rng_yact_count );

	return play_data;
}

yact_ogre::syact_data read_ogre_yact_data ( std::string filename ) {
	yact_ogre::syact_data data;

	cbinary_helper bin_help = cbinary_helper ( filename.c_str ( ) );
	
	int size = bin_help.read_int ( );
	int effect_tbl = bin_help.read_int ( );
	int effect_cnt = bin_help.read_int ( );
	int chnk1_tbl = bin_help.read_int ( );
	int chnk1_cnt = bin_help.read_int ( );
	int chnk2_tbl = bin_help.read_int ( );
	int chnk2_cnt = bin_help.read_int ( );
	int mot_tbl = bin_help.read_int ( );
	int mot_cnt = bin_help.read_int ( );
	int mdl_tbl = bin_help.read_int ( );
	int mdl_cnt = bin_help.read_int ( );
	int exmot_tbl = bin_help.read_int ( );
	int exmot_cnt = bin_help.read_int ( );
	int tex_tbl = bin_help.read_int ( );
	int tex_cnt = bin_help.read_int ( );

	return data;
}