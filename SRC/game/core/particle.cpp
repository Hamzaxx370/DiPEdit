#include "particle.h"
#include "engine\core\engine.h"
#include "engine\sys\render\render.h"
#include "game\core\game.h"
#include "game\file\file.h"
#include "ogre_maths.h"

// ============================================================================
// Helper Functions
// ============================================================================

int calculate_random_angle_offset ( unsigned int random_bits, int base_angle ) {
    int result = ( ( int ( random_bits & 0xffff ) >> 4 ) * ( base_angle * 2 ) ) - ( base_angle << 11 );
    return result >> 0xb;
}

void exec_uni_veloc ( float* data, float dt ) {
    data [ 0 ] += data [ 4 ] * dt;
    data [ 1 ] += data [ 5 ] * dt;
    data [ 2 ] += data [ 6 ] * dt;
    data [ 3 ] += data [ 7 ] * dt;
}

void exec_uni_accel ( float* data, float dt ) {
    data [ 0 ] += data [ 4 ] * dt;
    data [ 1 ] += data [ 5 ] * dt;
    data [ 2 ] += data [ 6 ] * dt;
    data [ 3 ] += data [ 7 ] * dt;

    data [ 4 ] += data [ 8 ] * dt;
    data [ 5 ] += data [ 9 ] * dt;
    data [ 6 ] += data [ 10 ] * dt;
    data [ 7 ] += data [ 11 ] * dt;
}

void meffector_store_tex_coord ( float* src, float* dst ) {
    dst [ 0 ] = src [ 0 ];
    dst [ 1 ] = src [ 1 ];
}

void meffector_store_stepped_tex_coord ( float* src, float* dst, int columns, int rows ) {
    if ( columns <= 0 ) columns = 1;
    if ( rows <= 0 ) rows = 1;

    float cell_w = 1.0f / columns;
    float cell_h = 1.0f / rows;

    int total_frames = columns * rows;
    int current_frame = ( int ) src [ 0 ];
    current_frame = current_frame % total_frames;

    int col_idx = current_frame % columns;
    int row_idx = current_frame / columns;

    dst [ 0 ] = col_idx * cell_w;
    dst [ 1 ] = row_idx * cell_h;
}

// ============================================================================
// Particle Output Initialization
// ============================================================================

void init_particle_output ( sparticle_vertex_param vxp, sparticle_output* output ) {
    glm::vec4 rnd_vec1 = glm::vec4 ( frandom ( ), frandom ( ), frandom ( ), frandom ( ) );

    if ( vxp.m_scale_flag == 2 ) {
        rnd_vec1 *= vxp.m_scale_range;
        rnd_vec1 += vxp.m_scale_base;
        output->m_scale = rnd_vec1;
    }
    else {
        output->m_scale = glm::vec4 ( vxp.m_scale_range.w * rnd_vec1.w + vxp.m_scale_base.w );
    }

    // Initialize color
    glm::vec4 rnd_vec2 = glm::vec4 ( frandom ( ), frandom ( ), frandom ( ), frandom ( ) );
    rnd_vec2 *= vxp.m_color_range;
    rnd_vec2 += vxp.m_color_base;

    output->m_rgba = ( ( unsigned int ) ( rnd_vec2.a * 255 ) << 24 ) |
        ( ( unsigned int ) ( rnd_vec2.r * 255 ) << 16 ) |
        ( ( unsigned int ) ( rnd_vec2.g * 255 ) << 8 ) |
        ( unsigned int ) ( rnd_vec2.b * 255 );

    // Initialize rotation
    glm::vec3 rnd_vec3 = glm::vec3 ( frandom ( ), frandom ( ), frandom ( ) );
    rnd_vec3 *= vxp.m_rotation_range;
    rnd_vec3 += vxp.m_rotation_base;
    output->m_rotation = rnd_vec3;

    glm::vec2 rnd_vec4 = glm::vec2 ( frandom ( ), frandom ( ) );
    rnd_vec4 *= vxp.m_uv_range;
    rnd_vec4 += vxp.m_uv_base;
    output->m_uv = rnd_vec4;

    // Initialize timing (life counts DOWN from lifespan to 0)
    output->m_life = 0.0f;  // Will be set to actual life when spawned
    output->m_position = glm::vec3 ( 0.0f );
}

void change_scale ( int vert_type, uint32_t scale_flags, glm::vec4& scale ) {
    // Only valid for vertex types 2 and 3
    if ( vert_type != 2 && vert_type != 3 )
        return;

    uint32_t flags = scale_flags;

    if ( ( flags & 0x04 ) == 0 ) {
        scale.w = scale.x;
        flags = scale_flags;
    }

    // If X not explicitly defined
    if ( ( flags & 0x08 ) == 0 ) {
        scale.x = scale.y;
        flags = scale_flags;
    }
    else {
        scale.x = scale.w;
        flags = scale_flags;
    }

    // If Y not explicitly defined
    if ( ( flags & 0x10 ) == 0 ) {
        scale.y = scale.z;
        return;
    }

    scale.y = scale.w;
}

