/**
  $Id: tinycfg.cpp 4679 2006-04-05 13:16:35Z majo $
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <tinyxml.h>

#include "xmlconfig.h"

/**
	An implementation of config file format using XML.
*/
class XMLConfig : public ConfigInterface
{
		friend class OldConfig;
		static const int MAX_HEX_LEN = 512;
		static const int MAX_CHILD_DEPTH = 32;
		TiXmlDocument* document;
		TiXmlNode* rootnode;
		char filename[256];
		bool change;
		bool writable;

		void init(void);
		TiXmlNode* find_group(const char* domain);
		TiXmlElement* find(const char* name, const char* type, const char* domain);
		TiXmlElement* find_array(const char* name, const char* type, int& range, const char* domain);

		const char* extract_node_name(const char* ptr, char* name, int& len);
		void remove_if_exists(const char* name, const char* type, const char* domain);

		TiXmlNode* WriteGroup(TiXmlNode* node, const char* name);
	protected:
		virtual bool Create(const char* fname);
	public:
		XMLConfig(bool write = true);
		virtual ~XMLConfig();

		virtual bool Load(const char* fname);
		virtual bool Save(void);
		virtual bool IsValid();

		virtual bool ReadBool(const char * name, bool &data, const char* domain = 0);
		virtual bool ReadInt(const char * name, int &data, const char* domain = 0);
		virtual bool ReadString(const char * name,	char *data, int max, const char* domain = 0);
		virtual bool ReadDouble(const char * name, double &data, const char* domain = 0);
		virtual bool ReadHex(const char * name, char *data, int length_of_item, const char* domain = 0);

		virtual bool ReadBoolArray(const char * name, bool data[], int range, const char* domain = 0);
		virtual bool ReadIntArray(const char * name, int data[], int range, const char* domain = 0);
		virtual bool ReadStringArray(const char * name, char* data[], int max, int range, const char* domain = 0);
		virtual bool ReadDoubleArray(const char * name, double data[], int range, const char* domain = 0);
		virtual bool ReadHexArray(const char * name, char* data[], int length_of_item, int range, const char* domain = 0);

		virtual void WriteBool(const char * name, bool data, const char* domain = 0);
		virtual void WriteInt(const char * name, int data, const char* domain = 0);
		virtual void WriteString(const char * name, const char* data, const char* domain = 0);
		virtual void WriteDouble(const char * name, double data, const char* domain = 0);
		virtual void WriteHex(const char * name, char* data, int length_of_item, const char* domain = 0);

		virtual void WriteBoolArray(const char * name, bool data[], int range, const char* domain = 0);
		virtual void WriteIntArray(const char * name, int data[], int range, const char* domain = 0);
		virtual void WriteStringArray(const char * name, const char *data[], int range, const char* domain = 0);
		virtual void WriteDoubleArray(const char * name, double data[], int range, const char* domain = 0);
		virtual void WriteHexArray(const char * name, char *data[], int length_of_item, int range, const char* domain = 0);


		virtual bool ReadString(const char * name,	std::string& data, const char* domain = 0);
		virtual bool ReadStringArray(const char * name, std::string data[], int range, const char* domain = 0);
		virtual void WriteString(const char * name, std::string& data, const char* domain = 0);
		virtual void WriteStringArray(const char * name, std::string data[], int range, const char* domain = 0);	
		
		bool ExistValid(const char* fname);
};

//
static int ChToI(char xCH)
{
	if ((xCH >= '0') & (xCH <= '9'))
		return ((int) xCH) - '0';
	if ((xCH >= 'a') & (xCH <= 'f'))
		return ((int) xCH) - 'a' + 10;
	if ((xCH >= 'A') & (xCH <= 'F'))
		return ((int) xCH) - 'A' + 10;
	return -1;
}

static int hexdata(const char *name)
{
	int a = ChToI(name[0]);
	int b = ChToI(name[1]);
	if (a!=-1 && b!=-1) return a*16 + b;
	return 256;
}

#define STRING_STRING			"string"
#define INT_STRING				"int"
#define DOUBLE_STRING  			"double"
#define BOOL_STRING				"bool"
#define HEX_STRING				"hex"

#define ARRAY_STRING			"array"
#define ITEM_STRING				"item"
#define GROUP_STRING	 		"group"
#define VALUE_STRING	 		"val"
#define NAME_STRING	 			"name"
#define TYPE_STRING	 			"type"
#define LEN_STRING	 			"len"
#define RANGE_STRING   			"range"

