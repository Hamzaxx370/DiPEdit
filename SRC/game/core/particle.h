#pragma once

#include "glm\glm\gtc\matrix_transform.hpp"
#include "glm\glm\gtc\type_ptr.hpp"
#include "glm\glm\gtx\norm.hpp"

#include <iostream>
#include <vector>

#include "engine\sys\render\render.h"

#define ROT_Y_TO_FLOAT( x ) ( ( static_cast< float >( x ) / 65536.0f ) * ( 2.0f * glm::pi<float>() ) )
#define ROT_Y_TO_FLOAT_ALT( x ) ( ( ( x ) / 65536.0f ) * ( 2.0f * glm::pi<float>() ) )

inline float angle_to_radians ( unsigned int angle_int ) {
    // 360 degrees = 2 * PI radians
    // angle_int / 2^16 * 360 = angle_rad / (2 * PI) * 360
    // angle_rad = angle_int / 2^16 * 2 * PI
    return static_cast< float >( angle_int ) * ( 2.0f * glm::pi<float> ( ) ) / 65536.0f; // 2^16
}

class cdraw_particle : public cdraw_base {
public:
    std::vector < cmesh_ref > m_meshes;
    struct sparticle_output* m_output;
    glm::mat4 m_mtx;

    int m_vert_type;

    cdraw_particle ( ) : cdraw_base ( ) { };
    ~cdraw_particle ( ) { };

    /**
    * @brief The drawing function for a basic drawable that
    * draws its registered meshes
    */
    void draw ( ) override;
};

enum class e_element_type {
    position,
    scale,
    color_8bit,
    angle,
    uv,
    pattern,
    color_7bit,
    null,
    unk
};

struct sparticle_element_param {
    int m_type;
    int m_effect_type;
    int m_format_flag;
    float m_end_time_scaled;
    float m_mtx [ 12 ];
};


struct sparticle_vector_param {
    float m_base;
    float m_vertical_base;
    float m_multiplier;
    int m_angle_base;
    float m_vertical_pan;
    float m_pos_pan;
    float m_pos_random;
    int m_unused;
};

struct sparticle_vertex_param {
    glm::vec4 m_color_range;
    glm::vec4 m_color_base;

    int m_scale_flag;
    glm::vec4 m_scale_range;
    glm::vec4 m_scale_base;

    glm::vec3 m_rotation_range;
    glm::vec3 m_rotation_base;
    glm::vec3 m_angular_accel_range;
    glm::vec3 m_angular_accel_base;

    int m_uv_flag;
    glm::vec2 m_uv_range;
    glm::vec2 m_uv_base;
    int m_columns;
    int m_rows;
    int m_start_frame;
    int m_end_frame;
    float m_width;
    float m_height;
    int m_material_flag = 0;
};

struct sparticle_emitter_param {
    int m_element_count;
    int m_delay_min;
    int m_delay_max;
    int m_emit_min;
    int m_emit_max;
    int m_unknown3;
    float m_life_time;
    float m_inverse_speed;
    int m_unknown4;
    int m_pool_size;
    int m_vertex_type;
    int m_render_state;
    int m_model_id;
    int m_texture_id;
    int m_element_pointer;
    std::vector<std::vector<sparticle_element_param>> m_element_chains;
    float m_time_scale;
    sparticle_vector_param m_vector_param;
    sparticle_vertex_param m_vertex_param;
    float m_frame_rate;
};

struct sparticle_param {
    int m_is_enabled;
    int m_cycle_min;
    int m_cycle_max;
    int m_emit_shape;
    int m_use_surface_normals;
    int m_transform_flag;
    float m_radius_min;
    float m_radius_max;
    int m_angle;
    int m_rot_y;
    int m_rot_z;
    float m_scale_y_min;
    float m_scale_y_max;
    float m_scale_xz_min;
    float m_scale_xz_max;
    int m_dir_angle_min;
    int m_dir_angle_max;
    sparticle_emitter_param m_emitter_param;
    int m_special_orient_flag;
};

struct sparticle {
    std::string m_name;
    int m_id;
    int m_unk;

    std::vector<sparticle_param> m_particle_params;
};