void change_material ( sparticle_vertex_param vxp, sparticle_output* output ) {
    uint32_t material_flag = vxp.m_material_flag;

    if ( material_flag == 0 )
        return;

    unsigned char source_channel = 0;

    // Extract color components
    unsigned char r = ( output->m_rgba >> 16 ) & 0xFF;
    unsigned char g = ( output->m_rgba >> 8 ) & 0xFF;
    unsigned char b = output->m_rgba & 0xFF;
    unsigned char a = ( output->m_rgba >> 24 ) & 0xFF;

    // Determine source channel based on material flag
    uint32_t channel = material_flag & 0xFF;
    if ( channel != 0 ) {
        if ( channel == 1 ) {
            source_channel = g;
        }
        else if ( channel == 2 ) {
            source_channel = b;
        }
        else if ( channel == 3 ) {
            source_channel = a;
        }
        else if ( channel == 4 ) {
            source_channel = r;
        }
    }

    // Apply to specified channels
    if ( ( material_flag & 0x100 ) != 0 ) {
        g = source_channel;
    }

    if ( ( material_flag & 0x200 ) != 0 ) {
        b = source_channel;
    }

    if ( ( material_flag & 0x400 ) != 0 ) {
        a = source_channel;
    }

    if ( ( material_flag & 0x800 ) != 0 ) {
        r = source_channel;
    }

    // Reconstruct RGBA
    output->m_rgba = ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | b;
}

// ============================================================================
// cparticle Implementation
// ============================================================================

bool cparticle::initialize ( sparticle_param param, sparticle_data data, glm::vec3 vertex ) {
    m_param = param;
    m_enabled = param.m_is_enabled;
    m_finished = false;

    if ( !m_enabled ) {
        m_finished = true;
        return false;
    }

    m_gen_num = m_param.m_cycle_min + frandom ( ) * ( m_param.m_cycle_max - m_param.m_cycle_min );
    m_emitter = new cparticle_emitter [ m_gen_num ];

    for ( int i = 0; i < m_gen_num; i++ ) {
        m_emitter [ i ].m_pos = data.m_pos;
        m_emitter [ i ].m_normal = data.m_normal;
        m_emitter [ i ].m_tmp0 = data.m_tmp0;
        m_emitter [ i ].m_vertex = vertex;

        bool success = m_emitter [ i ].initialize ( m_param, m_param.m_emitter_param, vertex );

        if ( !success )
            return false;
    }
    
    int mdl_id = m_param.m_emitter_param.m_model_id;
    int tex_id = m_param.m_emitter_param.m_texture_id;

    m_meshes = read_ogre_mesh_file (
        ( cgame::get ( )->m_particle_path + "\\" + std::to_string ( mdl_id ) + ".OME" ).c_str ( ),
        2,
        "Shaders\\vertex_general.glsl",
        "Shaders\\fragment_particle.glsl" );

    for ( auto& mesh : m_meshes ) {
        cmesh_buffer* m = mesh.get ( );
        m->m_used_tex = read_ogre_tex_file ( ( cgame::get ( )->m_particle_path + "\\" + std::to_string ( tex_id ) + ".TXB" ).c_str ( ) );
    }

    return true;
}

void cparticle::execute ( ) {
    if ( !m_emitter || !m_enabled ) {
        return;
    }

    m_finished = true;
    for ( int i = 0; i < m_gen_num; i++ ) {
        m_emitter [ i ].execute ( );
        if ( !m_emitter [ i ].m_finished ) {
            m_finished = false;
        }
    }
}

void cparticle::release ( ) {
    if ( !m_emitter || !m_enabled ) {
        return;
    }

    for ( int i = 0; i < m_gen_num; i++ ) {
        m_emitter [ i ].release ( );
    }

    delete [ ] m_emitter;
    m_emitter = nullptr;
}

void cparticle::draw ( ) {
    if ( !m_emitter || !m_enabled ) {
        return;
    }

    for ( int i = 0; i < m_gen_num; i++ ) {
        m_emitter [ i ].draw ( m_meshes );
    }
}

// ============================================================================
// cparticle_emitter Implementation
// ============================================================================

bool cparticle_emitter::initialize ( sparticle_param particle_param, sparticle_emitter_param emitter_param, glm::vec3 vertex ) {
    m_particle_param = particle_param;
    m_gen_rate = 0.0f;
    m_rotate_x = 0;
    m_element_chain_param = emitter_param.m_element_chains;
    m_pool_size = emitter_param.m_pool_size;
    m_emission_num = 0.0f;
    m_param = emitter_param;

    m_gen_num = 0.0f;

    m_finished = false;

    // Frame rate handling from decompilation
    float frame_rate_factor = m_param.m_frame_rate / 30.0f;
    if ( frame_rate_factor - 1.0f < 0.0f ) {
        m_gen_rate = 1.0f;
    }
    else {
        m_gen_rate = frame_rate_factor;
    }

    m_vertex = vertex;

    // Delay and emission duration from decompilation
    m_delay_duration = m_param.m_delay_min + frandom ( ) * ( m_param.m_delay_max - m_param.m_delay_min );
    m_delay_num = 0;

    // Emission duration logic
    if ( m_param.m_emit_max == -1 || m_param.m_emit_min == -1 ) {
        m_emission_duration = -1.0f;
    }
    else {
        int emit_max = abs ( m_param.m_emit_max );
        int emit_min = abs ( m_param.m_emit_min );
        m_emission_duration = emit_min + frandom ( ) * ( emit_max - emit_min );
    }

    // Initialize arrays
    m_output.resize ( m_pool_size );
    m_drawables.resize ( m_pool_size );
    m_matrix.resize ( m_pool_size );
    m_random_param.resize ( m_pool_size );
    m_output_chains.resize ( m_pool_size, nullptr );

    // Initialize all particles as dead (life = 0 means available for spawning)
    for ( auto& p : m_output ) {
        p.m_life = 0.0f;
    }

    return true;
}