XMLConfig::XMLConfig(bool write)
{
	init();
	writable = write;
}

void XMLConfig::init(void)
{
	document = 0;
	rootnode = 0;
	writable = false;
	filename[0] = 0;
	change = true;
}

XMLConfig::~XMLConfig()
{
	if (writable) Save();
	delete document;
	document = 0;
	init();
}

/**
	@return true if object state is valid.
*/
bool XMLConfig::IsValid()
{
	if (document == 0)
		return false;
	if (rootnode == 0)
		return false;
	
	return true;
}

/**
	Creates & saves an empty XML file in OpenGUI INI format.
*/
bool XMLConfig::Create(const char* fname)
{
	strncpy(filename, fname, sizeof(filename)-1);

	if (ExistValid(filename)) return true;

	// Vytvor nejaky prazdny XML subor
	const char* demoStart =
		"<?xml version=\"1.0\" standalone='yes' >\n"
		"<tinycfg>\n"
		"</tinycfg>";

	TiXmlDocument tmpdoc( filename );
	tmpdoc.Parse( demoStart );
	change = true;
	if ( tmpdoc.Error() )
	{
//		printf( "Error in %s: %s\n", tmpdoc.Value(), tmpdoc.ErrorDesc() );
		return false;
	}
	return tmpdoc.SaveFile();
}

bool XMLConfig::ExistValid(const char* fname)
{
	FILE* fp = fopen(fname, "r");
	bool ret = false;

	if (fp)
	{   char gps[80];
		fgets(gps,79,fp);
		ret = (strcmp(gps, "<?xml version=\"1.0\"  standalone='yes' >\n") == 0);
		fclose(fp);
	}
	return ret;
}

bool XMLConfig::Load(const char* fname)
{
	// save previous if any
	TiXmlDocument* old = document;
	TiXmlDocument* doc = new TiXmlDocument( fname );
	bool loadOkay = doc->LoadFile();

	if ( !loadOkay || doc->RootElement() == 0)
	{
//		printf( "Could not load test file '%s'. Error='%s'. Exiting.\n", fname, doc->ErrorDesc() );
		bool isnew = Create(fname);
		if (isnew == false)
		{
			document = old;	// comeback
			delete doc;
			return false;
		}
		loadOkay = doc->LoadFile();
		if ( !loadOkay )
		{
			delete doc;
			return false;
		}
	}

	if (old) delete old;

	document = doc;
	rootnode = document->RootElement();
	strncpy(filename, fname, sizeof(filename)-1);
	change = true;
	return !!rootnode;
}

/**
	Saves the document if any.
	@return true if OK
*/
bool XMLConfig::Save(void)
{
	if (writable == true && change == true && document)
	{
		change = false;
		return document->SaveFile();
	}
	return false;
}

TiXmlElement* XMLConfig::find(const char* name, const char* type, const char* domain)
{
	TiXmlElement* element = 0;
	const char* tmp_name;
	const char* tmp_value;

	TiXmlNode* node = find_group(domain);
	if (node == 0) return 0;

	for(element = node->FirstChildElement(type); element; element = element->NextSiblingElement(type))
	{
		tmp_name = element->Attribute(NAME_STRING);
		tmp_value = element->Attribute(VALUE_STRING);

		if (tmp_name && tmp_value)
		{
			if ( strcmp(tmp_name, name) == 0)
			{
				return element;
			}
		}
	}
	return 0;
}

TiXmlElement* XMLConfig::find_array(const char* name, const char* type, int& range, const char* domain)
{
	TiXmlElement* element = 0;
	const char* tmp_name;

	TiXmlNode* node = find_group(domain);
	if (node == 0) return 0;

	for(element = node->FirstChildElement(type); element; element = element->NextSiblingElement(type))
	{
		tmp_name = element->Attribute(NAME_STRING);
		range = 0;
		element->QueryIntAttribute(RANGE_STRING, &range);

		if (tmp_name)
		{
			if ( strcmp(tmp_name, name) == 0 )
			{
				// an empty array
				if (range == 0)
					return 0;

				return element->FirstChildElement();
			}
		}
	}
	return 0;
}

