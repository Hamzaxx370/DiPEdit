// ============================================================================
// File: BinaryHelper.cpp
//
// Purpose: Implements cbinary_helper
//
// ============================================================================

#include "binary_helper.h"

cbinary_helper::cbinary_helper ( const char* filepath, bool write_mode )
{
    m_is_writer = write_mode;

    if ( m_is_writer )
    {
        // Open for writing (binary, truncates existing file)
        m_file_buffer.open ( filepath, ios::binary | ios::out | ios::trunc );
    }
    else
    {
        // Open for reading
        m_file_buffer.open ( filepath, ios::binary | ios::in );
    }
}

cbinary_helper::~cbinary_helper ( )
{
    if ( m_file_buffer.is_open ( ) )
        m_file_buffer.close ( );

    // Clear stream buffers
    m_byte_reader.str ( "" );
    m_byte_reader.clear ( );
    m_byte_writer.str ( "" );
    m_byte_writer.clear ( );
}

void cbinary_helper::set_buffer_mode ( char* Buffer, int Size )
{
    if ( m_is_writer )
    {
        // If writing, we reset the output string stream
        m_byte_writer.str ( "" );
        m_byte_writer.clear ( );
    }
    else
    {
        // If reading, we populate the input stream
        m_byte_reader = std::istringstream ( std::move ( std::string ( Buffer, Size ) ), std::ios::in | std::ios::binary );
    }
    m_is_byte_buffer = true;
}

void cbinary_helper::end_buffer_mode ( )
{
    m_is_byte_buffer = false; // Fixed: Original code set this to true
}

string cbinary_helper::get_written_buffer_data ( )
{
    return m_byte_writer.str ( );
}

// ==========================================================
// Reading Implementation
// ==========================================================

string cbinary_helper::read_string ( )
{
    string result;
    char ch;
    // Helper lambda to get next char based on mode
    auto get_next = [ & ] ( ) -> bool {
        if ( !m_is_byte_buffer ) return ( bool ) m_file_buffer.read ( &ch, 1 );
        else return ( bool ) m_byte_reader.read ( &ch, 1 );
        };

    while ( get_next ( ) )
    {
        if ( ch != '\0' ) result += ch;
        else break;
    }
    return result;
}

string cbinary_helper::read_fixed_string ( const int size ) {
    string magic ( size, '\0' );
    // Warning: Original code used stack buffer char temp[100]. 
    // If size > 100 this crashes. Safe dynamic approach below:

    vector<char> temp ( size );

    if ( !m_is_byte_buffer ) m_file_buffer.read ( temp.data ( ), size );
    else m_byte_reader.read ( temp.data ( ), size );

    for ( int i = 0; i < size; i++ ) {
        magic [ i ] = temp [ i ];
    };

    magic.erase ( std::remove ( magic.begin ( ), magic.end ( ), '\0' ), magic.end ( ) );
    return magic;
};

unsigned int cbinary_helper::read_uint ( ) {
    unsigned int integer;
    if ( !m_is_byte_buffer ) m_file_buffer.read ( reinterpret_cast< char* > ( &integer ), sizeof ( int ) );
    else m_byte_reader.read ( reinterpret_cast< char* >( &integer ), sizeof ( int ) );
    return integer;
};

int cbinary_helper::read_int ( ) {
    int integer;
    if ( !m_is_byte_buffer ) m_file_buffer.read ( reinterpret_cast< char* >( &integer ), sizeof ( int ) );
    else m_byte_reader.read ( reinterpret_cast< char* >( &integer ), sizeof ( int ) );
    return integer;
};

unsigned char cbinary_helper::read_uchar ( ) {
    unsigned char byte;
    if ( !m_is_byte_buffer ) m_file_buffer.read ( reinterpret_cast< char* >( &byte ), sizeof ( unsigned char ) );
    else m_byte_reader.read ( reinterpret_cast< char* >( &byte ), sizeof ( unsigned char ) );
    return byte;
};

char cbinary_helper::read_char ( ) {
    char byte;
    if ( !m_is_byte_buffer ) m_file_buffer.read ( &byte, sizeof ( char ) );
    else m_byte_reader.read ( &byte, sizeof ( char ) );
    return byte;
};

short cbinary_helper::read_short ( ) {
    short theshort;
    if ( !m_is_byte_buffer ) m_file_buffer.read ( reinterpret_cast< char* >( &theshort ), sizeof ( short ) );
    else m_byte_reader.read ( reinterpret_cast< char* >( &theshort ), sizeof ( short ) );
    return theshort;
};

unsigned short cbinary_helper::read_ushort ( ) {
    unsigned short theshort;
    if ( !m_is_byte_buffer ) m_file_buffer.read ( reinterpret_cast< char* >( &theshort ), sizeof ( unsigned short ) );
    else m_byte_reader.read ( reinterpret_cast< char* >( &theshort ), sizeof ( unsigned short ) );
    return theshort;
};

float cbinary_helper::read_float ( )
{
    float thefloat;
    if ( !m_is_byte_buffer ) m_file_buffer.read ( reinterpret_cast< char* >( &thefloat ), sizeof ( float ) );
    else m_byte_reader.read ( reinterpret_cast< char* >( &thefloat ), sizeof ( float ) );
    return thefloat;
}

void cbinary_helper::read_chars ( char*& buffer, int size )
{
    if ( !m_is_byte_buffer ) m_file_buffer.read ( buffer, size );
    else m_byte_reader.read ( buffer, size );
}

void cbinary_helper::read_uchars ( unsigned char*& buffer, int size )
{
    if ( !m_is_byte_buffer ) m_file_buffer.read ( reinterpret_cast< char* >( buffer ), size );
    else m_byte_reader.read ( reinterpret_cast< char* >( buffer ), size );
}

