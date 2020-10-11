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

//
// dynamic loader support object
//

#include <stdio.h>

//#pragma implementation

#include "dll.h"

#if defined(__linux__) || defined (__sun__) || defined (__sun)
Dll::Dll(const char *libname, int flag)
{
	handle = dlopen(name=libname, flag);
#else
Dll::Dll(const char *libname)
{
	handle = (void *)LoadLibrary(name=libname);
#endif
	error = 0;
	if (!handle)
	{
#if defined(__linux__) || defined (__sun__) || defined (__sun)
		fputs(error = dlerror(), stderr);
#endif
		fputs(", cant't link ", stderr);
		fputs(libname, stderr);
		fputs("\n", stderr);
	}
}

Dll::~Dll()
{
	if (handle)
#if defined(__linux__) || defined (__sun__)
		dlclose(handle);
#else
		FreeLibrary((HINSTANCE)handle);
#endif
}

void * Dll::GetAddr(const char *sym)
{
#if defined(__linux__) || defined (__sun__)
	void * addr = dlsym(handle, sym);
	if ((error = dlerror()) != NULL)
#else
	void * addr = (void *)GetProcAddress((HINSTANCE)handle, sym);
	if (addr == NULL)
#endif
	{
		fputs(error, stderr);
		fputc('\n', stderr);
		return (void *)-1;
	}
	return addr;
}

