// ============================================================================
// File: skel.cpp
// 
// Purpose: Implements the skeleton system
// 
// ============================================================================

#include "skel.h"

#include "engine\core\engine.h"

cskel_bone::cskel_bone ( ) {
    m_index = -1;
    m_parent = -1;
    m_sibling = -1;
    m_child = -1;
    m_rest_pos = glm::vec3 ( 0.0 );
    m_anim_pos = glm::vec3 ( 0.0 );
    m_rest_rot = glm::quat ( 1.0, 0.0, 0.0, 0.0 );
    m_anim_rot = glm::quat ( 1.0, 0.0, 0.0, 0.0 );
    m_rest = glm::mat4 ( 1.0 );
    m_final = glm::mat4 ( 1.0 );
}

cskel_bone::~cskel_bone ( ) {
}

cskel_pos_key::cskel_pos_key ( ) {
    m_pos = glm::vec3 ( 0.0f );
}

cskel_rot_key::cskel_rot_key ( ) {
    m_rot = glm::quat ( 1.0f, 0.0f, 0.0f, 0.0f );
}

cskel_anim::cskel_anim ( ) {
    m_frame_num = 0;
    m_frame_rate = 30.0f;
}

// Find the index of the keyframe strictly before the current animation time
static int _get_pos_key_index ( const std::vector<cskel_pos_key>& keys, float anim_time ) {
    for ( size_t i = 0; i < keys.size ( ) - 1; i++ ) {
        if ( anim_time < keys [ i + 1 ].m_time ) {
            return ( int ) i;
        }
    }
    return -1; // Should not happen if time is clamped correctly
}

static int _get_rot_key_index ( const std::vector<cskel_rot_key>& keys, float anim_time ) {
    for ( size_t i = 0; i < keys.size ( ) - 1; i++ ) {
        if ( anim_time < keys [ i + 1 ].m_time ) {
            return ( int ) i;
        }
    }
    return -1;
}

// Calculate interpolated position
static glm::vec3 _calc_interpolated_pos ( const std::vector<cskel_pos_key>& keys, float anim_time ) {
    if ( keys.empty ( ) ) return glm::vec3 ( 0.0f );
    if ( keys.size ( ) == 1 ) return keys [ 0 ].m_pos;

    int idx = _get_pos_key_index ( keys, anim_time );

    // Safety check
    if ( idx == -1 || idx >= keys.size ( ) - 1 ) return keys.back ( ).m_pos;

    const cskel_pos_key& curr_key = keys [ idx ];
    const cskel_pos_key& next_key = keys [ idx + 1 ];

    // Calculate normalized time factor (0.0 to 1.0) between the two keys
    float delta_time = next_key.m_time - curr_key.m_time;
    float factor = ( anim_time - curr_key.m_time ) / delta_time;

    return glm::mix ( curr_key.m_pos, next_key.m_pos, factor );
}

// Calculate interpolated rotation (SLERP)
static glm::quat _calc_interpolated_rot ( const std::vector<cskel_rot_key>& keys, float anim_time ) {
    if ( keys.empty ( ) ) return glm::quat ( 1.0f, 0.0f, 0.0f, 0.0f );
    if ( keys.size ( ) == 1 ) return keys [ 0 ].m_rot;

    int idx = _get_rot_key_index ( keys, anim_time );

    // Safety check
    if ( idx == -1 || idx >= keys.size ( ) - 1 ) return keys.back ( ).m_rot;

    const cskel_rot_key& curr_key = keys [ idx ];
    const cskel_rot_key& next_key = keys [ idx + 1 ];

    float delta_time = next_key.m_time - curr_key.m_time;
    float factor = ( anim_time - curr_key.m_time ) / delta_time;

    return glm::slerp ( curr_key.m_rot, next_key.m_rot, factor );
}

// ============================================================================
// Public Animation Functions
// ============================================================================

void cskel_animator::skel_update_hierarchy ( std::vector<cskel_bone>& bones, const cskel_anim* p_anim, float time, int bone_idx, const glm::vec3& local_pos, const glm::quat& local_rot ) {
    if ( bone_idx < 0 || bone_idx >= bones.size ( ) ) return;

    cskel_bone& bone = bones [ bone_idx ];

    glm::quat parent_rotation = glm::quat ( 1.0, 0.0, 0.0, 0.0 );
    glm::vec3 parent_position = glm::vec3 ( 0.0 );

    if ( bone.m_parent != -1 )
    {
        cskel_bone& parent_bone = bones [ bone.m_parent ];
        parent_rotation = parent_bone.m_anim_rot;
        parent_position = parent_bone.m_anim_pos;
    }

    bone.m_anim_rot = parent_rotation * bone.m_rest_rot * local_rot;
    bone.m_anim_rot = glm::normalize ( bone.m_anim_rot );

    glm::vec3 parent_rest_position = ( bone.m_parent != -1 ) ? bones [ bone.m_parent ].m_rest_pos : glm::vec3 ( 0.0 );
    glm::vec3 rest_offset_from_parent = bone.m_rest_pos - parent_rest_position;
    glm::vec3 rotated_rest_offset = parent_rotation * rest_offset_from_parent;

    bone.m_anim_pos = parent_position + local_pos + rotated_rest_offset;

    if ( bone.m_parent == -1 ) {
        bone.m_anim_pos.y -= bone.m_rest_pos.y;
    }

    // Build the final transformation matrix
    glm::mat4 translation = glm::translate ( glm::mat4 ( 1.0 ), bone.m_anim_pos );
    glm::mat4 rotation = glm::mat4_cast ( bone.m_anim_rot );

    glm::mat4 global_transform = translation * rotation;

    bone.m_final = global_transform * glm::inverse ( bone.m_rest );

    // 4. Update Children
    int child_idx = bone.m_child;
    while ( child_idx != -1 ) {
        const cskel_bone_anim& child_bone_anim = p_anim->m_bone_anims [ child_idx ];
        glm::vec3 child_local_pos = _calc_interpolated_pos ( child_bone_anim.m_pos_keys, time );
        glm::quat child_local_rot = _calc_interpolated_rot ( child_bone_anim.m_rot_keys, time );
        skel_update_hierarchy ( bones, p_anim, time, child_idx, child_local_pos, child_local_rot );
        child_idx = bones [ child_idx ].m_sibling;
    }
}

void cskel_animator::skel_update_animation ( std::vector<cskel_bone>& bones, const cskel_anim* p_anim, float time ) {
    if ( !p_anim ) return;
    if ( bones.empty ( ) ) return;

    // 2. Update bone transforms by traversing the hierarchy
    for ( size_t i = 0; i < bones.size ( ); ++i ) {
        if ( bones [ i ].m_parent == -1 ) {
            const cskel_bone_anim& bone_anim = p_anim->m_bone_anims [ i ];
            glm::vec3 local_pos = _calc_interpolated_pos ( bone_anim.m_pos_keys, time );
            glm::quat local_rot = _calc_interpolated_rot ( bone_anim.m_rot_keys, time );
            skel_update_hierarchy ( bones, p_anim, time, ( int ) i, local_pos, local_rot );
        }
    }
}
