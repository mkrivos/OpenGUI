/*
*/

#include <fastgl/fastgl.h>


int main(int argc, char **argv)
{
	fgl::FGApp app(fgl::G640x480,argc,argv,0,fgl::APP_ENABLEALTX);
	
	fgl::FGDialog dlg(CWHITE, CBLACK, "Dialog box", "User defined string %f", 3.14);
	
	int rc = dlg.ShowOk();
	
	switch(rc)
	{
	    case fgl::mrOk:
		printf("Ok\n");
		break;
	    case fgl::mrCancel:
		printf("Cancel\n");
		break;
	}
	
	return 0;
}