const float FRAME_SPD = 1.0f / 30.0f;

void cparticle_emitter::execute ( ) {
    if ( m_emission_num >= m_emission_duration ) {
        m_finished = true;
        for ( int i = 0; i < m_pool_size; ++i ) {
            if ( m_output_chains [ i ] ) {
                m_finished = false;
            }
        }
    }

    //printf ( "%f, %f\n", m_emission_num, m_emission_duration );

    if ( m_finished ) return;

    float dt = cengine::get ( )->render_man->get_delta ( ) / FRAME_SPD;
    
    // Apply time scaling (from decompilation analysis)
    float scaled_dt = dt;

    // Delay gate
    if ( m_delay_num < m_delay_duration ) {
        m_delay_num += dt;
        return;
    }

    // Check emission duration
    if ( !( m_emission_duration >= 0.0f && m_emission_num >= m_emission_duration ) ) {
        if ( m_gen_rate > 0.0f ) {
            m_gen_num += m_gen_rate * dt;
            m_emission_num += dt;

            int spawn_count = ( int ) m_gen_num;
            if ( spawn_count > 0 ) {
                m_gen_num -= spawn_count;

                // 3. Spawn new particles
                for ( int i = 0; i < m_pool_size && spawn_count > 0; ++i ) {
                    if ( m_output [ i ].m_life <= 0.0f ) { // Available slot
                        // 1. Retrieve base parameters
                        float total_life = m_param.m_life_time;
                        float inv_speed_scale = m_param.m_inverse_speed;

                        // 2. Generate the Random Phase
                        float rnd = frandom ( );

                        // 3. Calculate 'initial_life' (The Random Start Offset)
                        // Decomp: initial_life = (float)((double)(float)(dVar1 * rand_float) * unk_life);
                        float initial_life_offset = ( total_life * rnd ) * inv_speed_scale;

                        // 4. Calculate 'unk_life' (The Compensated Chain Duration)
                        // Decomp: unk_life = -(double)(float)(dVar1 * unk_life - (double)(float)(dVar1 + (double)(initial_life + initial_life)));
                        // Logic: (Total + 2*Offset) - (Total * InvSpeed)
                        float chain_life_duration = ( total_life + 2.0f * initial_life_offset ) - ( total_life * inv_speed_scale );

                        // 5. Calculate 'chain_param' (The Playback Speed Factor)
                        // Decomp: chain_param = (double)(float)(dVar1 * (double)(float)(rand_float / unk_life));
                        // Note: If chain_life_duration is 0 (rare edge case), prevent division by zero.
                        float chain_speed_param = 1.0f;
                        if ( abs ( chain_life_duration ) > 0.0001f ) {
                            chain_speed_param = total_life * ( rnd / chain_life_duration );
                        }

                        // Initialize particle with full life
                        init_particle_output ( m_param.m_vertex_param, &m_output [ i ] );
                        m_output [ i ].m_life = m_param.m_life_time;

                        // Create matrix + random parameters
                        m_matrix [ i ] = create_matrix ( m_random_param [ i ] );

                        // Build element chain
                        cparticle_element* head = nullptr;
                        cparticle_element* prev = nullptr;

                        for ( auto& chain : m_element_chain_param ) {
                            auto* elem = new cparticle_element ( );
                            elem->initialize ( chain );
                            elem->init_params (
                                m_param.m_vertex_param,
                                m_param.m_vector_param,
                                m_param.m_unknown4
                            );

                            if ( !head ) {
                                head = elem;
                                elem->m_time_scale = chain_speed_param;
                                elem->m_life_time = chain_life_duration;
                                elem->m_end_time = chain_life_duration * elem->m_params [ 0 ].m_end_time_scaled;
                            }
                            else if ( prev ) {
                                head->create_chain ( elem, chain_life_duration, chain_speed_param );
                            }
                            prev = elem;
                        }

                        std::vector<sparticle_element_param> final_param_vec;
                        sparticle_element_param final_param;
                        final_param.m_type = 0;
                        final_param.m_effect_type = 1;
                        final_param.m_format_flag = 0;
                        final_param.m_end_time_scaled = 1.0f;
                        memset ( final_param.m_mtx, 0, 0x30 );
                        final_param_vec.push_back ( final_param );

                        auto* final_elem = new cparticle_element ( );
                        final_elem->initialize ( final_param_vec );
                        final_elem->init_params (
                            m_param.m_vertex_param,
                            m_param.m_vector_param,
                            m_param.m_unknown4
                        );

                        final_elem->m_life_time = m_param.m_life_time;
                        final_elem->m_end_time = m_param.m_life_time;
                        final_elem->m_time_scale = 1.0f;
                        final_elem->m_current_life = 0.0f;

                        if ( !head ) {
                            head = final_elem;
                        }
                        else if ( prev ) {
                            prev->m_next = final_elem;
                        }

                        if ( m_output_chains [ i ] ) {
                            cparticle_element* curr = m_output_chains [ i ];
                            while ( curr ) {
                                cparticle_element* next = curr->m_next;
                                delete curr;
                                curr = next;
                            }
                            m_output_chains [ i ] = nullptr;
                        }

                        m_output_chains [ i ] = head;

                        spawn_count--;
                    }
                }
            }
        }
    }

    for ( int i = 0; i < m_pool_size; ++i ) {
        if ( m_output [ i ].m_life > 0.0f ) {
            // Life counts down
            m_output [ i ].m_life -= scaled_dt;

            // If still alive, update element chain
            if ( m_output [ i ].m_life > 0.0f && !m_output_chains [ i ]->is_finished ( ) ) {
                m_output_chains [ i ]->execute ( scaled_dt );
                m_output_chains [ i ]->output ( &m_output [ i ], m_param.m_vertex_param );
            }
            else {
                // Particle died, mark as available
                m_output [ i ].m_life = 0.0f;
                if ( m_output_chains [ i ] ) {
                    cparticle_element* curr = m_output_chains [ i ];
                    while ( curr ) {
                        cparticle_element* next = curr->m_next;
                        delete curr;
                        curr = next;
                    }
                    m_output_chains [ i ] = nullptr;
                }
            }
        }
    }


}