/**
	@return the node with group of this name (eg. 'myapp.somedata.bank0')
*/
TiXmlNode* XMLConfig::find_group(const char* domain)
{
	if (domain == 0) return rootnode;

	assert(rootnode);
	
	TiXmlHandle docHandle( rootnode );
	TiXmlHandle childHandle(docHandle);
	char node_name[64];
	const char* tmp = domain;
	int len;

	for(int depth = 0; depth < MAX_CHILD_DEPTH; depth++)
	{
		node_name[0] = 0;
		tmp = extract_node_name(tmp, node_name, len);

		if (tmp == 0 || len < 1)
		{
			printf("Bad domain name '%s' for config variable!\a\n", domain);
			return rootnode;		// bad name character
		}

		childHandle = docHandle.FirstChild(node_name);

		if (childHandle.Node() == 0)
		{
			WriteGroup(docHandle.Node(), node_name);
			childHandle = docHandle.FirstChild(node_name);
		}

		docHandle = childHandle;

		if (*tmp == 0)
			return docHandle.Node();    // the end of domain

	}
	return rootnode;
}

TiXmlNode* XMLConfig::WriteGroup(TiXmlNode* node, const char* name)
{
	TiXmlElement item( name );
	TiXmlNode* where = node->InsertEndChild( item );
	return where;
}

const char* XMLConfig::extract_node_name(const char* ptr, char* name, int& len)
{
	int maxi = 63;
	int c;
	len = 0;

	while( (c = *ptr) != 0)
	{
		if ( isalnum(c) || c == '_')
		{
			*name++ = c;
			*name = 0;
			maxi--;
			if (maxi == 0) return 0;
			ptr++;
			len++;
			continue;
		}
		else if (c == '.')
			return ++ptr;
		break;
	}
	return ptr;
}

void XMLConfig::remove_if_exists(const char* name, const char* type, const char* domain)
{
	int range = 0;
	TiXmlElement*ele = find_array(name, type, range, domain);
	if (ele == 0) return;

	TiXmlNode* orig = ele->Parent();
	while(ele)
	{
		ele->Parent()->RemoveChild(ele);
		ele = find_array(name, type, range, domain);
	}
	orig->Parent()->RemoveChild(orig);
	return;
}

// -------------------------------------------------------------------------

bool XMLConfig::ReadBool(const char* name, bool &value, const char* domain)
{
	TiXmlElement* text = find(name, BOOL_STRING, domain);

	// not found!
	if (text == 0) return false;
	value = !!strcmp(text->Attribute(VALUE_STRING), "false");
	return true;
}

bool XMLConfig::ReadInt(const char* name, int &value, const char* domain)
{
	TiXmlElement* text = find(name, INT_STRING, domain);

	// not found!
	if (text == 0)
		return false;
	return text->QueryIntAttribute(VALUE_STRING, &value) == TIXML_SUCCESS;
}

bool XMLConfig::ReadString(const char* name, char *value, int max, const char* domain)
{
	TiXmlElement* text = find(name, STRING_STRING, domain);

	// not found!
	if (text == 0)
		return false;
	strncpy(value, text->Attribute(VALUE_STRING), max);
	return true;
}

bool XMLConfig::ReadDouble(const char* name, double &value, const char* domain)
{
	TiXmlElement* text = find(name, DOUBLE_STRING, domain);

	// not found!
	if (text == 0)
		return false;
	return text->QueryDoubleAttribute(VALUE_STRING, &value) == TIXML_SUCCESS;
}

bool XMLConfig::ReadHex(const char* name, char *data, int len, const char* domain)
{
	assert(len <= MAX_HEX_LEN);

	TiXmlElement* text = find(name, HEX_STRING, domain);

	// not found!
	if (text == 0) return false;
	const char* hexstr = text->Attribute(VALUE_STRING);

	int i = 0, val;
	while(hexstr && hexstr[i*2] && ( val = hexdata(hexstr+i*2)) < 256 && i<len )
	{
		data[i++] = val;
	}
	return true;
}

bool XMLConfig::ReadBoolArray(const char* name, bool data[], int range, const char* domain)
{
	int in_file_range = 0;
	TiXmlElement* items = find_array(name, BOOL_STRING, in_file_range, domain);

	if (items == 0) return false;

	for (int i = 0; i < range && items; i++)
	{
		data[i] = false;    // initialize array if no value
		const char* value = items->Attribute(VALUE_STRING);
		if (value)
			data[i] = !!strcmp(value, "false");
		items = items->NextSiblingElement(ITEM_STRING);
	}
	return true;
}

