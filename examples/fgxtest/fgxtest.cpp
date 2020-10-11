#include <fastgl/fastgl.h>
#include <fastgl/widgets.h>
#include <fastgl/fgx.h>

void NewWindow(CallBack, void* userdata)
{
	XUIBuilder* builder = static_cast<XUIBuilder*>(userdata);
	builder->Show("id1");
}

void RemoveWindow(CallBack, void* userdata)
{
	XUIBuilder* builder = static_cast<XUIBuilder*>(userdata);
	FGWindow* another_window = builder->GetWidget("id1");
	if (another_window)
		another_window->WindowClose();
}

void MyTextEntry(CallBack, const char* text, void* userdata)
{
	XUIBuilder* builder = static_cast<XUIBuilder*>(userdata);
	FGWindow* another_window = builder->GetWidget("id1");
	if (another_window)
	{
		another_window->printf(20,20,"Text input: '%s'", text);
	}
}

void MyIntegerEntry(CallBack, const int i, void* userdata)
{
	XUIBuilder* builder = static_cast<XUIBuilder*>(userdata);
	FGWindow* another_window = builder->GetWidget("id1");
	if (another_window)
	{
		another_window->printf(20,60,"Integer input: '%d'", i);
	}
}

void MyDoubleEntry(CallBack, const double d, void* userdata)
{
	XUIBuilder* builder = static_cast<XUIBuilder*>(userdata);
	FGWindow* another_window = builder->GetWidget("id1");
	if (another_window)
	{
		another_window->printf(20,100,"Double input: '%f'", d);
	}
}

void MyWindowHandler(FGEvent*e, void* userdata)
{
	printf("MyHandler %d %s\n", e->GetType(), (char *)userdata);
}

static void GenerateXUI()
{
	XUIBuilder builder;
	XUIWindow wnd1;

	XUIWindow wnd2("id1", "Yet another window", 400,0,400,600);
		FGColor w((unsigned)FGColor::white);
		FGColor b((unsigned)FGColor::black);
		XUIText text("A text label", 32,32, b,w);
		wnd2.AddComponent(&text);
		wnd2.resize = true;
		wnd2.persistent = true;

	wnd1.SetId("id0");
	wnd1.shape = FGRect(0,0,400,600);
	wnd1.frame = 0;
	wnd1.SetLabel("This is my Window!");
	wnd1.SetHandlerName("MyHandler");

	XUIPushButton button("id3", "New Window", 'N');
		button.shape = FGRect(32,30,128,25);
		button.SetSignalName("CreateWindow");
		wnd1.AddComponent(&button);
	XUIPushButton button2("id4", "Close Window", 'C');
		button2.shape = FGRect(32,70,128,25);
		button2.SetSignalName("CloseWindow");
		wnd1.AddComponent(&button2);
	XUIPushButton button3("id5", "Close Application", 'A');
		button3.shape = FGRect(32,120,128,25);
		button3.SetSignalName("__CloseApplication");
		button3.selected = true;
		wnd1.AddComponent(&button3);

	// edit
	XUIEditBox entry1("id7", "Text Entry :", 'T');
		entry1.shape = FGRect(120,220,128,25);
		entry1.SetSignalName("MyTextEntry");
		entry1.scrambled = true;
		wnd1.AddComponent(&entry1);
	XUIEditBox entry2("id8", "Integer Entry :", 'T');
		entry2.shape = FGRect(120,250,128,25);
		entry2.SetSignalName("MyIntegerEntry");
		entry2.hexadecimal = true;
		wnd1.AddComponent(&entry2);
	XUIEditBox entry3("id9", "Double Entry :", 'T');
		entry3.shape = FGRect(120,280,128,25);
		entry3.SetSignalName("MyDoubleEntry");
		wnd1.AddComponent(&entry3);

	XUICheckBox chb1("id10", "CheckBox", 'C', 32,320);
		chb1.SetSignalName("MyIntegerEntry");
		wnd1.AddComponent(&chb1);

	XUISlideBar slider("id15", 32, 350, -100, 100, 10);
		slider.SetSignalName("MyIntegerEntry");
		wnd1.AddComponent(&slider);

	XUIListBox lb("id16", 32, 380, 100, 20, 5);
		lb.SetSignalName("MyIntegerEntry");
		wnd1.AddComponent(&lb);

	const char* _label[]= { "prvy", "druhy", "treti", "stvrty", "piaty", 0 };
	int key[] = { 'p', 'd', 0, 0, 0 };

	XUIRadioGroup grp0("id12", _label, key, 200, 20, 40);
		grp0.SetSignalName("MyIntegerEntry");
		wnd1.AddComponent(&grp0);

	XUIMenuBar menu;
		XUIMenu file("id13", "File", 'F');
			XUIPopupMenu popup1("id13_1");
			popup1.shape.w = 200;
			XUIMenuItem item1("id13_1_1", "Item 1", 1);
			XUIMenuItem item2("id13_1_2", "Item 2", 2);
			XUIMenuItem item3("id13_1_3", "Quit", 'q');
			item3.selected = true;
			item3.SetSignalName("__CloseApplication");
			popup1.AddComponent(&item1);
			popup1.AddComponent(&item2);
			popup1.AddComponent(&item3);
			file.AddComponent(&popup1);
		menu.AddComponent(&file);
		XUIMenu edit("id14", "Edit", 'E');
			edit.SetSignalName("__CloseApplication");
		menu.AddComponent(&edit);
		wnd1.AddComponent(&menu);


	builder.AddWidget(wnd1);
	builder.AddWidget(wnd2);
	builder.SaveGUI("test1.xui");
}

int main(int argc, char **argv)
{
	FGApp MyApp(3,argc,argv,0,APP_ENABLEALTX | APP_CFG);
	// -------------------------------------------------------------------------

	GenerateXUI();

	XUIBuilder* builder2 = new XUIBuilder;
	builder2->LoadGUI("test1.xui");
	builder2->SaveGUI("test2.xui");
	char test_string[20]="DS";
	int ii = 4231;
	double dd = 423.423;

	// I'm using an userdata ptr to export builder instance in this case.
	RegisterOnClickSignal("CreateWindow", NewWindow, builder2 );
	RegisterOnClickSignal("CloseWindow", RemoveWindow, builder2 );

	RegisterOnEnterSignal("MyTextEntry", MyTextEntry, builder2);
	builder2->SetData("id0", "id7", test_string, 10 );
	RegisterOnEnterSignal("MyIntegerEntry", MyIntegerEntry, builder2);
	builder2->SetData("id0", "id8", ii);
	RegisterOnEnterSignal("MyDoubleEntry", MyDoubleEntry, builder2);
	builder2->SetData("id0", "id9", dd);


	builder2->Show("id0");
	builder2->SetData("id0", "id12", 1 );
	builder2->SetData("id0", "id16", "One", 0 );
	builder2->SetData("id0", "id16", "Two", 1 );

	MyApp.Run();

	delete builder2;

	CallClosureDebug();

	return 0;
}

 