void cparticle_emitter::release ( ) {
    // Clean up element chains
    for ( auto& block : m_output_chains ) {
        if ( block ) {
            cparticle_element* curr = block;
            while ( curr ) {
                cparticle_element* next = curr->m_next;
                delete curr;
                curr = next;
            }
        }
    }
    m_output_chains.clear ( );

    // Clear arrays
    m_output.clear ( );
    m_drawables.clear ( );
    m_matrix.clear ( );
    m_random_param.clear ( );
}

void cparticle_emitter::draw ( std::vector<cmesh_ref> meshes ) {
    for ( int i = 0; i < m_pool_size; i++ ) {
        sparticle_output& o = m_output [ i ];
        cdraw_particle& d = m_drawables [ i ];

        // Only draw living particles
        if ( o.m_life > 0.0f ) {
            
            //change_material ( m_param.m_vertex_param, &o );
            //change_scale ( m_param.m_vertex_type, m_param.m_vertex_param.m_scale_flag, o.m_scale );

            if ( m_param.m_vertex_param.m_scale_flag != 2 ) {
                o.m_scale = glm::vec4 ( o.m_scale.w );
            }

            // Create final matrix
            m_matrix [ i ] = set_matrix (
                m_particle_param,
                m_pos,
                m_normal,
                m_tmp0,
                m_random_param [ i ]
            );

            d.m_meshes = meshes;
            d.m_output = &o;
            d.m_mtx = m_attach * m_matrix [ i ];
            d.m_vert_type = m_param.m_vertex_type;

            cengine::get ( )->render_man->regist_draw ( &d, e_draw_type::particle );
        }
    }
}

glm::mat4 cparticle_emitter::create_matrix ( sparticle_random_param& random_param ) {
    float cos_dir1, sin_dir1;
    float cos_dir2, sin_dir2;

    uint32_t dir_angle1 = random ( );
    uint32_t dir_angle2 = random ( );

    // Calculate random direction angle
    float dir_angle_range = ( float ) ( m_particle_param.m_dir_angle_max - m_particle_param.m_dir_angle_min );
    float rand_dir_angle_f = frandom ( ) * dir_angle_range + ( float ) m_particle_param.m_dir_angle_min;
    int rand_dir_angle_int = static_cast< int >( rand_dir_angle_f );

    // Calculate direction vectors
    int angle1 = calculate_random_angle_offset ( dir_angle1, rand_dir_angle_int );
    int angle2 = calculate_random_angle_offset ( dir_angle2, rand_dir_angle_int );

    sin_cos ( &cos_dir1, &sin_dir1, ( int16_t ) angle1 );
    sin_cos ( &cos_dir2, &sin_dir2, ( int16_t ) angle2 );

    glm::vec3 random_dir;
    random_dir.x = sin_dir2 * cos_dir1;
    random_dir.y = cos_dir2;
    random_dir.z = sin_dir2 * sin_dir1;

    uint32_t rand_rot1 = random ( );
    uint32_t rand_rot2 = random ( );

    int rot_y_int = ( ( ( rand_rot1 & 0x7FFF ) >> 4 ) * ( int ) m_particle_param.m_rot_y ) >> 10;
    rot_y_int -= ( int ) m_particle_param.m_rot_y;

    int rot_z_int = ( ( ( rand_rot2 & 0x7FFF ) >> 4 ) * ( int ) m_particle_param.m_rot_z ) >> 10;
    rot_z_int -= ( int ) m_particle_param.m_rot_z;

    float rot_y_rad = ROT_Y_TO_FLOAT ( rot_y_int );
    float rot_z_rad = ROT_Y_TO_FLOAT ( rot_z_int );

    float scale_y = frandom ( ) * ( m_particle_param.m_scale_y_max - m_particle_param.m_scale_y_min ) +
        m_particle_param.m_scale_y_min;

    float scale_xz = frandom ( ) * ( m_particle_param.m_scale_xz_max - m_particle_param.m_scale_xz_min ) +
        m_particle_param.m_scale_xz_min;

    random_param.m_dir = glm::normalize ( random_dir );
    random_param.roty = rot_y_rad;
    random_param.rotz = rot_z_rad;
    random_param.m_scale = glm::vec3 ( scale_xz, scale_y, scale_xz );
    random_param.m_pos = glm::vec3 ( 0.0f );

    // Calculate initial position
    random_param.m_pos = calc_pos ( );

    // Create final matrix
    glm::mat4 matrix = set_matrix (
        m_particle_param,
        m_pos,
        m_normal,
        m_tmp0,
        random_param
    );

    return matrix;
}

