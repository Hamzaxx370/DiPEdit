#include "file.h"

#include "engine\file\binary_helper.h"

enum eKEYFRAME_TYPE
{
	eKEYFRAME_TYPE_NONE = 0,
	eKEYFRAME_TYPE_PATTERN = 1,
	eKEYFRAME_TYPE_ROTATION = 4,
	eKEYFRAME_TYPE_POSITION_ROTATION = 5,
	eKEYFRAME_TYPE_ROTATION_XY = 16
};

struct somt_hdr
{
	int m_anim_pointer;
	int m_keyframe_pointer;
	int m_anim_count;
	int m_flag_pointer;
	int m_frame_count;
	int m_pointers;
	int m_bone_count;
	int m_flag;
};

struct somt_bone { 
	eKEYFRAME_TYPE m_mot_type;
	int m_pos_ptr;
	int m_quat_ptr;
	int m_pos_time_ptr;
	int m_quat_time_ptr;
	int m_pos_num;
	int m_quat_num;
};


void get_pos ( cbinary_helper& bin_help, cskel_pos_key& keyframe, eKEYFRAME_TYPE type )
{
	keyframe.m_pos.x = bin_help.read_float ( );
	keyframe.m_pos.y = bin_help.read_float ( );
	keyframe.m_pos.z = bin_help.read_float ( );
}
void get_rot ( cbinary_helper& bin_help, cskel_rot_key& keyframe, eKEYFRAME_TYPE type )
{
	unsigned short data_w = bin_help.read_ushort ( );
	unsigned short data_x = bin_help.read_ushort ( );
	unsigned short data_y = 16384;
	unsigned short data_z = 16384;
	if ( type == eKEYFRAME_TYPE_ROTATION )
	{
		data_y = bin_help.read_ushort ( );
		data_z = bin_help.read_ushort ( );
	}

	double quat_data_w = ( ( double ) data_w / 16384.0 ) - 1.0;
	double quat_data_x = ( ( double ) data_x / 16384.0 ) - 1.0;
	double quat_data_y = ( ( double ) data_y / 16384.0 ) - 1.0;
	double quat_data_z = ( ( double ) data_z / 16384.0 ) - 1.0;

	double sums = ( pow ( quat_data_w, 2 ) + pow ( quat_data_x, 2 ) + pow ( quat_data_y, 2 ) + pow ( quat_data_z, 2 ) );

	double Magnitude = sqrt ( sums );

	double quat_normalized_w = ( quat_data_w / Magnitude );
	double quat_normalized_x = ( quat_data_x / Magnitude );
	double quat_normalized_y = ( quat_data_y / Magnitude );
	double quat_normalized_z = ( quat_data_z / Magnitude );

	glm::quat inverse = glm::quat ( -quat_normalized_w, quat_normalized_x, quat_normalized_y, quat_normalized_z );
	keyframe.m_rot = glm::normalize ( inverse );
}

