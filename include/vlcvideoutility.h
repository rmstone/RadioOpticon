#ifndef VLC_VIDEO_UTILITY_H
#define VLC_VIDEO_UTILITY_H

#include <string>
#include <stdint.h>
#include <vector>
#include <X11/Xlib.h>

#include <vlc/vlc.h>

class VLCVideoUtility
{
public:
	VLCVideoUtility();
	~VLCVideoUtility();

	void PlayActive();
	void PlayIdle();

	bool IsPlaying();
	void Stop();

	bool IsExitPressed();

	Display *_disp;
	Window _win;

private:
	bool LoadFile( const std::string &strFile );
	void UnloadFile();
	void Play();
	bool _loading;
	XWindowAttributes _xwa;

	std::vector<std::string> _activeFiles;
	std::vector<std::string> _idleFiles;

	libvlc_media_player_t *m_pVLCPlayer;
	libvlc_instance_t *m_pVLCInstance;


};

#endif