bool XMLConfig::ReadIntArray(const char* name, int data[], int range, const char* domain)
{
	int in_file_range = 0;
	TiXmlElement* items = find_array(name, INT_STRING, in_file_range, domain);

	if (items == 0) return false;

	for (int i = 0; i < range && items; i++)
	{
		data[i] = 0;    // initialize array if no value
		items->QueryIntAttribute(VALUE_STRING, &data[i]);
		items = items->NextSiblingElement(ITEM_STRING);
	}
	return true;
}

bool XMLConfig::ReadStringArray(const char* name, char *data[], int max, int range, const char* domain)
{
	int in_file_range = 0;
	TiXmlElement* items = find_array(name, STRING_STRING, in_file_range, domain);

	if (items == 0) return false;

	for (int i = 0; i < range && items; i++)
	{
		data[i][0] = 0;    // initialize array if no value
		const char* value = items->Attribute(VALUE_STRING);
		if (value)
			strncpy(&data[i][0], value, max);
		items = items->NextSiblingElement(ITEM_STRING);
	}
	return true;
}

bool XMLConfig::ReadDoubleArray(const char* name, double data[], int range, const char* domain)
{
	int in_file_range = 0;
	TiXmlElement* items = find_array(name, DOUBLE_STRING, in_file_range, domain);

	if (items == 0) return false;

	for (int i = 0; i < range && items; i++)
	{
		data[i] = 0.0;    // initialize array if no value
		items->QueryDoubleAttribute(VALUE_STRING, &data[i]);
		items = items->NextSiblingElement(ITEM_STRING);
	}
	return true;
}

bool XMLConfig::ReadHexArray(const char* name, char *data[], int length_of_item, int range, const char* domain)
{
	int in_file_range = 0;
	TiXmlElement* items = find_array(name, HEX_STRING, in_file_range, domain);

	if (items == 0) return false;

	assert(length_of_item <= MAX_HEX_LEN);

	for (int i = 0; i < range && items; i++)
	{
		memset(&data[i][0], 0, length_of_item);    // initialize array if no value

		int j = 0, val;
		const char* value = items->Attribute(VALUE_STRING);
		if (value)
			while( value[j*2] && ( val = hexdata(value+j*2)) < 256 && j<length_of_item )
		{
			data[i][j++] = val;
		}
		items = items->NextSiblingElement(ITEM_STRING);
	}
	return true;
}

void XMLConfig::WriteString(const char * name, const char * value, const char* domain)
{
	TiXmlElement* found_item = find(name, STRING_STRING, domain);

	if (found_item)
	{
		found_item->SetAttribute( VALUE_STRING, value );
	}
	else
	{
		TiXmlElement* item = new TiXmlElement( STRING_STRING );
		item->SetAttribute( NAME_STRING, name );
		item->SetAttribute( VALUE_STRING, value );
		TiXmlNode* node = find_group(domain);
		node->InsertEndChild( *item );
		delete item;
		item = 0;
	}
	change = true;
}

void XMLConfig::WriteInt(const char * name, int value, const char* domain)
{
	TiXmlElement* text = find(name, INT_STRING, domain);

	if (text)
	{
		text->SetAttribute( VALUE_STRING, value );
	}
	else
	{
		TiXmlElement* item = new TiXmlElement( INT_STRING );
		item->SetAttribute( NAME_STRING, name );
		item->SetAttribute( VALUE_STRING, value );
		TiXmlNode* node = find_group(domain);
		node->InsertEndChild( *item );
		delete item;
		item = 0;
	}
	change = true;
}

void XMLConfig::WriteDouble(const char* name, double value, const char* domain)
{
	TiXmlElement* text = find(name, DOUBLE_STRING, domain);

	if (text)
	{
		text->SetDoubleAttribute( VALUE_STRING, value );
	}
	else
	{
		TiXmlElement* item = new TiXmlElement( DOUBLE_STRING );

		item->SetAttribute( NAME_STRING, name );
		item->SetDoubleAttribute( VALUE_STRING, value );
		TiXmlNode* node = find_group(domain);
		node->InsertEndChild( *item );
		delete item;
		item = 0;
	}
	change = true;
}

