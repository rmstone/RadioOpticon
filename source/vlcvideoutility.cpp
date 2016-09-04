#include "vlcvideoutility.h"

#include <vlc/vlc.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <time.h>

#define MAX(A, B) (A > B) ? A : B

std::string singleDot( "." );
std::string doubleDot( ".." );
std::string dirSep( "/" );

int getDir( std::string dir, std::vector<std::string> &files )
{
	DIR *dp;
	struct dirent *dirp;
	if( ( dp = opendir( dir.c_str() ) ) == NULL )
	{
		std::cout << "Error(" << errno << ") opening " << dir << std::endl;
		return errno;
	}

	while( ( dirp = readdir( dp ) ) != NULL )
	{
		std::string filename( dirp->d_name );
		if( filename.compare( singleDot ) != 0 && filename.compare( doubleDot ) != 0 != 0 )
			files.push_back( dir + dirSep + filename );
	}
	closedir( dp );
	return 0;
}


VLCVideoUtility::VLCVideoUtility()
{
	_disp = XOpenDisplay( NULL );
	int screen = DefaultScreen( _disp );
	_win = XCreateSimpleWindow( _disp, RootWindow( _disp, screen ), 0, 0, 100, 100, 0,
								BlackPixel( _disp, screen ), BlackPixel( _disp, screen ) );

	XGetWindowAttributes( _disp, DefaultRootWindow( _disp ), &_xwa );
	XMoveResizeWindow( _disp, _win, 0, 0, _xwa.width, _xwa.height );
	XMapWindow( _disp, _win );
	XFlush( _disp );

	XSelectInput( _disp, _win, KeyPressMask | ButtonPressMask );

	std::vector<const char*> vecVLCArgs;
	vecVLCArgs.push_back( "--no-osd" );
	vecVLCArgs.push_back( "--fullscreen" );
	m_pVLCInstance = libvlc_new( vecVLCArgs.size(), &vecVLCArgs[0] );
	char cCurrentPath[FILENAME_MAX];
	getcwd( cCurrentPath, sizeof( cCurrentPath ) );
	std::string activeDir = std::string( cCurrentPath ) + "/Active";
	std::string idleDir = std::string( cCurrentPath ) + "/Idle";
	getDir( activeDir, _activeFiles );
	getDir( idleDir, _idleFiles );
	srand( time( NULL ) );
}

VLCVideoUtility::~VLCVideoUtility()
{
	UnloadFile();
	libvlc_media_player_release( m_pVLCPlayer );
	libvlc_release( m_pVLCInstance );
	XCloseDisplay( _disp );
}

void VLCVideoUtility::UnloadFile()
{
	if( m_pVLCPlayer )
		libvlc_media_player_stop( m_pVLCPlayer );
}

bool VLCVideoUtility::LoadFile( const std::string &strFile )
{
	_loading = true;
	if( m_pVLCPlayer )
		UnloadFile();

	libvlc_media_t *pMedia = libvlc_media_new_path( m_pVLCInstance, strFile.c_str() );
	if( m_pVLCPlayer )
		libvlc_media_player_set_media( m_pVLCPlayer, pMedia );
	else
	{
		m_pVLCPlayer = libvlc_media_player_new_from_media( pMedia );
		libvlc_media_player_set_xwindow( m_pVLCPlayer, _win );
		XWarpPointer( _disp, _win, _win, 0, 0, _xwa.width, _xwa.height, _xwa.width - 1, _xwa.height - 1 );
	}

	libvlc_media_release( pMedia );

	return true;
}

bool VLCVideoUtility::IsPlaying()
{
	bool ret = false;

	if( m_pVLCPlayer )
	{
		ret = libvlc_media_player_is_playing( m_pVLCPlayer );
		if( ret )
			_loading = false;
	}

	return ret || _loading;
}

void VLCVideoUtility::PlayActive()
{
	int index = rand() % _activeFiles.size();
	printf( "Playing Active video index %d of %d\n%s\n", index, _activeFiles.size(), _activeFiles.at( index ).c_str() );
	LoadFile( _activeFiles.at( index ) );
	Play();
}

void VLCVideoUtility::PlayIdle()
{
	int index = rand() % _idleFiles.size();
	printf( "Playing Idle video index %d of %d\n%s\n", index, _idleFiles.size(), _idleFiles.at( index ).c_str() );
	LoadFile( _idleFiles.at( index ) );
	Play();
}

void VLCVideoUtility::Play()
{
	XWarpPointer( _disp, _win, _win, 0, 0, 0, 0, _xwa.width - 1, _xwa.height - 1 );
	if( m_pVLCPlayer )
	{
		libvlc_audio_set_mute( m_pVLCPlayer, false );
		libvlc_audio_set_volume( m_pVLCPlayer, 100 );
		libvlc_media_player_play( m_pVLCPlayer );
	}
}

void VLCVideoUtility::Stop()
{
	if( m_pVLCPlayer )
		libvlc_media_player_stop( m_pVLCPlayer );
}