cskel_anim* read_ogre_motion ( const char* filename ) {
	cbinary_helper bin_help = cbinary_helper ( filename );

	cskel_anim* motion = new cskel_anim ( );

	std::vector<somt_bone> bone_info;

	bin_help.seek ( 0 );
	bool is_short_frames = false;

	somt_hdr header;

	header.m_anim_pointer = bin_help.read_uint ( );
	header.m_keyframe_pointer = bin_help.read_uint ( );
	header.m_anim_count = bin_help.read_uint ( );
	header.m_flag_pointer = bin_help.read_uint ( );
	header.m_frame_count = bin_help.read_uint ( );
	header.m_pointers = bin_help.read_uint ( );
	header.m_bone_count = bin_help.read_uint ( );
	header.m_flag = bin_help.read_uint ( );

	motion->m_frame_num = header.m_frame_count;
	motion->m_frame_rate = 30.0f;

	bone_info.resize ( header.m_bone_count );
	motion->m_bone_anims.resize ( header.m_bone_count );

	if ( header.m_frame_count > 0xFF )
	{
		is_short_frames = true;
	}

	bin_help.seek ( header.m_flag_pointer );
	for ( int f = 0; f < header.m_bone_count; f++ )
	{
		bone_info [ f ].m_mot_type = ( eKEYFRAME_TYPE ) bin_help.read_uchar ( );
	}
	bin_help.seek ( header.m_pointers );
	for ( int f = 0; f < header.m_bone_count; f++ )
	{
		if ( bone_info [ f ].m_mot_type == eKEYFRAME_TYPE_POSITION_ROTATION )
		{
			bone_info [ f ].m_pos_ptr = bin_help.read_uint ( );
			bone_info [ f ].m_quat_ptr = bin_help.read_uint ( );
		}
		else if ( bone_info [ f ].m_mot_type == eKEYFRAME_TYPE_ROTATION_XY || bone_info [ f ].m_mot_type == eKEYFRAME_TYPE_ROTATION )
		{
			bone_info [ f ].m_pos_ptr = -1;
			bone_info [ f ].m_quat_ptr = bin_help.read_uint ( );
		}
		else if ( bone_info [ f ].m_mot_type == eKEYFRAME_TYPE_PATTERN )
		{
			bin_help.read_uint ( );
			bone_info [ f ].m_pos_ptr = -1;
			bone_info [ f ].m_quat_ptr = -1;
		}
		else if ( bone_info [ f ].m_mot_type == eKEYFRAME_TYPE_NONE )
		{
			bone_info [ f ].m_pos_ptr = -1;
			bone_info [ f ].m_quat_ptr = -1;
		}
	}
	for ( int f = 0; f < header.m_bone_count; f++ )
	{
		if ( bone_info [ f ].m_mot_type == eKEYFRAME_TYPE_POSITION_ROTATION )
		{
			bone_info [ f ].m_pos_time_ptr = bin_help.read_uint ( );
			bone_info [ f ].m_quat_time_ptr = bin_help.read_uint ( );
		}
		else if ( bone_info [ f ].m_mot_type == eKEYFRAME_TYPE_ROTATION_XY || bone_info [ f ].m_mot_type == eKEYFRAME_TYPE_ROTATION )
		{
			bone_info [ f ].m_pos_time_ptr = -1;
			bone_info [ f ].m_quat_time_ptr = bin_help.read_uint ( );
		}
		else if ( bone_info [ f ].m_mot_type == eKEYFRAME_TYPE_PATTERN )
		{
			bin_help.read_uint ( );
			bone_info [ f ].m_pos_time_ptr = -1;
			bone_info [ f ].m_quat_time_ptr = -1;
		}
		else if ( bone_info [ f ].m_mot_type == eKEYFRAME_TYPE_NONE )
		{
			bone_info [ f ].m_pos_time_ptr = -1;
			bone_info [ f ].m_quat_time_ptr = -1;
		}
	}
	for ( int f = 0; f < header.m_bone_count; f++ )
	{
		if ( bone_info [ f ].m_mot_type == eKEYFRAME_TYPE_POSITION_ROTATION )
		{
			if ( is_short_frames )
			{
				bone_info [ f ].m_pos_num = bin_help.read_ushort ( );
				bone_info [ f ].m_quat_num = bin_help.read_ushort ( );
			}
			else
			{
				bone_info [ f ].m_pos_num = bin_help.read_uchar ( );
				bone_info [ f ].m_quat_num = bin_help.read_uchar ( );
			}
		}
		else if ( bone_info [ f ].m_mot_type == eKEYFRAME_TYPE_ROTATION_XY || bone_info [ f ].m_mot_type == eKEYFRAME_TYPE_ROTATION )
		{
			if ( is_short_frames )
			{
				bone_info [ f ].m_quat_num = bin_help.read_ushort ( );
			}
			else
			{
				bone_info [ f ].m_quat_num = bin_help.read_uchar ( );
			}
		}
		else if ( bone_info [ f ].m_mot_type == eKEYFRAME_TYPE_NONE )
		{
		}
		if ( bone_info [ f ].m_pos_num != -1 ) {
			motion->m_bone_anims [ f ].m_pos_keys.resize ( bone_info [ f ].m_pos_num );
		}
		if ( bone_info [ f ].m_quat_num != -1 ) {
			motion->m_bone_anims [ f ].m_rot_keys.resize ( bone_info [ f ].m_quat_num );
		}
	}
	for ( int f = 0; f < header.m_bone_count; f++ )
	{
		if ( bone_info [ f ].m_mot_type == eKEYFRAME_TYPE_POSITION_ROTATION )
		{
			bin_help.seek ( bone_info [ f ].m_pos_time_ptr );
			for ( int t = 0; t < bone_info [ f ].m_pos_num; t++ )
			{
				if ( is_short_frames )
				{
					motion->m_bone_anims [ f ].m_pos_keys [ t ].m_time = ( float ) bin_help.read_ushort ( );
				}
				else
				{
					motion->m_bone_anims [ f ].m_pos_keys [ t ].m_time = bin_help.read_uchar ( );
				}
			}
			bin_help.seek ( bone_info [ f ].m_quat_time_ptr );
			for ( int t = 0; t < bone_info [ f ].m_quat_num; t++ )
			{
				if ( is_short_frames )
				{
					motion->m_bone_anims [ f ].m_rot_keys [ t ].m_time = bin_help.read_ushort ( );
				}
				else
				{
					motion->m_bone_anims [ f ].m_rot_keys [ t ].m_time = bin_help.read_uchar ( );
				}
			}

		}
		else if ( bone_info [ f ].m_mot_type == eKEYFRAME_TYPE_ROTATION_XY || bone_info [ f ].m_mot_type == eKEYFRAME_TYPE_ROTATION )
		{
			bin_help.seek ( bone_info [ f ].m_quat_time_ptr );
			for ( int t = 0; t < bone_info [ f ].m_quat_num; t++ )
			{
				if ( is_short_frames )
				{
					motion->m_bone_anims [ f ].m_rot_keys [ t ].m_time = bin_help.read_ushort ( );
				}
				else
				{
					motion->m_bone_anims [ f ].m_rot_keys [ t ].m_time = bin_help.read_uchar ( );
				}
			}
		}
		else if ( bone_info [ f ].m_mot_type == eKEYFRAME_TYPE_NONE )
		{
		}
	}
	for ( int f = 0; f < header.m_bone_count; f++ )
	{

		if ( bone_info [ f ].m_mot_type == eKEYFRAME_TYPE_POSITION_ROTATION )
		{
			bin_help.seek ( bone_info [ f ].m_pos_ptr );

			for ( int t = 0; t < bone_info [ f ].m_pos_num; t++ )
			{
				get_pos ( bin_help, motion->m_bone_anims [ f ].m_pos_keys [ t ], eKEYFRAME_TYPE_POSITION_ROTATION );
			}
			bin_help.seek ( bone_info [ f ].m_quat_ptr );
			for ( int t = 0; t < bone_info [ f ].m_quat_num; t++ )
			{
				get_rot ( bin_help, motion->m_bone_anims [ f ].m_rot_keys [ t ], eKEYFRAME_TYPE_ROTATION );
			}
		}
		else if ( bone_info [ f ].m_mot_type == eKEYFRAME_TYPE_ROTATION )
		{
			bin_help.seek ( bone_info [ f ].m_quat_ptr );
			for ( int t = 0; t < bone_info [ f ].m_quat_num; t++ )
			{
				get_rot ( bin_help, motion->m_bone_anims [ f ].m_rot_keys [ t ], eKEYFRAME_TYPE_ROTATION );
			}
		}
		else if ( bone_info [ f ].m_mot_type == eKEYFRAME_TYPE_ROTATION_XY )
		{
			bin_help.seek ( bone_info [ f ].m_quat_ptr );
			for ( int t = 0; t < bone_info [ f ].m_quat_num; t++ )
			{
				get_rot ( bin_help, motion->m_bone_anims [ f ].m_rot_keys [ t ], eKEYFRAME_TYPE_ROTATION_XY );
			}
		}
		else if ( bone_info [ f ].m_mot_type == eKEYFRAME_TYPE_NONE )
		{
		}
	}
	return motion;
}

