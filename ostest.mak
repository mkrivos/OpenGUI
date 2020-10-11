
OSTYPE = $(shell uname -s)

ifdef WINDIR
OS := win32
else
OS := unknown
endif

ifeq ($(OSTYPE),GNU/Linux)
OS := linux
endif

ifeq ($(OSTYPE),GNU/linux)
OS := linux
endif

ifeq ($(OSTYPE),Linux)
OS := linux
endif

ifeq ($(OSTYPE),linux)
OS := linux
endif

ifeq ($(OSTYPE),SunOS)
OS := solaris
endif

ifeq ($(OSTYPE),unknown)
$(error Unknown Operating system, try override manually)
endif

#OS := [ linux, solaris, win32 ]