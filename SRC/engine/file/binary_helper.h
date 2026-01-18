// ============================================================================
// File: BinaryHelper.h
//
// Purpose: Defines a custom binary helper class (Reader and Writer)
//
// ============================================================================

#ifndef BINARY_HELPER_H
#define BINARY_HELPER_H

#pragma once
#include "defs.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
using namespace std;

class cbinary_helper {
public:
    // Changed from ifstream to fstream to support both read/write
    fstream m_file_buffer;

    // Memory streams
    istringstream m_byte_reader;
    ostringstream m_byte_writer;

    bool m_is_byte_buffer = false;
    bool m_is_writer = false; // Tracks if we are in write mode

    int m_begin = 0;
    int m_buffer_begin = 0;

    // Constructor now accepts a write mode (default is false/read-only)
    cbinary_helper ( const char* filepath, bool write_mode = false );
    cbinary_helper ( ) { };
    ~cbinary_helper ( );

    void set_buffer_mode ( char* Buffer, int Size );
    void end_buffer_mode ( );

    // For retrieving data when writing to memory buffer
    string get_written_buffer_data ( );

    // ==========================================================
    // Reading funcs
    // ==========================================================
    unsigned char read_uchar ( );
    char read_char ( );
    short read_short ( );
    unsigned short read_ushort ( );
    unsigned int read_uint ( );
    int read_int ( );
    float read_float ( );
    void read_chars ( char*& buffer, const int size );
    void read_uchars ( unsigned char*& buffer, const int size );
    void read_chars_fixed ( char* buffer, const int size );
    void read_uchars_fixed ( unsigned char* buffer, const int size );
    string read_string ( );
    string read_fixed_string ( const int size );

    // ==========================================================
    // Writing funcs
    // ==========================================================
    void write_uchar ( unsigned char val );
    void write_char ( char val );
    void write_short ( short val );
    void write_ushort ( unsigned short val );
    void write_uint ( unsigned int val );
    void write_int ( int val );
    void write_float ( float val );
    void write_chars ( const char* buffer, const int size );
    void write_uchars ( const unsigned char* buffer, const int size );
    void write_string ( const string& str ); // Writes null-terminated string
    void write_fixed_string ( const string& str, const int size ); // Writes string + padding
    void write_padding ( const int size ); // Utility to write 0x00 bytes
    void align ( int alignment ) {
        int pos = get_pos ( );
		int padding = ( alignment - ( pos % alignment ) ) % alignment;
		write_padding ( padding );
    };

    // ==========================================================
    // Utilities
    // ==========================================================
    int get_pos ( );
    void seek ( int offset );
    void set_begin ( int begin );
};

#endif // !BINARY_HELPER_H