glm::vec3 cparticle_emitter::calc_pos ( ) {
    glm::vec3 outpos;
    float s = 0.0f;
    float c = 0.0f;
    float s_s = 0.0f;
    float c_s = 0.0f;

    float r_min = ( float ) m_particle_param.m_radius_min;
    if ( r_min < 0.1f ) r_min = 0.1f;

    float r_max = ( float ) m_particle_param.m_radius_max;
    if ( r_max < r_min ) r_max = r_min;

    float radius = r_min + frandom ( ) * ( r_max - r_min );

    int max_angle = m_particle_param.m_angle;

    unsigned int angle_circle = random ( );
    int circle_offset = calculate_random_angle_offset ( angle_circle, max_angle );
    sin_cos ( &c, &s, ( int16_t ) circle_offset );

    if ( m_particle_param.m_emit_shape == 0 ) {
        // Circle (Flat)
        outpos [ 0 ] = radius * c;
        outpos [ 1 ] = 0.0f;
        outpos [ 2 ] = radius * s;
    }
    else {
        // Sphere
        unsigned int angle_sphere = random ( );
        int sphere_offset = calculate_random_angle_offset ( angle_sphere, max_angle );
        sin_cos ( &c_s, &s_s, ( int16_t ) sphere_offset );

        outpos [ 0 ] = ( radius * s_s ) * c;
        outpos [ 1 ] = radius * c_s;
        outpos [ 2 ] = ( radius * s_s ) * s;
    }

    return outpos;
}

glm::mat4 cparticle_emitter::set_matrix (
    sparticle_param particle_param,
    glm::vec3 pos,
    glm::vec3 normal,
    glm::vec3 tmp0,
    sparticle_random_param random_param
) {
    // Stage 1: Initial Matrix for Spawn Position
    glm::mat4 spawn_mtx = glm::mat4 ( 1.0f );

    const glm::vec3& p1 = ( particle_param.m_use_surface_normals == 0 ) ? normal : tmp0;
    const glm::vec3& p2 = ( particle_param.m_use_surface_normals == 0 ) ? tmp0 : normal;

    // Calculate initial orientation
    float spawn_ay = std::atan2 ( p1.x, p1.z );
    float spawn_ax = std::atan2 ( -p1.y, std::hypot ( p1.x, p1.z ) );

    spawn_mtx = glm::rotate ( spawn_mtx, spawn_ay, glm::vec3 ( 0, 1, 0 ) );
    spawn_mtx = glm::rotate ( spawn_mtx, spawn_ax + m_rotate_x, glm::vec3 ( 1, 0, 0 ) );

    // Align secondary axis
    glm::vec3 transformed_norm = glm::transpose ( glm::mat3 ( spawn_mtx ) ) * p2;
    float spawn_az = std::atan2 ( -transformed_norm.x, transformed_norm.y );
    spawn_mtx = glm::rotate ( spawn_mtx, spawn_az, glm::vec3 ( 0, 0, 1 ) );

    // Transform local random position into world-space offset
    glm::vec3 spawn_pos_offset = glm::mat3 ( spawn_mtx ) * random_param.m_pos;

    // Stage 2: Final Matrix Construction
    glm::mat4 final_mtx = glm::mat4 ( 1.0f );
    final_mtx = glm::translate ( final_mtx, pos );
    final_mtx = glm::translate ( final_mtx, spawn_pos_offset );

    // Special orientation flag logic
    if ( particle_param.m_special_orient_flag == 1 ) {
        // Align to normal
        final_mtx = glm::rotate ( final_mtx, std::atan2 ( normal.x, normal.z ), glm::vec3 ( 0, 1, 0 ) );

        // Apply random direction
        float rand_ay = std::atan2 ( random_param.m_dir.x, random_param.m_dir.z );
        float rand_ax = std::atan2 ( -random_param.m_dir.y, std::hypot ( random_param.m_dir.x, random_param.m_dir.z ) );

        final_mtx = glm::rotate ( final_mtx, rand_ay, glm::vec3 ( 0, 1, 0 ) );
        final_mtx = glm::rotate ( final_mtx, rand_ax + m_rotate_x, glm::vec3 ( 1, 0, 0 ) );
    }
    else {
        // Standard transformation logic
        glm::vec3 basis;
        if ( particle_param.m_transform_flag == 0 )      basis = normal;
        else if ( particle_param.m_transform_flag == 1 ) basis = tmp0;
        else if ( particle_param.m_transform_flag == 2 ) basis = glm::normalize ( spawn_pos_offset );
        else                                    basis = spawn_pos_offset;

        // Basis rotation
        final_mtx = glm::rotate ( final_mtx, std::atan2 ( basis.x, basis.z ), glm::vec3 ( 0, 1, 0 ) );
        final_mtx = glm::rotate ( final_mtx, std::atan2 ( -basis.y, std::hypot ( basis.x, basis.z ) ) + m_rotate_x, glm::vec3 ( 1, 0, 0 ) );

        // Random direction rotation
        float rand_ay = std::atan2 ( random_param.m_dir.x, random_param.m_dir.z );
        float rand_ax = std::atan2 ( -random_param.m_dir.y, std::hypot ( random_param.m_dir.x, random_param.m_dir.z ) );

        final_mtx = glm::rotate ( final_mtx, rand_ay, glm::vec3 ( 0, 1, 0 ) );
        final_mtx = glm::rotate ( final_mtx, rand_ax + m_rotate_x, glm::vec3 ( 1, 0, 0 ) );

        // Final normal alignment
        glm::vec3 sec_norm = ( particle_param.m_use_surface_normals == 0 ) ? tmp0 : normal;
        glm::vec3 final_trans_norm = glm::transpose ( glm::mat3 ( final_mtx ) ) * sec_norm;
        float final_az = std::atan2 ( -final_trans_norm.x, final_trans_norm.y );

        final_mtx = glm::rotate ( final_mtx, final_az, glm::vec3 ( 0, 0, 1 ) );
    }

    // Apply random rotations and scaling
    final_mtx = glm::rotate ( final_mtx, random_param.roty, glm::vec3 ( 0, 1, 0 ) );
    final_mtx = glm::rotate ( final_mtx, random_param.rotz, glm::vec3 ( 0, 0, 1 ) );
    final_mtx = glm::scale ( final_mtx, random_param.m_scale );

    return final_mtx;
}

