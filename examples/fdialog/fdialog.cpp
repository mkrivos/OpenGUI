#include <fastgl/fastgl.h>
#include <fastgl/widgets.h>

static void routine(char *filename, fgl::FGFileDialog *)
{
}

int main(int argc, char **argv)
{
	fgl::FGApp MyApp(3, argc, argv, fgl::CBLACK);
	fgl::FGFileDialog * cd = new fgl::FGFileDialog(routine, "Select your file ...");
	MyApp.Run(0);
	delete cd;
	return 0;
}