struct sparticle_random_param {
	glm::vec3 m_pos;
	glm::vec3 m_dir;
    float roty;
    float rotz;
	glm::vec3 m_scale;
};

struct sparticle_data {
    glm::vec3 m_pos;
    glm::vec3 m_normal;
    glm::vec3 m_tmp0;
};

struct sparticle_output {
    glm::vec3 m_position;
    glm::vec3 m_rotation;
    glm::vec4 m_scale;
    unsigned int m_rgba;
    glm::vec2 m_uv;
    float m_life;
};

class cparticle_element {
public:
    std::vector<sparticle_element_param> m_params;

    float m_life_time;
    float m_current_life;
    float m_end_time;
    float m_time_scale;

    float m_effector_data [ 12 ];
    unsigned char m_key_max;
    unsigned char m_key_curr;
    int field15_0x30;

    cparticle_element* m_next;

    void initialize ( std::vector<sparticle_element_param> params );
    void execute ( float dt );

    void init_params ( sparticle_vertex_param vxp, sparticle_vector_param vcp, int unk_angle );
    void create_chain ( cparticle_element* next, float life, float scale );

    void output ( sparticle_output* output, sparticle_vertex_param vxp );

    bool is_finished ( ) { return ( m_current_life > m_life_time ); };
};

class cparticle_emitter {
public:
    // File data
    sparticle_emitter_param m_param;
    sparticle_param m_particle_param;
    std::vector<std::vector<sparticle_element_param>> m_element_chain_param;

    // Runtime data
    unsigned char m_pool_size;
    std::vector<sparticle_output> m_output;
    std::vector<sparticle_random_param> m_random_param;

    glm::mat4 m_attach;
    glm::vec3 m_pos;
    glm::vec3 m_normal;
    glm::vec3 m_tmp0;

    glm::vec3 m_vertex;

    int m_rotate_x;

    float m_gen_num;
    float m_gen_rate;

    std::vector<cparticle_element*> m_output_chains;

    float m_emission_duration;
    float m_delay_duration;
    float m_emission_num;
    float m_delay_num;

    bool m_finished;

    std::vector<glm::mat4> m_matrix;

    std::vector<cdraw_particle> m_drawables;

    bool initialize ( sparticle_param particle_param, sparticle_emitter_param emitter_param, glm::vec3 vertex );
    void release ( );

    void execute ( );
    void draw ( std::vector<cmesh_ref> meshes );

    glm::mat4 create_matrix ( sparticle_random_param& random_param );
    glm::vec3 calc_pos ( );
    glm::mat4 set_matrix (
        sparticle_param particle_param,
        glm::vec3 pos,
        glm::vec3 normal,
        glm::vec3 tmp0,
        sparticle_random_param random_param
    );
};

class cparticle {
public:
    cparticle_emitter* m_emitter;
    sparticle_param m_param;

    int m_enabled;
    bool m_finished;

    int m_gen_num;

    std::vector<cmesh_ref> m_meshes;

    bool initialize ( sparticle_param param, sparticle_data data, glm::vec3 vertex );
    void release ( );

    void execute ( );
    void draw ( );

    void set_attach_mtx ( glm::mat4 attach ) { 
        if ( !m_emitter ) {
            return;
        }

        for ( int i = 0; i < m_gen_num; i++ ) {
            m_emitter [ i ].m_attach = attach;
        }
    };
    void set_particle_data ( sparticle_data data ) {
        if ( !m_emitter ) {
            return;
        }

        for ( int i = 0; i < m_gen_num; i++ ) {
            m_emitter [ i ].m_pos = data.m_pos;
            m_emitter [ i ].m_normal = data.m_normal;
            m_emitter [ i ].m_tmp0 = data.m_tmp0;
        }
    }
};


struct sptcl_info {
    std::string name;
    int id;
};

//Data stuff
extern sptcl_info g_ptcl_info [ 697 ];

static std::string get_ptcl_from_id ( int id ) {
    for ( int i = 0; i < 697; i++ ) {
        if ( g_ptcl_info [ i ].id == id )
            return g_ptcl_info [ i ].name;
    }
    return "";
}