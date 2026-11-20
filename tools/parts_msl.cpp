// Prints the MSL generated from a shader parts file (workingdir/shaders/parts), without a window or device: CI compiles
// it with the offline Metal compiler, which runs on GPU-less macOS runners.
// Usage: parts_msl <parts dir> <name> <vs|ps> ["#define ..." variant line]
// "//@include <file>" lines are expanded and the variant line goes before the file's defines, as
// CreateInternalShaderFromParts (dear_widgets.cpp) does.
#include "ImPlatform.h"
#include <stdio.h>
#include <string>

static bool ReadText( const std::string& path, std::string& out )
{
	FILE* f = fopen( path.c_str(), "rb" );
	if ( !f )
	{
		fprintf( stderr, "cannot open %s\n", path.c_str() );
		return false;
	}
	char buffer[ 4096 ];
	for ( size_t n; ( n = fread( buffer, 1, sizeof( buffer ), f ) ) > 0; )
		out.append( buffer, n );
	fclose( f );
	return true;
}

int main( int argc, char** argv )
{
	if ( argc < 4 )
	{
		fprintf( stderr, "usage: parts_msl <parts dir> <name> <vs|ps> [\"#define ...\"]\n" );
		return 2;
	}
	const std::string dir = argv[ 1 ];
	std::string raw, text;
	if ( !ReadText( dir + "/" + argv[ 2 ] + ".hlsl", raw ) )
		return 1;
	for ( size_t pos = 0; pos < raw.size(); )
	{
		size_t end = raw.find( '\n', pos );
		end = end == std::string::npos ? raw.size() : end + 1;
		const std::string line = raw.substr( pos, end - pos );
		if ( line.compare( 0, 11, "//@include " ) == 0 )
		{
			std::string inc;
			if ( !ReadText( dir + "/" + line.substr( 11, line.find_first_of( " \t\r\n", 11 ) - 11 ), inc ) )
				return 1;
			text += inc + "\n";
		}
		else
			text += line;
		pos = end;
	}
	ImPlatform_ShaderParts parts = {};
	if ( !ImPlatform_ParseShaderParts( &text[ 0 ], &parts ) )
		return 1;
	std::string defines;
	if ( argc > 4 )
	{
		defines = std::string( argv[ 4 ] ) + "\n" + ( parts.defines ? parts.defines : "" );
		parts.defines = defines.c_str();
	}
	const ImPlatform_ShaderStage stage = std::string( argv[ 3 ] ) == "vs" ? ImPlatform_ShaderStage_Vertex : ImPlatform_ShaderStage_Fragment;
	const int len = ImPlatform_GenerateShaderSource( &parts, stage, ImPlatform_ShaderFormat_MSL, NULL, 0 );
	if ( len < 0 )
		return 1;
	std::string source( (size_t)len + 1, '\0' );
	ImPlatform_GenerateShaderSource( &parts, stage, ImPlatform_ShaderFormat_MSL, &source[ 0 ], len + 1 );
	fwrite( source.c_str(), 1, (size_t)len, stdout );
	return 0;
}