void cbinary_helper::read_chars_fixed ( char* buffer, int size )
{
    if ( !m_is_byte_buffer ) m_file_buffer.read ( buffer, size );
    else m_byte_reader.read ( buffer, size );
}

void cbinary_helper::read_uchars_fixed ( unsigned char* buffer, int size )
{
    if ( !m_is_byte_buffer ) m_file_buffer.read ( reinterpret_cast< char* >( buffer ), size );
    else m_byte_reader.read ( reinterpret_cast< char* >( buffer ), size );
}

// ==========================================================
// Writing Implementation
// ==========================================================

void cbinary_helper::write_uchar ( unsigned char val ) {
    if ( !m_is_byte_buffer ) m_file_buffer.write ( reinterpret_cast< const char* >( &val ), sizeof ( unsigned char ) );
    else m_byte_writer.write ( reinterpret_cast< const char* >( &val ), sizeof ( unsigned char ) );
}

void cbinary_helper::write_char ( char val ) {
    if ( !m_is_byte_buffer ) m_file_buffer.write ( &val, sizeof ( char ) );
    else m_byte_writer.write ( &val, sizeof ( char ) );
}

void cbinary_helper::write_short ( short val ) {
    if ( !m_is_byte_buffer ) m_file_buffer.write ( reinterpret_cast< const char* >( &val ), sizeof ( short ) );
    else m_byte_writer.write ( reinterpret_cast< const char* >( &val ), sizeof ( short ) );
}

void cbinary_helper::write_ushort ( unsigned short val ) {
    if ( !m_is_byte_buffer ) m_file_buffer.write ( reinterpret_cast< const char* >( &val ), sizeof ( unsigned short ) );
    else m_byte_writer.write ( reinterpret_cast< const char* >( &val ), sizeof ( unsigned short ) );
}

void cbinary_helper::write_uint ( unsigned int val ) {
    if ( !m_is_byte_buffer ) m_file_buffer.write ( reinterpret_cast< const char* >( &val ), sizeof ( unsigned int ) );
    else m_byte_writer.write ( reinterpret_cast< const char* >( &val ), sizeof ( unsigned int ) );
}

void cbinary_helper::write_int ( int val ) {
    if ( !m_is_byte_buffer ) m_file_buffer.write ( reinterpret_cast< const char* >( &val ), sizeof ( int ) );
    else m_byte_writer.write ( reinterpret_cast< const char* >( &val ), sizeof ( int ) );
}

void cbinary_helper::write_float ( float val ) {
    if ( !m_is_byte_buffer ) m_file_buffer.write ( reinterpret_cast< const char* >( &val ), sizeof ( float ) );
    else m_byte_writer.write ( reinterpret_cast< const char* >( &val ), sizeof ( float ) );
}

void cbinary_helper::write_chars ( const char* buffer, const int size ) {
    if ( !m_is_byte_buffer ) m_file_buffer.write ( buffer, size );
    else m_byte_writer.write ( buffer, size );
}

void cbinary_helper::write_uchars ( const unsigned char* buffer, const int size ) {
    if ( !m_is_byte_buffer ) m_file_buffer.write ( reinterpret_cast< const char* >( buffer ), size );
    else m_byte_writer.write ( reinterpret_cast< const char* >( buffer ), size );
}

void cbinary_helper::write_string ( const string& str ) {
    // Write characters
    if ( !m_is_byte_buffer ) m_file_buffer.write ( str.c_str ( ), str.length ( ) );
    else m_byte_writer.write ( str.c_str ( ), str.length ( ) );

    // Write Null terminator
    write_char ( '\0' );
}

void cbinary_helper::write_fixed_string ( const string& str, const int size ) {
    int len = ( int ) str.length ( );
    // Truncate if string is longer than fixed size
    if ( len > size ) len = size;

    // Write content
    write_chars ( str.c_str ( ), len );

    // Pad the rest with 0
    if ( len < size ) {
        write_padding ( size - len );
    }
}

void cbinary_helper::write_padding ( const int size ) {
    if ( size <= 0 ) return;

    // Create a buffer of zeros
    vector<char> zeros ( size, 0 );

    if ( !m_is_byte_buffer ) m_file_buffer.write ( zeros.data ( ), size );
    else m_byte_writer.write ( zeros.data ( ), size );
}

// ==========================================================
// Utilities
// ==========================================================

int cbinary_helper::get_pos ( )
{
    if ( !m_is_byte_buffer )
    {
        if ( m_is_writer ) return ( int ) ( m_file_buffer.tellp ( ) - streampos ( m_begin ) );
        else return ( int ) ( m_file_buffer.tellg ( ) - streampos ( m_begin ) );
    }
    else
    {
        if ( m_is_writer ) return ( int ) ( m_byte_writer.tellp ( ) - streampos ( m_buffer_begin ) );
        else return ( int ) ( m_byte_reader.tellg ( ) - streampos ( m_buffer_begin ) );
    }
}

void cbinary_helper::seek ( int offset )
{
    if ( !m_is_byte_buffer )
    {
        if ( m_is_writer ) m_file_buffer.seekp ( std::streamoff ( offset + m_begin ) );
        else m_file_buffer.seekg ( std::streamoff ( offset + m_begin ) );
    }
    else
    {
        if ( m_is_writer ) m_byte_writer.seekp ( std::streamoff ( offset + m_buffer_begin ) );
        else m_byte_reader.seekg ( std::streamoff ( offset + m_buffer_begin ) );
    }
}

void cbinary_helper::set_begin ( int begin )
{
    if ( !m_is_byte_buffer )
    {
        m_begin = begin;
    }
    else
    {
        m_buffer_begin = begin;
    }
}
