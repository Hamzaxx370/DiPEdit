#include "file.h"

#include "game\core\game.h"
#include "engine\file\binary_helper.h"
#include "game\sys\ogre\particle\particle.h"

sparticle* load_particle_file ( std::string filename ) {
	sparticle* particle = new sparticle();
	cbinary_helper bin_help = cbinary_helper ( filename.c_str ( ) );

	bin_help.seek ( 0 );
	particle->m_name = bin_help.read_fixed_string ( 12 );
	bin_help.seek ( 16 );
	int count = bin_help.read_int ( );
    bin_help.seek ( 28 );
    particle->m_unk = bin_help.read_int ( );
    bin_help.seek ( 40 );
    particle->m_id = bin_help.read_int ( );
	bin_help.seek ( 48 );

	for ( int i = 0; i < count; i++ ) {
		sparticle_param emitter;
		emitter.m_is_enabled = bin_help.read_int ( );
		emitter.m_cycle_min = bin_help.read_int ( );
		emitter.m_cycle_max = bin_help.read_int ( );
		emitter.m_emit_shape = bin_help.read_int ( );
		emitter.m_use_surface_normals = bin_help.read_int ( );
		emitter.m_transform_flag = bin_help.read_int ( );
		emitter.m_radius_min = bin_help.read_float ( );
		emitter.m_radius_max = bin_help.read_float ( );
		emitter.m_angle = bin_help.read_int ( );
		emitter.m_rot_y = bin_help.read_int ( );
		emitter.m_rot_z = bin_help.read_int ( );
		emitter.m_scale_y_min = bin_help.read_float ( );
		emitter.m_scale_y_max = bin_help.read_float ( );
		emitter.m_scale_xz_min = bin_help.read_float ( );
		emitter.m_scale_xz_max = bin_help.read_float ( );
		emitter.m_dir_angle_min = bin_help.read_int ( );
		emitter.m_dir_angle_max = bin_help.read_int ( );

		int em_pointer = bin_help.read_int ( );

		emitter.m_special_orient_flag = bin_help.read_int ( );
		int skip = 164;
		if ( cgame::get ( )->m_is_y2 )
			skip = 4;
		int returnpos = bin_help.get_pos ( ) + skip;

		bin_help.seek ( em_pointer );
		emitter.m_emitter_param.m_element_count = bin_help.read_short ( );
		emitter.m_emitter_param.m_delay_min = bin_help.read_short ( );
		emitter.m_emitter_param.m_delay_max = bin_help.read_short ( );
		emitter.m_emitter_param.m_emit_min = bin_help.read_short ( );
		emitter.m_emitter_param.m_emit_max = bin_help.read_short ( );
		emitter.m_emitter_param.m_unknown3 = bin_help.read_short ( );
		emitter.m_emitter_param.m_life_time = bin_help.read_float ( );
		emitter.m_emitter_param.m_inverse_speed = bin_help.read_float ( );
		emitter.m_emitter_param.m_unknown4 = bin_help.read_int ( );
		emitter.m_emitter_param.m_pool_size = bin_help.read_int ( );
		emitter.m_emitter_param.m_vertex_type = bin_help.read_int ( );
		emitter.m_emitter_param.m_render_state = bin_help.read_int ( );
		emitter.m_emitter_param.m_model_id = bin_help.read_short ( );
		emitter.m_emitter_param.m_texture_id = bin_help.read_short ( );
		emitter.m_emitter_param.m_element_pointer = bin_help.read_int ( );
		emitter.m_emitter_param.m_time_scale = bin_help.read_float ( );
		emitter.m_emitter_param.m_vector_param.m_base = bin_help.read_float ( );
		emitter.m_emitter_param.m_vector_param.m_vertical_base = bin_help.read_float ( );
		emitter.m_emitter_param.m_vector_param.m_multiplier = bin_help.read_float ( );
		emitter.m_emitter_param.m_vector_param.m_angle_base = bin_help.read_int ( );
		emitter.m_emitter_param.m_vector_param.m_vertical_pan = bin_help.read_float ( );
		emitter.m_emitter_param.m_vector_param.m_pos_pan = bin_help.read_float ( );
		emitter.m_emitter_param.m_vector_param.m_pos_random = bin_help.read_float ( );
		emitter.m_emitter_param.m_vector_param.m_unused = bin_help.read_int ( );
		emitter.m_emitter_param.m_vertex_param.m_color_range = glm::vec4 ( 0.0f );
		emitter.m_emitter_param.m_vertex_param.m_color_range.r = bin_help.read_float ( );
		emitter.m_emitter_param.m_vertex_param.m_color_range.g = bin_help.read_float ( );
		emitter.m_emitter_param.m_vertex_param.m_color_range.b = bin_help.read_float ( );
		emitter.m_emitter_param.m_vertex_param.m_color_range.a = bin_help.read_float ( );
		emitter.m_emitter_param.m_vertex_param.m_color_base = glm::vec4 ( 0.0f );
		emitter.m_emitter_param.m_vertex_param.m_color_base.r = bin_help.read_float ( );
		emitter.m_emitter_param.m_vertex_param.m_color_base.g = bin_help.read_float ( );
		emitter.m_emitter_param.m_vertex_param.m_color_base.b = bin_help.read_float ( );
		emitter.m_emitter_param.m_vertex_param.m_color_base.a = bin_help.read_float ( );
		emitter.m_emitter_param.m_vertex_param.m_scale_flag = bin_help.read_int ( );
		emitter.m_emitter_param.m_vertex_param.m_scale_range = glm::vec4 ( 0.0f );
		emitter.m_emitter_param.m_vertex_param.m_scale_range.w = bin_help.read_float ( );
		emitter.m_emitter_param.m_vertex_param.m_scale_range.x = bin_help.read_float ( );
		emitter.m_emitter_param.m_vertex_param.m_scale_range.y = bin_help.read_float ( );
		emitter.m_emitter_param.m_vertex_param.m_scale_range.z = bin_help.read_float ( );
		emitter.m_emitter_param.m_vertex_param.m_scale_base = glm::vec4 ( 0.0f );
		emitter.m_emitter_param.m_vertex_param.m_scale_base.w = bin_help.read_float ( );
		emitter.m_emitter_param.m_vertex_param.m_scale_base.x = bin_help.read_float ( );
		emitter.m_emitter_param.m_vertex_param.m_scale_base.y = bin_help.read_float ( );
		emitter.m_emitter_param.m_vertex_param.m_scale_base.z = bin_help.read_float ( );
		emitter.m_emitter_param.m_vertex_param.m_rotation_range = glm::vec3 ( 0.0f );
		emitter.m_emitter_param.m_vertex_param.m_rotation_range.x = static_cast< float >( bin_help.read_int ( ) );
		emitter.m_emitter_param.m_vertex_param.m_rotation_range.y = static_cast< float >( bin_help.read_int ( ) );
		emitter.m_emitter_param.m_vertex_param.m_rotation_range.z = static_cast< float >( bin_help.read_int ( ) );
		emitter.m_emitter_param.m_vertex_param.m_rotation_base = glm::vec3 ( 0.0f );
		emitter.m_emitter_param.m_vertex_param.m_rotation_base.x = static_cast< float >( bin_help.read_int ( ) );
		emitter.m_emitter_param.m_vertex_param.m_rotation_base.y = static_cast< float >( bin_help.read_int ( ) );
		emitter.m_emitter_param.m_vertex_param.m_rotation_base.z = static_cast< float >( bin_help.read_int ( ) );
		emitter.m_emitter_param.m_vertex_param.m_angular_accel_range.x = static_cast< float >( bin_help.read_int ( ) );
		emitter.m_emitter_param.m_vertex_param.m_angular_accel_range.y = static_cast< float >( bin_help.read_int ( ) );
		emitter.m_emitter_param.m_vertex_param.m_angular_accel_range.z = static_cast< float >( bin_help.read_int ( ) );
		emitter.m_emitter_param.m_vertex_param.m_angular_accel_base.x = static_cast< float >( bin_help.read_int ( ) );
		emitter.m_emitter_param.m_vertex_param.m_angular_accel_base.y = static_cast< float >( bin_help.read_int ( ) );
		emitter.m_emitter_param.m_vertex_param.m_angular_accel_base.z = static_cast< float >( bin_help.read_int ( ) );
		emitter.m_emitter_param.m_vertex_param.m_uv_flag = bin_help.read_int ( );
		emitter.m_emitter_param.m_vertex_param.m_uv_range = glm::vec2 ( bin_help.read_float ( ), bin_help.read_float ( ) );
		emitter.m_emitter_param.m_vertex_param.m_uv_base = glm::vec2 ( bin_help.read_float ( ), bin_help.read_float ( ) );
		emitter.m_emitter_param.m_vertex_param.m_columns = bin_help.read_char ( );
		emitter.m_emitter_param.m_vertex_param.m_rows = bin_help.read_char ( );
		emitter.m_emitter_param.m_vertex_param.m_start_frame = bin_help.read_char ( );
		emitter.m_emitter_param.m_vertex_param.m_end_frame = bin_help.read_char ( );
		emitter.m_emitter_param.m_vertex_param.m_width = bin_help.read_float ( );
		emitter.m_emitter_param.m_vertex_param.m_height = bin_help.read_float ( );
		bin_help.seek ( em_pointer + 244 );
		emitter.m_emitter_param.m_frame_rate = bin_help.read_float ( );

		bin_help.seek ( emitter.m_emitter_param.m_element_pointer );
		for ( int e = 0; e < emitter.m_emitter_param.m_element_count; e++ ) {
			std::vector<sparticle_element_param> chain;
			int em_ptr = bin_help.read_int ( );
			int el_returnpos = bin_help.get_pos ( );
			bin_help.seek ( em_ptr );
			while ( true ) {
				sparticle_element_param element;
				element.m_type = bin_help.read_int ( );
				element.m_effect_type = bin_help.read_int ( );
				element.m_format_flag = bin_help.read_int ( );
				element.m_end_time_scaled = bin_help.read_float ( );
				for ( int m = 0; m < 12; m++ ) {
					element.m_mtx [ m ] = bin_help.read_float ( );
				}

				chain.push_back ( element );

				if ( element.m_end_time_scaled == 1.0f )
					break;
			}
			emitter.m_emitter_param.m_element_chains.push_back ( chain );
			bin_help.seek ( el_returnpos );
		}
		particle->m_particle_params.push_back ( emitter );
		bin_help.seek ( returnpos );
	}

	return particle;
}

