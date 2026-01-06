// ============================================================================
// File: components.cpp
// 
// Purpose: Implements components
// 
// ============================================================================

#include "Components.h"

#include "engine\core\engine.h"

#include "engine\sys\collision\collision.h"
#include "engine\sys\render\skel.h"

ccomponent_anim::ccomponent_anim ( ) {
    m_anim = nullptr;
    m_looped = false;
    m_frame_counter = 0.0f;
}

ccomponent_anim::~ccomponent_anim ( ) {
    if ( m_anim )
        delete m_anim;
    m_anim = nullptr;
}

void ccomponent_anim::exec ( ) {
    if ( !m_anim ) return;

    cskel_animator::skel_update_animation ( m_bones, m_anim, m_frame_counter );

    m_frame_counter += cengine::get ( )->render_man->get_delta ( ) / ( 1.0f / m_anim->m_frame_rate );
}

ccomponent_physics::ccomponent_physics ( ) {
    m_max_speed = 10.0f;
    m_accel = 20.0f;
    m_deccel = 10.0f;
    m_gravity = 30.0f;

    m_capsule = new ccol_capsule ( );
    m_capsule->m_height = 3.5f;
    m_capsule->m_radius = 1.0f;

    m_capsule->m_pos.y = 1.0f;

    m_rot = glm::quat ( 1.0f, 0.0f, 0.0f, 0.0f );

    m_grounded = true;
};

ccomponent_physics::~ccomponent_physics ( ) {
    delete m_capsule;
}

void ccomponent_physics::exec ( ) {
    float delta = cengine::get ( )->render_man->get_delta ( );

    glm::vec3 desired_move = m_capsule->m_vel * delta;
    glm::vec3 original_pos = m_capsule->m_pos;

    m_capsule->m_pos += desired_move;

    for ( int i = 0; i < 3; i++ ) {
        std::vector<ccol_info> cols;

        cengine::get ( )->col_man->col_check ( *m_capsule, cols );

        if ( cols.empty ( ) ) break;

        for ( auto& col : cols ) {
            if ( !col.m_collided ) continue;

            glm::vec3 push = col.m_normal * ( col.m_depth );

            m_capsule->m_pos += push;

            float vel_dot = glm::dot ( m_capsule->m_vel, col.m_normal );
            if ( vel_dot < 0.0f ) {
                m_capsule->m_vel -= col.m_normal * vel_dot;
            }
        }
    }

    m_grounded = false;

    if ( m_capsule->m_vel.y > -0.1f && m_capsule->m_vel.y < 0.1f ) {
    }

    ccol_info ground_check;
    if ( cengine::get ( )->col_man->ray_cast ( glm::vec3 ( m_capsule->m_pos.x, m_capsule->m_pos.y + 0.05, m_capsule->m_pos.z ), glm::vec3 ( 0, -1, 0 ), 1.1f + m_capsule->m_radius + 0.2f, &ground_check ) ) {
        if ( ground_check.m_normal.y > 0.7f ) {
            float dist = ground_check.m_depth;
            m_grounded = true;
        }
    }

    m_capsule->m_vel.y -= m_gravity * delta;
};