void XMLConfig::WriteBool(const char * name, bool value, const char* domain)
{
	TiXmlElement* text = find(name, BOOL_STRING, domain);

	if (text) // if exist
	{
		text->SetAttribute(VALUE_STRING, value ? "true" : "false" );
	}
	else      // if not exist
	{
		TiXmlElement* item = new TiXmlElement( BOOL_STRING );

		item->SetAttribute( NAME_STRING, name );
		item->SetAttribute(VALUE_STRING, value ? "true" : "false" );

		TiXmlNode* node = find_group(domain);
		node->InsertEndChild( *item );
		delete item;
		item = 0;
	}
	change = true;
}

void XMLConfig::WriteHex(const char * name, char *value, int len, const char* domain)
{
	assert(len <= MAX_HEX_LEN);

	char hex[MAX_HEX_LEN * 2 + 1];
	*hex = 0;
	for(int j=0;j<len;j++)
	{
		sprintf(hex+j*2, "%02x", value[j]&255);
		hex[j*2+2] = 0;
	}

	TiXmlElement* text = find(name, HEX_STRING, domain);

	if (text)
	{
		text->SetAttribute(VALUE_STRING, hex);
	}
	else
	{
		TiXmlElement* item = new TiXmlElement( HEX_STRING );
		item->SetAttribute( NAME_STRING, name );
		item->SetAttribute( VALUE_STRING, hex);
		item->SetAttribute( LEN_STRING, len);
		TiXmlNode* node = find_group(domain);
		node->InsertEndChild( *item );
		delete item;
		item = 0;
	}
	change = true;
}

void XMLConfig::WriteBoolArray(const char * name, bool data[], int range, const char* domain)
{
	TiXmlElement* item = new TiXmlElement( BOOL_STRING );

	remove_if_exists(name, BOOL_STRING, domain);

	item->SetAttribute( NAME_STRING, name );
	item->SetAttribute( RANGE_STRING, range );

	for( int i = 0; i < range; i++)
	{
		TiXmlElement array_item( ITEM_STRING );
		array_item.SetAttribute( VALUE_STRING, data[i] ? "true" : "false" );
		item->InsertEndChild( array_item );
	}

	TiXmlNode* node = find_group(domain);
	node->InsertEndChild( *item );
	delete item;
	item = 0;
	change = true;
}

void XMLConfig::WriteIntArray(const char* name, int data[], int range, const char* domain)
{
	TiXmlElement* item = new TiXmlElement( INT_STRING );

	remove_if_exists(name, INT_STRING, domain);

	item->SetAttribute( NAME_STRING, name );
	item->SetAttribute( RANGE_STRING, range );

	for( int i = 0; i < range; i++)
	{
		TiXmlElement array_item( ITEM_STRING );
		array_item.SetAttribute( VALUE_STRING, data[i]);
		item->InsertEndChild( array_item );
	}
	TiXmlNode* node = find_group(domain);
	node->InsertEndChild( *item );
	delete item;
	item = 0;
	change = true;
}

void XMLConfig::WriteStringArray(const char* name, const char* data[], int range, const char* domain)
{
	remove_if_exists(name, STRING_STRING, domain);

	TiXmlElement* item = new TiXmlElement( STRING_STRING );
	item->SetAttribute( NAME_STRING, name );
	item->SetAttribute( RANGE_STRING, range );

	for( int i = 0; i < range; i++)
	{
		TiXmlElement array_item( ITEM_STRING );
		array_item.SetAttribute( VALUE_STRING, data[i]);
		item->InsertEndChild( array_item );
	}

	TiXmlNode* node = find_group(domain);
	node->InsertEndChild( *item );
	delete item;
	item = 0;
	change = true;
}

void XMLConfig::WriteDoubleArray(const char* name, double data[], int range, const char* domain)
{
	remove_if_exists(name, DOUBLE_STRING, domain);

	TiXmlElement* item = new TiXmlElement( DOUBLE_STRING );
	item->SetAttribute( NAME_STRING, name );
	item->SetAttribute( RANGE_STRING, range );

	for( int i = 0; i < range; i++)
	{
		TiXmlElement array_item( ITEM_STRING );
		array_item.SetDoubleAttribute( VALUE_STRING, data[i]);
		item->InsertEndChild( array_item );
	}

	TiXmlNode* node = find_group(domain);
	node->InsertEndChild( *item );
	delete item;
	item = 0;
	change = true;
}

