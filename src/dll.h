/*
    OpenGUI - Drawing & Windowing library

    Copyright (C) 1996,2004  Marian Krivos

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    nezmar@atlas.sk

*/

#if defined(_WIN32) || defined(__CYGWIN__)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

//#pragma interface

class Dll {
		const char *name;
		void *handle;
		char *error;
	public:
#if defined( __linux__) || defined (__sun__)
		Dll(const char *, int flag=RTLD_LAZY);
#else
		Dll(const char *);
#endif
		~Dll();
		char * GetResult(void)
		{
			if (handle==0) return error;
			return 0;
		}
		void * GetAddr(const char *);
};