cpmm_data read_ogre_pmm ( const char* filename ) {
	cbinary_helper bin_help = cbinary_helper ( filename );
	cpmm_data data;



	int ptr = bin_help.read_int ( );
	int count = bin_help.read_int ( );
	int ptr1 = bin_help.read_int ( );
	int count1 = bin_help.read_int ( );

	bin_help.seek ( ptr );
	for ( int i = 0; i < count; i++ ) {
		cpmm_property prop;
		prop.m_type = bin_help.read_int ( );
		prop.m_start = bin_help.read_float ( );
		prop.m_end = bin_help.read_float ( );
		prop.m_tmp0 = bin_help.read_int ( );
		prop.m_tmp1 = bin_help.read_int ( );
		prop.m_tmp2 = bin_help.read_int ( );
		prop.m_tmp3 = bin_help.read_int ( );
		prop.m_tmp4 = bin_help.read_int ( );

		data.m_properties.push_back ( prop );
	}

	bin_help.seek ( ptr1 );
	for ( int i = 0; i < count1; i++ ) {
		ceffect_authoring effect;
		int start = bin_help.get_pos ( );
		effect.m_play_type = bin_help.read_int ( );
		effect.m_start = bin_help.read_float ( );
		effect.m_end = bin_help.read_float ( );
		effect.m_speed = bin_help.read_float ( );
		effect.m_bone_idx = bin_help.read_int ( );
		bin_help.seek ( start + 32 );
		effect.m_xyz.x = bin_help.read_float ( );
		effect.m_xyz.y = bin_help.read_float ( );
		effect.m_xyz.z = bin_help.read_float ( );
		effect.m_type = bin_help.read_int ( );
		effect.m_normal.x = bin_help.read_float ( );
		effect.m_normal.y = bin_help.read_float ( );
		effect.m_normal.z = bin_help.read_float ( );
		effect.m_id = bin_help.read_int ( );
		effect.m_tmp0.x = bin_help.read_float ( );
		effect.m_tmp0.y = bin_help.read_float ( );
		effect.m_tmp0.z = bin_help.read_float ( );
		effect.m_tmp0_int = bin_help.read_uint ( );
		effect.m_tmp1.x = bin_help.read_float ( );
		effect.m_tmp1.y = bin_help.read_float ( );
		effect.m_tmp1.z = bin_help.read_float ( );
		bin_help.seek ( start + 92 );
		effect.m_tmp1_int = bin_help.read_int ( );
		bin_help.seek ( start + 96 );
		data.m_effects.push_back ( effect );
	}
	return data;
}