void XMLConfig::WriteHexArray(const char* name, char * data[], int length_of_item, int range, const char* domain)
{
	assert(length_of_item <= MAX_HEX_LEN);
	char hex[MAX_HEX_LEN * 2 + 1];

	remove_if_exists(name, HEX_STRING, domain);

	TiXmlElement* item = new TiXmlElement( HEX_STRING );

	item->SetAttribute( NAME_STRING, name );
	item->SetAttribute( LEN_STRING, length_of_item);
	item->SetAttribute( RANGE_STRING, range );

	for( int i = 0; i < range; i++)
	{
		*hex = 0;
		for(int j=0;j<length_of_item;j++)
		{
			sprintf(hex+j*2, "%02x", data[i][j]&255);
			hex[j*2+2] = 0;
		}

		TiXmlElement array_item( ITEM_STRING );
		array_item.SetAttribute( VALUE_STRING, hex);
		item->InsertEndChild( array_item );
	}

	TiXmlNode* node = find_group(domain);
	node->InsertEndChild( *item );
	delete item;
	item = 0;
	change = true;
}

bool XMLConfig::ReadString(const char* name,	std::string& data, const char* domain)
{
	TiXmlElement* text = find(name, STRING_STRING, domain);

	// not found!
	if (text == 0)
		return false;
	
	data = text->Attribute(VALUE_STRING);
	return true;
}

bool XMLConfig::ReadStringArray(const char* name, std::string data[], int range, const char* domain)
{
	int in_file_range = 0;
	TiXmlElement* items = find_array(name, STRING_STRING, in_file_range, domain);

	if (items == 0) return false;

	for (int i = 0; i < range && items; i++)
	{
		data[i] = "";    // initialize array if no value
		const char* value = items->Attribute(VALUE_STRING);
		if (value)
			data[i] = value;
		items = items->NextSiblingElement(ITEM_STRING);
	}
	return true;
}

void XMLConfig::WriteString(const char* name, std::string& value, const char* domain)
{
	TiXmlElement* found_item = find(name, STRING_STRING, domain);

	if (found_item)
	{
		found_item->SetAttribute( VALUE_STRING, value.c_str() );
	}
	else
	{
		TiXmlElement* item = new TiXmlElement( STRING_STRING );
		item->SetAttribute( NAME_STRING, name );
		item->SetAttribute( VALUE_STRING, value.c_str() );
		TiXmlNode* node = find_group(domain);
		node->InsertEndChild( *item );
		delete item;
		item = 0;
	}
	change = true;
}

void XMLConfig::WriteStringArray(const char* name, std::string value[], int range, const char* domain)
{
	remove_if_exists(name, STRING_STRING, domain);

	TiXmlElement* item = new TiXmlElement( STRING_STRING );
	item->SetAttribute( NAME_STRING, name );
	item->SetAttribute( RANGE_STRING, range );

	for( int i = 0; i < range; i++)
	{
		TiXmlElement array_item( ITEM_STRING );
		array_item.SetAttribute( VALUE_STRING, value[i].c_str() );
		item->InsertEndChild( array_item );
	}

	TiXmlNode* node = find_group(domain);
	node->InsertEndChild( *item );
	delete item;
	item = 0;
	change = true;
}

// -----------------------------------------------------------------------------

/**
	Creates and loads the config object from the file.
	@param s the file name
	@param write the flag for object writing, false = read-only object.
*/
Config::Config(const char	*s, bool write)
{
	realcfg = new XMLConfig(write);
	realcfg->Load(s);
}

/**
	Creates the new and empty config object.
	@param write the flag for object writing, false = read-only object.
*/
Config::Config(bool write)
{
	realcfg = new XMLConfig(write);
}

/**
	Destruct object.
	The object will be saved if is 'writable' and state is 'changed'.
*/
Config::~Config()
{
	delete realcfg;
	realcfg = 0;
}

/**
	@return true if object state is valid.
*/
bool Config::IsValid()
{
	if (realcfg == 0)
		return false;
	
	return realcfg->IsValid();
}

/**
	Saves the object if is 'writable' and state is 'changed'.
	@return true if object was correctly load from file.
*/
bool Config::Save(void)
{
	return realcfg->Save();
}

/**
	Loads object from the file.
	@param fname the file name
	@return true if object was correctly load from file.
*/
bool Config::Load(const char* fname)
{
	return realcfg->Load(fname);
}