// ============================================================================
// cparticle_element Implementation
// ============================================================================

void cparticle_element::initialize ( std::vector<sparticle_element_param> params ) {
    m_params = params;
    m_life_time = 0.0f;
    m_time_scale = 1.0f;
    m_end_time = 0.0f;
    m_current_life = 0.0f;
    m_next = nullptr;

    // Count how many keyframes we have (until end_time_scaled reaches 1.0)
    int counter = 1;
    int index = 0;

    while ( index < params.size ( ) && params [ index ].m_end_time_scaled < 1.0f ) {
        counter++;
        index++;
    }

    m_key_curr = 0;
    m_key_max = index + 1;
}

void cparticle_element::execute ( float dt ) {
    // Apply time scaling
    float step = dt * m_time_scale;


    while ( step > 0.0f && m_key_curr < m_key_max ) {
        float remaining = m_end_time - m_current_life;

        if ( m_params [ 0 ].m_type == 0 ) {
            double end_time = ( double ) m_end_time;
            double next_time = m_current_life + step;

            if ( end_time <= next_time ) {
                // Partial execution for remaining time
                double partial = end_time - m_current_life; // (dVar5 + dVar7) - dVar7... simplified
                // Execute Effector (Offset 0x2c)
                if ( m_params [ m_key_curr ].m_effect_type == 0 ) {
                    exec_uni_veloc ( m_effector_data, partial );
                }
                else {
                    exec_uni_accel ( m_effector_data, partial );
                }
            }
            else {
                // Full execution
                if ( m_params [ m_key_curr ].m_effect_type == 0 ) {
                    exec_uni_veloc ( m_effector_data, step );
                }
                else {
                    exec_uni_accel ( m_effector_data, step );
                }
            }

            // Advance time
            m_current_life = ( float ) ( m_current_life + step );
            step = 0.0f;
        }
        else {
            if ( step <= remaining ) {
                // Update within current keyframe
                if ( m_params [ m_key_curr ].m_effect_type == 0 ) {
                    exec_uni_veloc ( m_effector_data, step );
                }
                else {
                    exec_uni_accel ( m_effector_data, step );
                }

                m_current_life += step;
                step = 0.0f;
            }
            else {
                // Finish current keyframe
                if ( m_params [ m_key_curr ].m_effect_type == 0 ) {
                    exec_uni_veloc ( m_effector_data, remaining );
                }
                else {
                    exec_uni_accel ( m_effector_data, remaining );
                }

                step -= remaining;
                m_current_life = m_end_time;

                if ( m_key_curr + 1 < m_key_max ) {
                    m_key_curr++;

                    m_current_life = m_end_time;
                    m_end_time = m_life_time * m_params [ m_key_curr ].m_end_time_scaled;
                    memcpy ( &m_effector_data [ 4 ], &m_params [ m_key_curr ].m_mtx [ 4 ], sizeof ( float ) * 8 );
                }
                else {
                    // No more keyframes
                    step = 0.0f;
                }
            }
        }
    }

    // Execute next element in chain
    if ( m_next ) {
        m_next->execute ( dt );
    }
}


