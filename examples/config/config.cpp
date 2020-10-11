#include <stdio.h>
#include <fastgl/fastgl.h>
#include "tinycfg.h"


int main(int argc, char **argv)
{
	int number = -1;
	double number2= 12345.67890;
	bool bool_array[3] = { true, false, true };
	int int_array[3] = { true, false, true };
	double double_array[3] = { 3.14, 26346346643576.7456, -0.00032 };
	char* str_array[3] = { "fdsfsd", "26346346643576.7456", "Jano ma maju" };
	char tmpstr[32];
	FGPoint points[4];
	points[0] = FGPoint(0,0);
	points[1] = FGPoint(50,100);
	points[2] = FGPoint(100,50);
	points[3] = FGPoint(200,100);

	Config* cCfg = new Config();
	cCfg->Load("bench.xml");

	bool foo = true;
	cCfg->WriteBool("foo", foo);
	foo = false;
	cCfg->WriteBool("foo", foo, "foo.bar");

	cCfg->WriteInt("integer_variable2", number);
	cCfg->WriteDouble("double_variable", number2);
	cCfg->WriteDouble("double_variable", 3.14);
	cCfg->WriteString("string_variable", "Hello, world <> ' ");
	cCfg->WriteBool("bool_variable", true);
	cCfg->WriteHex("hex_variable", (char *)points, sizeof(points));

	cCfg->WriteBoolArray("bool_array", bool_array, 3);

	cCfg->WriteIntArray("int_array", int_array, 3);

	cCfg->WriteDoubleArray("double_array", double_array, 3);

	cCfg->WriteStringArray("str_array", str_array, 3);
	cCfg->WriteHexArray("hex_array", str_array, 12, 3);

	cCfg->ReadInt("integer", number, "nejaka_grupa");
	cCfg->ReadInt("integer", number, "mayapp.data.bank0");
	cCfg->Save();
	cCfg->ReadBool("bool", bool_array[0]);
	cCfg->ReadDouble("double", number2);

	cCfg->ReadString("string", tmpstr, 32);
	cCfg->ReadHex("hex", (char *)points, 4);

	cCfg->ReadBoolArray("bool_array", bool_array, 4);
	cCfg->ReadIntArray("int_array", int_array, 3);

	return 0;
}