/**
	Reads an boolean variable from the config file.
	@param name the symbolic name of variable
	@param data where the value of the variable will be stored
	@param domain the path where variable is stored
	@return true if variable was found and loaded correctly to the data parameter. If false the 'data' parameter is untouched.
*/
bool Config::ReadBool(const char* name, bool &data, const char* domain)
{
	return realcfg->ReadBool(name, data, domain);
}

/**
	Reads an integer variable from the config file.
	@param name the symbolic name of variable
	@param data where the value of the variable will be stored
	@param domain the path where variable is stored
	@return true if variable was found and loaded correctly to the data parameter. If false the 'data' parameter is untouched.
*/
bool Config::ReadInt(const char* name, int &data, const char* domain)
{
	return realcfg->ReadInt(name, data, domain);
}

/**
	Reads a double variable from the config file.
	@param name the symbolic name of variable
	@param data where the value of the variable will be stored
	@param domain the path where variable is stored
	@return true if variable was found and loaded correctly to the data parameter. If false the 'data' parameter is untouched.
*/
bool Config::ReadDouble(const char* name, double &data, const char* domain)
{
	return realcfg->ReadDouble(name, data, domain);
}

/**
	Reads a string variable from the config file.
	@param name the symbolic name of variable
	@param data the pointer where the string will be stored
	@param max the size of allocated memory for string
	@param domain the path where variable is stored
	@return true if variable was found, memory size fits its size and loaded correctly to the data parameter. If false the 'data' parameter is untouched.
*/
bool Config::ReadString(const char* name,	char *data, int max, const char* domain)
{
	return realcfg->ReadString(name, data, max, domain);
}

/**
	Reads a block of binary data from the config file.
	@param name the symbolic name of variable
	@param data the pointer where the data will be stored
	@param size the size of allocated memory for data (max 512)
	@param domain the path where variable is stored
	@return true if variable was found, memory size fits its size and loaded correctly to the data parameter. If false the 'data' parameter is untouched.
*/
bool Config::ReadHex(const char* name, char *data, int size, const char* domain)
{
	return realcfg->ReadHex(name, data, size, domain);
}

/**
	Writes an item of type boolean with the 'name' to the config object (to save to the file use Config::Save() )
	@param name the name of variable
	@param val the value to write (true or false)
	@param domain the path where variable will be stored
	@see Config::Config()
*/
void Config::WriteBool(const char* name, bool val, const char* domain)
{
	realcfg->WriteBool(name, val, domain);
}

/**
	Writes an item of type 'int' with the 'name' to the config object (to save to the file use Config::Save() )
	@param name the name of variable
	@param val the value to write ( C integer )
	@param domain the path where variable will be stored
	@see Config::Config()
*/
void Config::WriteInt(const char* name, int val, const char* domain)
{
	realcfg->WriteInt(name, val, domain);
}

/**
	Writes an item of type 'char *' with the 'name' to the config object (to save to the file use Config::Save() )
	@param name the name of variable
	@param val the string to write (a traditional ASCIIZ form)
	@param domain the path where variable will be stored
	@see Config::Config()
*/
void Config::WriteString(const char* name, const char *val, const char* domain)
{
	Config::realcfg->WriteString(name, val, domain);
}

/**
	Writes an item of type 'double' with the 'name' to the config object (to save to the file use Config::Save() )
	@param name the name of variable
	@param val the value to write
	@param domain the path where variable will be stored
	@see Config::Config()
*/
void Config::WriteDouble(const char* name, double val, const char* domain)
{
	realcfg->WriteDouble(name, val, domain);
}

/**
	Writes an item of type 'char *' with the 'name' to the config object (to save to the file use Config::Save() ).
	This saves memory contents as HEXADECIMAL string. Bytes 00ff00 will be writed as  "00FF00" ascii string.
	@param name the name of variable
	@param val the value to write ( C integer )
	@param len the size of object in bytes to write (max. 512 bytes)
	@param domain the path where variable will be stored
	@see Config::Config()
*/
void Config::WriteHex(const char* name, char* val, int len, const char* domain)
{
	realcfg->WriteHex(name, val, len, domain);
}

/**
	Writes an array of boolean type with name and range.
	@param name the name of variable
	@param data data array to write
	@param range the number of items in the array
	@param domain the path where variable will be stored
*/
void Config::WriteBoolArray(const char* name, bool data[], int range, const char* domain)
{
	realcfg->WriteBoolArray(name, data, range, domain);
}

