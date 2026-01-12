#pragma once

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