void write_ogre_pmm ( const char* filename, cpmm_data data ) {
	cbinary_helper bin_help = cbinary_helper ( filename, true );

	int ptr = 32;
	int count = data.m_properties.size ( );
	int ptr1 = ptr + ( 32 * count );
	int count1 = data.m_effects.size ( );
	int size = ptr1 + ( count1 * 96 );

	bin_help.write_int ( ptr );
	bin_help.write_int ( count );
	bin_help.write_int ( ptr1 );
	bin_help.write_int ( count1 );
	bin_help.write_int ( size );
	bin_help.write_padding ( 12 );

	for ( auto& prop : data.m_properties ) {
		bin_help.write_int ( prop.m_type );
		bin_help.write_float ( prop.m_start );
		bin_help.write_float ( prop.m_end );
		bin_help.write_int ( prop.m_tmp0 );
		bin_help.write_int ( prop.m_tmp1 );
		bin_help.write_int ( prop.m_tmp2 );
		bin_help.write_int ( prop.m_tmp3 );
		bin_help.write_int ( prop.m_tmp4 );
	}

	for ( auto& effect : data.m_effects ) {
		bin_help.write_int ( effect.m_play_type );
		bin_help.write_float ( effect.m_start );
		bin_help.write_float ( effect.m_end );
		bin_help.write_float ( effect.m_speed );
		bin_help.write_int ( effect.m_bone_idx );
		bin_help.write_padding ( 12 );
		bin_help.write_float ( effect.m_xyz.x );
		bin_help.write_float ( effect.m_xyz.y );
		bin_help.write_float ( effect.m_xyz.z );
		bin_help.write_int ( effect.m_type );
		bin_help.write_float ( effect.m_normal.x );
		bin_help.write_float ( effect.m_normal.y );
		bin_help.write_float ( effect.m_normal.z );
		bin_help.write_int ( effect.m_id );
		bin_help.write_float ( effect.m_tmp0.x );
		bin_help.write_float ( effect.m_tmp0.y );
		bin_help.write_float ( effect.m_tmp0.z );
		bin_help.write_int ( effect.m_tmp0_int );
		bin_help.write_float ( effect.m_tmp1.x );
		bin_help.write_float ( effect.m_tmp1.y );
		bin_help.write_float ( effect.m_tmp1.z );
		bin_help.write_int ( effect.m_tmp1_int );
	}
}