/**
	Writes an array of integer type with name and range.
	@param name the name of variable
	@param data data array to write
	@param range the number of items in the array
	@param domain the path where variable will be stored
*/
void Config::WriteIntArray(const char* name, int data[], int range, const char* domain)
{
	realcfg->WriteIntArray(name, data, range, domain);
}

/**
	Writes an array of double type with name and range.
	@param name the name of variable
	@param data data array to write
	@param range the number of items in the array
	@param domain the path where variable will be stored
*/
void Config::WriteDoubleArray(const char* name, double data[], int range, const char* domain)
{
	realcfg->WriteDoubleArray(name, data, range, domain);
}

/**
	Writes an array of char* type with name and range.
	@param name the name of variable
	@param data data array to write
	@param range the number of items in the array
	@param domain the path where variable will be stored
*/
void Config::WriteStringArray(const char* name, const char *data[], int range, const char* domain)
{
	realcfg->WriteStringArray(name, data, range, domain);
}

/**
	Writes an array of char* as binary data type with name and range.
	@param name the name of variable
	@param data data array to write
	@param length_of_item the sizeof of one data item
	@param range the number of items in the array
	@param domain the path where variable will be stored
*/
void Config::WriteHexArray(const char* name, char *data[], int length_of_item, int range, const char* domain)
{
	return realcfg->WriteHexArray(name, data, length_of_item, range, domain);
}

/**
	Reads an array of boolean from the config file.
	@param name the symbolic name of variable
	@param data where the value of the variable will be stored
	@param range the number of items to read
	@param domain the path where variable is stored
	@return true if variable was found and loaded correctly to the data parameter. If false the 'data' parameter is untouched.
*/
bool Config::ReadBoolArray(const char* name, bool data[], int range, const char* domain)
{
	return realcfg->ReadBoolArray(name, data, range, domain);
}

/**
	Reads an array of integer from the config file.
	@param name the symbolic name of variable
	@param data where the value of the variable will be stored
	@param range the number of items to read
	@param domain the path where variable is stored
	@return true if variable was found and loaded correctly to the data parameter. If false the 'data' parameter is untouched.
*/
bool Config::ReadIntArray(const char* name, int data[], int range, const char* domain)
{
	return realcfg->ReadIntArray(name, data, range, domain);
}

/**
	Reads an array of double from the config file.
	@param name the symbolic name of variable
	@param data where the value of the variable will be stored
	@param range the number of items to read
	@param domain the path where variable is stored
	@return true if variable was found and loaded correctly to the data parameter. If false the 'data' parameter is untouched.
*/
bool Config::ReadDoubleArray(const char* name, double data[], int range, const char* domain)
{
	return realcfg->ReadDoubleArray(name, data, range, domain);
}

/**
	Reads an array of char* from the config file.
	@param name the symbolic name of variable
	@param data an array of pointers to char - the pointers must be set to already allocated memory.
	@param max the size of allocated memory for each one string
	@param range the number of items to read
	@param domain the path where variable is stored
	@return true if variable was found and loaded correctly to the data parameter. If false the 'data' parameter is untouched.
*/
bool Config::ReadStringArray(const char* name, char *data[], int max, int range, const char* domain)
{
	return realcfg->ReadStringArray(name, data, max, range, domain);
}

/**
	Reads an array of binary items from the config file.
	@param name the symbolic name of variable
	@param data an array of pointers to char - the pointers must be set to already allocated memory.
	@param length_of_item the size of allocated memory for each one object
	@param range the number of items to read
	@param domain the path where variable is stored
	@return true if variable was found and loaded correctly to the data parameter. If false the 'data' parameter is untouched.
*/
bool Config::ReadHexArray(const char* name, char *data[], int length_of_item, int range, const char* domain)
{
	return realcfg->ReadHexArray(name, data, length_of_item, range, domain);
}


bool Config::ReadString(const char* name,	std::string& data, const char* domain)
{
	return realcfg->ReadString(name, data, domain);
}

bool Config::ReadStringArray(const char* name, std::string data[], int range, const char* domain)
{
	return realcfg->ReadStringArray(name, data, range, domain);
}

void Config::WriteString(const char* name, std::string& data, const char* domain)
{
	realcfg->WriteString(name, data, domain);
}

void Config::WriteStringArray(const char* name, std::string data[], int range, const char* domain)
{
	realcfg->WriteStringArray(name, data, range, domain);
}