void cparticle_element::init_params ( sparticle_vertex_param vxp, sparticle_vector_param vcp, int unk_angle ) {
    m_current_life = 0.0f;
    m_key_curr = 0;
    m_end_time = m_life_time * m_params [ 0 ].m_end_time_scaled;

    // Initialize with first keyframe data
    memcpy ( m_effector_data, m_params [ 0 ].m_mtx, sizeof ( float ) * 12 );

    glm::vec4 rnd_vec = glm::vec4 ( frandom ( ), frandom ( ), frandom ( ), frandom ( ) );

    switch ( ( e_element_type ) m_params [ 0 ].m_type ) {
    case e_element_type::position: {
        float rand1 = frandom ( );
        float rand2 = frandom ( );
        float rand3 = frandom ( );

        float base_val = vcp.m_pos_pan * rand1 + vcp.m_base;
        float mult_val = vcp.m_multiplier * ( ( vcp.m_vertical_pan * rand2 * 2.0f ) - vcp.m_vertical_pan );
        float vertical_val = vcp.m_vertical_base * ( ( vcp.m_pos_random * rand3 * 2.0f ) - vcp.m_pos_random );

        int angle_offset = calculate_random_angle_offset ( random ( ), unk_angle );
        int angle = ( int ) vcp.m_angle_base + angle_offset;

        float sin_val, cos_val;
        sin_cos ( &sin_val, &cos_val, angle );

        m_effector_data [ 0 ] = 0.0f;
        m_effector_data [ 1 ] = 0.0f;
        m_effector_data [ 2 ] = 0.0f;
        m_effector_data [ 3 ] = 0.0f;

        m_effector_data [ 4 ] = 0.0f;
        m_effector_data [ 5 ] = sin_val * base_val;
        m_effector_data [ 6 ] = cos_val * base_val;
        m_effector_data [ 7 ] = 0.0f;

        m_effector_data [ 8 ] = 0.0f;
        m_effector_data [ 9 ] = sin_val * mult_val + vertical_val;
        m_effector_data [ 10 ] = cos_val * mult_val;
        m_effector_data [ 11 ] = 0.0f;
        break;
    }

    case e_element_type::scale: {
        if ( ( vxp.m_scale_flag & 1U ) == 0  ) {
            rnd_vec = glm::vec4 ( frandom ( ), frandom ( ), frandom ( ), frandom ( ) );
            rnd_vec *= vxp.m_scale_range;
            rnd_vec += vxp.m_scale_base;
            m_effector_data [ 0 ] += rnd_vec.w;
            m_effector_data [ 1 ] += rnd_vec.x;
            m_effector_data [ 2 ] += rnd_vec.y;
            m_effector_data [ 3 ] += rnd_vec.z;
        }
        else {
            float rnd = frandom ( );
            m_effector_data [ 0 ] += vxp.m_scale_range.w * rnd + vxp.m_scale_base.w;
        }
        break;
    }

    case e_element_type::color_8bit:
    case e_element_type::color_7bit: {
        float scale;
        if ( ( e_element_type ) m_params [ 0 ].m_type == e_element_type::color_8bit ) {
            scale = 255.0f;
        }
        else {
            scale = 127.0f;
        }

        rnd_vec *= vxp.m_color_range;
        rnd_vec += vxp.m_color_base;
        rnd_vec *= scale;

        m_effector_data [ 0 ] += rnd_vec.r;
        m_effector_data [ 1 ] += rnd_vec.g;
        m_effector_data [ 2 ] += rnd_vec.b;
        m_effector_data [ 3 ] += rnd_vec.a;
        break;
    }

    case e_element_type::angle: {
        float rnd = frandom ( );
        m_effector_data [ 0 ] += vxp.m_rotation_range.x * rnd + vxp.m_rotation_base.x;
        rnd = frandom ( );
        m_effector_data [ 1 ] += vxp.m_rotation_range.y * rnd + vxp.m_rotation_base.y;
        rnd = frandom ( );
        m_effector_data [ 2 ] += vxp.m_rotation_range.z * rnd + vxp.m_rotation_base.z;
        rnd = frandom ( );

        float new_accel_value_x = vxp.m_angular_accel_range.x * rnd + vxp.m_angular_accel_base.x;
        m_effector_data [ 4 ] += new_accel_value_x * FRAME_SPD;

        rnd = frandom ( );
        float new_accel_value_y = vxp.m_angular_accel_range.y * rnd + vxp.m_angular_accel_base.y;
        m_effector_data [ 5 ] += new_accel_value_y * FRAME_SPD;

        rnd = frandom ( );
        float new_accel_value_z = vxp.m_angular_accel_range.z * rnd + vxp.m_angular_accel_base.z;
        m_effector_data [ 6 ] += new_accel_value_z * FRAME_SPD;

        break;
    }

    case e_element_type::uv:
    case e_element_type::pattern: {
        if ( vxp.m_uv_flag == 1 ) {
            unsigned char start = vxp.m_start_frame;
            unsigned char end = vxp.m_end_frame;
            float frame_rand = frandom ( );
            frame_rand = ( float ) ( ( int ) end - ( int ) start + 1 ) * frame_rand + ( float ) start;

            int frame = ( int ) frame_rand;
            float cell_width = 1.0f / ( float ) ( vxp.m_columns + 1 );
            float cell_height = 1.0f / ( float ) ( vxp.m_rows + 1 );

            m_effector_data [ 0 ] = cell_width * ( float ) ( ( int ) ( ( vxp.m_width * ( float ) frame ) / cell_width ) );
            m_effector_data [ 1 ] = cell_height * ( float ) ( ( int ) ( ( vxp.m_height * ( float ) frame ) / cell_height ) );
        }
        else {
            float rand = frandom ( );
            m_effector_data [ 0 ] += vxp.m_uv_range [ 0 ] * rand + vxp.m_uv_base [ 0 ];
            rand = frandom ( );
            m_effector_data [ 1 ] += vxp.m_uv_range [ 1 ] * rand + vxp.m_uv_base [ 1 ];
        }
        break;
    }

    default:
        // For other element types, just use the initialized data
        break;
    }
}