static inline int ANGLE_TO_INT ( float deg )
{
	return ( int ) ( deg * 65536.0f / 360.0f );
}

void write_particle_file ( const std::string& filename, sparticle* particle ) {
	bool is_y2 = cgame::get ( )->m_is_y2;
    cbinary_helper bin_help = cbinary_helper ( filename.c_str ( ), true );

    // Calculate element counts first
    int total_element_count = 0;
    int total_element_chains = 0;

    for ( auto& emitter : particle->m_particle_params ) {
        total_element_chains += emitter.m_emitter_param.m_element_count;
        for ( auto& chain : emitter.m_emitter_param.m_element_chains ) {
            total_element_count += chain.size ( );
        }
    }

    // Write header
    bin_help.write_fixed_string ( particle->m_name, 12 );
    bin_help.write_int ( 0 ); // FileSize placeholder
    bin_help.write_int ( particle->m_particle_params.size ( ) );
    bin_help.write_int ( particle->m_particle_params.size ( ) );
    bin_help.write_int ( total_element_count );
    bin_help.write_int ( 0 ); // Particle.Unk
    bin_help.write_int ( 48 ); // SPtr
    bin_help.write_int ( 0 ); // Pad

    if ( is_y2 ) {
        bin_help.write_int ( 0 ); // Particle.ID placeholder
    }

    bin_help.write_int ( 0 ); // Extra int for Y2 alignment

    // Calculate sizes
    int emitter_header_size = is_y2 ? 80 : 240;
    int header_start = 48;
    int emitter_start = header_start + ( particle->m_particle_params.size ( ) * emitter_header_size );
    int emitter_end = emitter_start + ( particle->m_particle_params.size ( ) * 272 );
    int element_end = emitter_end + ( total_element_count * 64 );
    int total_size = element_end - 48;

    // Reserve space for the data
    bin_help.write_padding ( total_size );

    // Write emitter headers
    bin_help.seek ( header_start );
    int curr_emitter_ptr = emitter_start;

    for ( auto& emitter : particle->m_particle_params ) {
        bin_help.write_int ( emitter.m_is_enabled );
        bin_help.write_int ( emitter.m_cycle_min );
        bin_help.write_int ( emitter.m_cycle_max );
        bin_help.write_int ( emitter.m_emit_shape );
        bin_help.write_int ( emitter.m_use_surface_normals );
        bin_help.write_int ( emitter.m_transform_flag );
        bin_help.write_float ( emitter.m_radius_min );
        bin_help.write_float ( emitter.m_radius_max );
        bin_help.write_int ( emitter.m_angle );
        bin_help.write_int ( emitter.m_rot_y );
        bin_help.write_int ( emitter.m_rot_z );
        bin_help.write_float ( emitter.m_scale_y_min );
        bin_help.write_float ( emitter.m_scale_y_max );
        bin_help.write_float ( emitter.m_scale_xz_min );
        bin_help.write_float ( emitter.m_scale_xz_max );
        bin_help.write_int ( emitter.m_dir_angle_min );
        bin_help.write_int ( emitter.m_dir_angle_max );
        bin_help.write_int ( curr_emitter_ptr );

        // Write unk2 data
        if ( is_y2 ) {
            bin_help.write_int ( emitter.m_special_orient_flag );
            bin_help.write_int ( 0 ); // 4 more bytes for Y2
        }
        else {
            bin_help.write_int ( emitter.m_special_orient_flag );
            bin_help.write_padding ( 164 ); // Reserve space for unk2
        }

        curr_emitter_ptr += 272;
    }

    // Write emitter parameters and elements
    int curr_element_ptr = emitter_end;
    int curr_element_grp_ptr = element_end;

    for ( auto& emitter : particle->m_particle_params ) {
        // Write emitter parameters
        bin_help.write_short ( emitter.m_emitter_param.m_element_count );
        bin_help.write_short ( emitter.m_emitter_param.m_delay_min );
        bin_help.write_short ( emitter.m_emitter_param.m_delay_max );
        bin_help.write_short ( emitter.m_emitter_param.m_emit_min );
        bin_help.write_short ( emitter.m_emitter_param.m_emit_max );
        bin_help.write_short ( emitter.m_emitter_param.m_unknown3 );
        bin_help.write_float ( emitter.m_emitter_param.m_life_time );
        bin_help.write_float ( emitter.m_emitter_param.m_inverse_speed );
        bin_help.write_int ( emitter.m_emitter_param.m_unknown4 );
        bin_help.write_int ( emitter.m_emitter_param.m_pool_size );
        bin_help.write_int ( emitter.m_emitter_param.m_vertex_type );
        bin_help.write_int ( emitter.m_emitter_param.m_render_state );
        bin_help.write_short ( emitter.m_emitter_param.m_model_id );
        bin_help.write_short ( emitter.m_emitter_param.m_texture_id );
        bin_help.write_int ( curr_element_grp_ptr );
        bin_help.write_float ( emitter.m_emitter_param.m_time_scale );

        // Vector parameters
        bin_help.write_float ( emitter.m_emitter_param.m_vector_param.m_base );
        bin_help.write_float ( emitter.m_emitter_param.m_vector_param.m_vertical_base );
        bin_help.write_float ( emitter.m_emitter_param.m_vector_param.m_multiplier );
        bin_help.write_int ( emitter.m_emitter_param.m_vector_param.m_angle_base );
        bin_help.write_float ( emitter.m_emitter_param.m_vector_param.m_vertical_pan );
        bin_help.write_float ( emitter.m_emitter_param.m_vector_param.m_pos_pan );
        bin_help.write_float ( emitter.m_emitter_param.m_vector_param.m_pos_random );
        bin_help.write_int ( emitter.m_emitter_param.m_vector_param.m_unused );

        // Vertex parameters - color
        bin_help.write_float ( emitter.m_emitter_param.m_vertex_param.m_color_range.r );
        bin_help.write_float ( emitter.m_emitter_param.m_vertex_param.m_color_range.g );
        bin_help.write_float ( emitter.m_emitter_param.m_vertex_param.m_color_range.b );
        bin_help.write_float ( emitter.m_emitter_param.m_vertex_param.m_color_range.a );
        bin_help.write_float ( emitter.m_emitter_param.m_vertex_param.m_color_base.r );
        bin_help.write_float ( emitter.m_emitter_param.m_vertex_param.m_color_base.g );
        bin_help.write_float ( emitter.m_emitter_param.m_vertex_param.m_color_base.b );
        bin_help.write_float ( emitter.m_emitter_param.m_vertex_param.m_color_base.a );

        // Vertex parameters - scale
        bin_help.write_int ( emitter.m_emitter_param.m_vertex_param.m_scale_flag );
        bin_help.write_float ( emitter.m_emitter_param.m_vertex_param.m_scale_range.w );
        bin_help.write_float ( emitter.m_emitter_param.m_vertex_param.m_scale_range.x );
        bin_help.write_float ( emitter.m_emitter_param.m_vertex_param.m_scale_range.y );
        bin_help.write_float ( emitter.m_emitter_param.m_vertex_param.m_scale_range.z );
        bin_help.write_float ( emitter.m_emitter_param.m_vertex_param.m_scale_base.w );
        bin_help.write_float ( emitter.m_emitter_param.m_vertex_param.m_scale_base.x );
        bin_help.write_float ( emitter.m_emitter_param.m_vertex_param.m_scale_base.y );
        bin_help.write_float ( emitter.m_emitter_param.m_vertex_param.m_scale_base.z );

        // Vertex parameters - rotation
        bin_help.write_int ( static_cast< int >( emitter.m_emitter_param.m_vertex_param.m_rotation_range.x ) );
        bin_help.write_int ( static_cast< int >( emitter.m_emitter_param.m_vertex_param.m_rotation_range.y ) );
        bin_help.write_int ( static_cast< int >( emitter.m_emitter_param.m_vertex_param.m_rotation_range.z ) );
        bin_help.write_int ( static_cast< int >( emitter.m_emitter_param.m_vertex_param.m_rotation_base.x ) );
        bin_help.write_int ( static_cast< int >( emitter.m_emitter_param.m_vertex_param.m_rotation_base.y ) );
        bin_help.write_int ( static_cast< int >( emitter.m_emitter_param.m_vertex_param.m_rotation_base.z ) );

        // Vertex parameters - angular acceleration
        bin_help.write_int ( static_cast< int >( emitter.m_emitter_param.m_vertex_param.m_angular_accel_range.x ) );
        bin_help.write_int ( static_cast< int >( emitter.m_emitter_param.m_vertex_param.m_angular_accel_range.y ) );
        bin_help.write_int ( static_cast< int >( emitter.m_emitter_param.m_vertex_param.m_angular_accel_range.z ) );
        bin_help.write_int ( static_cast< int >( emitter.m_emitter_param.m_vertex_param.m_angular_accel_base.x ) );
        bin_help.write_int ( static_cast< int >( emitter.m_emitter_param.m_vertex_param.m_angular_accel_base.y ) );
        bin_help.write_int ( static_cast< int >( emitter.m_emitter_param.m_vertex_param.m_angular_accel_base.z ) );

        // Vertex parameters - UV
        bin_help.write_int ( emitter.m_emitter_param.m_vertex_param.m_uv_flag );
        bin_help.write_float ( emitter.m_emitter_param.m_vertex_param.m_uv_range.x );
        bin_help.write_float ( emitter.m_emitter_param.m_vertex_param.m_uv_range.y );
        bin_help.write_float ( emitter.m_emitter_param.m_vertex_param.m_uv_base.x );
        bin_help.write_float ( emitter.m_emitter_param.m_vertex_param.m_uv_base.y );
        bin_help.write_char ( emitter.m_emitter_param.m_vertex_param.m_columns );
        bin_help.write_char ( emitter.m_emitter_param.m_vertex_param.m_rows );
        bin_help.write_char ( emitter.m_emitter_param.m_vertex_param.m_start_frame );
        bin_help.write_char ( emitter.m_emitter_param.m_vertex_param.m_end_frame );
        bin_help.write_float ( emitter.m_emitter_param.m_vertex_param.m_width );
        bin_help.write_float ( emitter.m_emitter_param.m_vertex_param.m_height );

        // Data1 placeholder (16 bytes)
        bin_help.write_padding ( 16 );

        // Frame rate
        bin_help.write_float ( emitter.m_emitter_param.m_frame_rate );

        // Data2 placeholder (24 bytes)
        bin_help.write_padding ( 24 );

        // Save current position
        int return_pos = bin_help.get_pos ( );

        // Write element chains
        bin_help.seek ( curr_element_grp_ptr );

        for ( auto& chain : emitter.m_emitter_param.m_element_chains ) {
            bin_help.write_int ( curr_element_ptr );

            // Save group pointer position
            int group_ptr_pos = bin_help.get_pos ( );

            // Write elements in this chain
            bin_help.seek ( curr_element_ptr );
            for ( auto& element : chain ) {
                bin_help.write_int ( element.m_type );
                bin_help.write_int ( element.m_effect_type );
                bin_help.write_int ( element.m_format_flag );
                bin_help.write_float ( element.m_end_time_scaled );

                for ( int m = 0; m < 12; m++ ) {
                    bin_help.write_float ( element.m_mtx [ m ] );
                }

                curr_element_ptr += 64;
            }

            // Move to next group pointer
            curr_element_grp_ptr += 4;
            bin_help.seek ( curr_element_grp_ptr );
        }

        // Return to write position for next emitter
        bin_help.seek ( return_pos );
    }

    // Calculate final size and write it
        // Align to 16 bytes
    bin_help.seek ( curr_element_grp_ptr );
    bin_help.align ( 16 );
    int final_size = bin_help.get_pos ( );
    bin_help.seek ( 12 ); // FileSize position
    bin_help.write_int ( final_size );

    bin_help.seek ( 40 );
    if ( is_y2 )
		bin_help.write_int ( particle->m_id );

}

void get_particle_info ( std::string filename, std::string& ptcl_name, int& id ) {
    cbinary_helper bin_help = cbinary_helper ( filename.c_str ( ) );

    ptcl_name = bin_help.read_fixed_string ( 12 );
    bin_help.seek ( 40 );
    id = bin_help.read_int ( );
};