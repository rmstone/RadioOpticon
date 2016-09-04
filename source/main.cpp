#include <stdio.h>
#include <string>
#include <pthread.h>
#include <X11/keysym.h>

#include "Create.h"
#include "Arduino.h"
#include "vlcvideoutility.h"


std::string createPort = "/dev/serial/by-id/usb-Keyspan__a_division_of_InnoSys_Inc._Keyspan_USA-19H-if00-port0";
std::string arduinoPort = "/dev/serial/by-id/usb-SparkFun_SparkFun_Pro_Micro-if00";

Create *create;
Arduino *arduino;
VLCVideoUtility *video;
pthread_t windowThread;

enum State
{
	initialize = 0,
	running,
	found,
	low_battery,
	charging,
	error,
	exit,
};

volatile State state;

void *WindowThreadRun( void * )
{
	XEvent report;
	bool running = true;
	while( running )
	{
		XNextEvent( video->_disp, &report );
		switch( report.type )
		{
			case KeyPress:
				if( XLookupKeysym( &report.xkey, 0 ) == XK_Escape )
				{
					printf( "Escape was pressed.\n" );
					running = false;
					state = exit;
				}
				break;
		}
	}
}

State InitializeSystem()
{
	create = new Create( createPort );
	arduino = new Arduino( arduinoPort );
	video = new VLCVideoUtility();
	if( pthread_create( &windowThread, NULL, WindowThreadRun, NULL ) )
		return error;
	return running;
}

void Shutdown()
{
	delete create;
	delete arduino;
	delete video;
}

int main()
{
	state = initialize;
	while( state != exit )
	{
		switch( state )
		{
			case initialize:
				printf( "Initializing...\n" );
				state = InitializeSystem();
				printf( "Initialization complete, Driving around\n" );
				break;
			case running:
				if( !video->IsPlaying() )
					video->PlayIdle();
				if( create->IsBatteryLow() )
					state = low_battery;
				else if( arduino->CheckSensors() == personFound )
				{
					state = found;
					create->Stop();
					create->TurnAround();
				}
				else
					create->Driving();
				break;
			case found:
				state = running;
				printf( "Found someone, changing video\n" );
				video->PlayActive();
				while( video->IsPlaying() );
				printf( "video done, driving around again\n" );
				arduino->ClearFound();
				break;
			case low_battery:
				state = charging;
				create->GoHome();
				break;
			case charging:
				if( !video->IsPlaying() )
					video->PlayActive();
				if( create->IsCharged() )
				{
					create->Undock();
					if( state != exit )
						state = running;
				}
				break;
			case error:	//for testing only
				printf( "Error, exitting...\n" );
				state = exit;
				break;
			default:
				printf( "Unknown state: %d", state );
				state = exit;
				break;
		}
	}
	Shutdown();
}