void cparticle_element::create_chain ( cparticle_element* next, float life, float timeScale ) {
    cparticle_element* curr = this;

    // Find the end of the chain
    while ( curr->m_next ) {
        curr = curr->m_next;
    }

    // Append new element
    curr->m_next = next;

    // Initialize timing for the new element
    next->m_life_time = life;
    next->m_end_time = life * next->m_params [ 0 ].m_end_time_scaled;
    next->m_time_scale = timeScale;
    next->m_current_life = 0.0f;
    next->m_key_curr = 0;
    next->m_next = nullptr;
}

void cparticle_element::output ( sparticle_output* output, sparticle_vertex_param vxp ) {
    cparticle_element* curr = this;

    while ( curr ) {
        switch ( ( e_element_type ) curr->m_params [ 0 ].m_type ) {
        case e_element_type::position:
            output->m_position = glm::make_vec3 ( &curr->m_effector_data [ 0 ] );
            break;

        case e_element_type::scale:
            output->m_scale = glm::vec4 ( curr->m_effector_data [ 1 ], curr->m_effector_data [ 2 ], curr->m_effector_data [ 3 ], curr->m_effector_data [ 0 ] );
            break;

        case e_element_type::color_8bit:
        case e_element_type::color_7bit: {
            glm::vec4 color = glm::clamp ( glm::make_vec4 ( curr->m_effector_data ), 0.0f, 255.0f );
            output->m_rgba = ( ( unsigned int ) ( color.a ) << 24 ) |
                ( ( unsigned int ) ( color.r ) << 16 ) |
                ( ( unsigned int ) ( color.g ) << 8 ) |
                ( unsigned int ) ( color.b );
            break;
        }

        case e_element_type::angle:
            output->m_rotation = glm::make_vec3 ( &curr->m_effector_data [ 0 ] );
            break;

        case e_element_type::uv:
            meffector_store_tex_coord ( curr->m_effector_data, &output->m_uv [ 0 ] );
            break;

        case e_element_type::pattern:
            meffector_store_stepped_tex_coord ( curr->m_effector_data, &output->m_uv [ 0 ],
                vxp.m_columns, vxp.m_rows );
            break;

        default:
            // Other element types don't affect output
            break;
        }

        curr = curr->m_next;
    }
}

// ============================================================================
// cdraw_particle Implementation
// ============================================================================

void cdraw_particle::draw ( ) {
    if ( !m_output || m_meshes.empty ( ) )
        return;

    glm::mat4 final = m_mtx;

    // Apply particle position
    final = glm::translate ( final, m_output->m_position );

    // Apply rotations in correct order: Yaw (Y), Pitch (X), Roll (Z)
    final = glm::rotate ( final, ROT_Y_TO_FLOAT ( m_output->m_rotation.z ), glm::vec3 ( 0, 0, 1 ) );  // Roll
    final = glm::rotate ( final, ROT_Y_TO_FLOAT ( m_output->m_rotation.y ), glm::vec3 ( 0, 1, 0 ) );  // Yaw
    final = glm::rotate ( final, ROT_Y_TO_FLOAT ( m_output->m_rotation.x ), glm::vec3 ( 1, 0, 0 ) );  // Pitch

    // Apply scale
    glm::vec3 scale = glm::vec3 ( m_output->m_scale );
    final = glm::scale ( final, scale );

    if ( m_vert_type == 1 || m_vert_type == 0 ) { // Billboard

        glm::vec3 cam_right = glm::vec3 ( g_view [ 0 ][ 0 ], g_view [ 1 ][ 0 ], g_view [ 2 ][ 0 ] );
        glm::vec3 cam_up = glm::vec3 ( g_view [ 0 ][ 1 ], g_view [ 1 ][ 1 ], g_view [ 2 ][ 1 ] );
        glm::vec3 cam_forward = glm::vec3 ( g_view [ 0 ][ 2 ], g_view [ 1 ][ 2 ], g_view [ 2 ][ 2 ] );

        glm::vec3 local_pos = glm::vec3 ( final [ 3 ] );

        glm::vec3 world_pos = glm::vec3 ( glm::vec4 ( local_pos, 1.0f ) );

        glm::mat4 billboard = glm::mat4 ( 1.0f );
        billboard [ 0 ] = glm::vec4 ( cam_right * m_output->m_scale.x, 0.0f );
        billboard [ 1 ] = glm::vec4 ( cam_up * m_output->m_scale.y, 0.0f );
        billboard [ 2 ] = glm::vec4 ( -cam_forward * m_output->m_scale.z, 0.0f );
        billboard [ 3 ] = glm::vec4 ( world_pos, 1.0f );

        final = billboard;
    }

    // Draw all meshes
    for ( auto& mref : m_meshes ) {
        cmesh_buffer* m = mref.get ( );
        m->m_mdl = final;

        // Update textures with particle color and UV
        for ( auto& tref : m->m_used_tex ) {
            ctex_buffer* t = tref.get ( );

            // Extract and convert color (BGRA to RGBA)
            glm::vec4 color = glm::unpackUnorm4x8 ( m_output->m_rgba );
            color = glm::vec4 ( color.z, color.y, color.x, color.w );

            t->m_base_color = color;
            t->m_base_uv = m_output->m_uv;
        }

        m->draw ( );
    }
}