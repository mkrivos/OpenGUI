/*
  $Id: tinycfg.h 4679 2006-04-05 13:16:35Z majo $
*/

#if !defined(XMLCONFIG)
#define XMLCONFIG

#include <string>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "tinyxml.h"

#define XMLCFG_MAJOR			2
#define XMLCFG_MINOR			0

/**
	Base config interface to implement.
*/
class ConfigInterface
{
	public:
		ConfigInterface() {}
		virtual ~ConfigInterface() {}

		void Sync(void) { Save(); }
		virtual bool IsValid() = 0;
		virtual bool Load(const char* fname) = 0;
		virtual bool Save(void) = 0;

		virtual bool ReadBool(const char * name, bool &data, const char* domain = 0) = 0;
		virtual bool ReadInt(const char * name, int &data, const char* domain = 0) = 0;
		virtual bool ReadString(const char * name,	char *data, int max, const char* domain = 0) = 0;
		virtual bool ReadDouble(const char * name, double &data, const char* domain = 0) = 0;
		virtual bool ReadHex(const char * name, char *data, int length_of_item, const char* domain = 0) = 0;

		virtual bool ReadBoolArray(const char * name, bool data[], int range, const char* domain = 0) = 0;
		virtual bool ReadIntArray(const char * name, int data[], int range, const char* domain = 0) = 0;
		virtual bool ReadStringArray(const char * name, char* data[], int max, int range, const char* domain = 0) = 0;
		virtual bool ReadDoubleArray(const char * name, double data[], int range, const char* domain = 0) = 0;
		virtual bool ReadHexArray(const char * name, char* data[], int length_of_item, int range, const char* domain = 0) = 0;

		virtual void WriteBool(const char * name, bool data, const char* domain = 0) = 0;
		virtual void WriteInt(const char * name, int data, const char* domain = 0) = 0;
		virtual void WriteString(const char * name, const char* data, const char* domain = 0) = 0;
		virtual void WriteDouble(const char * name, double data, const char* domain = 0) = 0;
		virtual void WriteHex(const char * name, char* data, int length_of_item, const char* domain = 0) = 0;

		virtual void WriteBoolArray(const char * name, bool data[], int range, const char* domain = 0) = 0;
		virtual void WriteIntArray(const char * name, int data[], int range, const char* domain = 0) = 0;
		virtual void WriteStringArray(const char * name, const char *data[], int range, const char* domain = 0) = 0;
		virtual void WriteDoubleArray(const char * name, double data[], int range, const char* domain = 0) = 0;
		virtual void WriteHexArray(const char * name, char *data[], int length_of_item, int range, const char* domain = 0) = 0;
		
		virtual bool ReadString(const char * name,	std::string& data, const char* domain = 0) = 0;
		virtual bool ReadStringArray(const char * name, std::string data[], int range, const char* domain = 0) = 0;
		virtual void WriteString(const char * name, std::string& data, const char* domain = 0) = 0;
		virtual void WriteStringArray(const char * name, std::string data[], int range, const char* domain = 0) = 0;	
};

/**
	@defgroup Config Configuration file support
	Why XML Persistence, Anyway?

	If you are allergic towards industry buzzwords (like myself), you probably
	think XML is for consultants and "enterprise developers" (whatever that means),
	not for Real Programmers (like yourself). Still, I think XML is a Good Thing (TM),
	and is suitable not only for some multi-million dollar project with 5000 pages
	of design documents filled with UML and Booch notation, but also for the config
	files of your little pet project you're developing in your spare time, or for
	the document format of the application you're working on to in your [non-spare?!?] time.
	Here are a few reasons why I like XML:

	- XML files are easy to view and easy to edit with a simple text editor - anything
	  from Notepad to emacs will do. There are also several freely available XML editors,
	  which make the task even easier - check out the "Buyer's Guide" section at XML.com.
	  This means it will be easier to debug your save/load code. Ever had the problem when
	  you write your save and load routines, and saving and loading a file definitely
	  doesn't produce the same memory objects you started with? You have no idea whether
	  the bug is in the save or the load routine, and spend 1 hour tracing the correct one.
	  Not with XML: one look at the XML file itself can tell you if it's correct.
	- XML files are open - anyone can read your XML files, process them, convert
	  them with a XML parser or a few lines of Perl code. (If you still haven't picked
	  up Perl, go ahead and spend 2-3 days to learn it. You'll thank me.) That "anyone"
	  includes yourself after six months - if you're like me, after 6 months you won't
	  know anything about the code you're happily hacking at today, including how to read
	  its data files. You can find XML parsers for all kinds of environments and programming languages.
	- XML files are high-level - the parser and the support lib (something like tinyxml)
	  will take care of handling primitive data types like numbers and strings, and you'll
	  never have the joy again of having to store the string length before the string
	  itself (brrr...) Still, XML files are more compact than most high-level alternatives
	  (I've used MS OLE IPropertyBags, stored in OLE structured storage files, which definitely
	  incur more overhead).
	- Disk space is ultra-cheap today, so the overhead of XML compared to binary files
	  can be safely ignored in most cases. (In the rest of the cases, you're looking
	  at more data than you can handle without a database engine.)
	- XML is simple to use. The special case of using XML for object persistence
	  is made even more simple with tinycfg.
	@{

*/

/**
	The wrapper to hide implementation details (and bridge between an old config file format).
	From the version of the OpenGUI 5.0.2 up, is Config file provided by separate
	library called 'tinycfg'. It is based on LGPL library from Lee Thomason
	and Yves Berquincalled - 'tinyxml'.
	TINYXML HOMEPAGE: www.sourceforge.net/projects/tinyxml

	This object saves and restores variables or arrays of basic types to XML file.
	There are possibilities to write all primitive data types:
	- int
	- double
	- string (aka ASCIIZ)
	- bool
	- and raw binary data up to 512 bytes length)

	You can write all these data types as ordinary variable
	or as an array of of one. I.e. you can write entire array
	of doubles within one line!

	@code
	config->WriteDoubleArray("an_array", array_of_doubles, sizeof(array_of_doubles)/sizeof(double) );
	@endcode

	One big plus of this implementation is that uses 'namespaces' or 'domain'
	to save one variable name from other to colide:

	@code
	bool foo = true;
	config->WriteBool("foo", foo);
	foo = false;
	config->WriteBool("foo", foo, "foo.bar");

	generated file:

	<?xml version="1.0" standalone="no" ?>
	<tinycfg>
		<bool name="foo" val="true" />
		<foo>
			<bar>
				<bool name="foo" val="false" />
			</bar>
		</foo>
	</tinycfg>

	@endcode
	These two writes uses its own namespaces. The default namespace
	is 0 (no parameter) for the root of config file.

	@see XML
*/
class Config : public ConfigInterface
{
		ConfigInterface* realcfg;
	public:
		Config(bool write = true);
		Config(const char *, bool write = true);
		virtual ~Config();

		virtual bool IsValid();
		virtual bool Load(const char* fname);
		virtual bool Save(void);

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
};

/**
	@}
*/

#endif // Unit1_H
