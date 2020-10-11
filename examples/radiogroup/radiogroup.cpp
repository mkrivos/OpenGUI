#include <fastgl/fastgl.h>
#include <fastgl/widgets.h>

#ifdef FG_NAMESPACE
//using namespace fgl;
#endif

static void callback(fgl::CallBack cb, int index)
{
    cb->GetOwner()->SetName(cb->GetName());
    cb->GetOwner()->printf(130,58,"value of item is: %d\n", index );
}

int main(int argc, char* argv[])
{
	fgl::FGApp app(3,argc,argv,0,fgl::APP_ALL);
	
	fgl::FGWindow* w = new fgl::FGWindow(0,100,100,600,240,"Window", 0, fgl::CWHITE, fgl::CGRAYED);
	
	char *strings[] = {
	    "Item 1",
	    "Item 2",
	    "Item 3",
	    "Item 4",
	    "Item 5",
	    "Item 6",
	    "Item 7",
	    "Last Item",
	    0 };
	
	int keys[]={ '1', '2', '3', '4', '5', '6', '7', 'L'};
	
	// create new RadioGroup with active item at index 2 (third line)	    
	int controled_variable1 = 2;
	fgl::FGRadioGroupVertical* grp = new fgl::FGRadioGroupVertical(32,42,strings,keys,&controled_variable1,callback,w);
	
	int controled_variable2 = 7;
	fgl::FGRadioGroupHorizontal* grp2 = new fgl::FGRadioGroupHorizontal(32,12,strings,keys,&controled_variable2,callback,w,64);

	app.Run();
	
	delete grp;	
	delete grp2;	
	
	return 0;
}
