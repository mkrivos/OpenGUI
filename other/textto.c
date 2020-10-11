/*
    OpenGUI - Drawing & Windowing library

    Copyright (C) 1996,2002  Marian Krivos

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
    text converting <CR+LF> <-> <LF>

  $Id: textto.c 2086 2005-05-12 10:52:34Z majo $

  $Log$
  Revision 1.1  2005/05/12 10:52:35  majo
  Initial revision

  Revision 1.2  2004/10/22 10:29:40  majo
  *

  Revision 1.1.1.1  2004/01/22 16:25:08  majo
  initial release

*/

#include <stdio.h>

int main(int argc, char **argv)
{
        FILE *in, *out;
        int n=2,c,mode,last,lines=0;
	
	if (argc<=2) 
	{
                printf("Usage: textto [-c -l -t -m] file1, file2 ...\n\a");
		return 1;
	}        
        
	if (argv[1][1]=='l') mode = 0;
        else 
		if (argv[1][1]=='c') mode = 1;
        else 
		if (argv[1][1]=='t') mode = 3;
        else 
		if (argv[1][1]=='m') mode = 2;
	else return -1;
	
	argc--;	
	if (argc<2) return -2;
	while (--argc)
        {
		if (!strcmp(argv[0], argv[n])) { continue; n++; }
                in  = fopen(argv[n], "rb");
		if (mode<3)
		{
    	            out = fopen("__ASD__", "wb");
		if (!in || !out) 
		{
                    perror(argv[n]);
		    exit(-1);
		}
                printf("Converting %s ... \n", argv[n]);
		last = -1;
                while((c=fgetc(in))!=EOF)
                {
			if (c==10) lines++;
			if (mode==0 && c==13) continue;
			if (mode==1 && c==10 && last != 13) 
				if (fputc(13, out) == EOF) return -1;
			if (mode==2 && c == 13) c = 10;
                        if (fputc(c, out) == EOF) return -1;
			last = c;
                }
                fclose(out);
		}
		else while((c=fgetc(in))!=EOF)
                {
			if (c==13) 
			{
				printf("CRLF detected in %s\n", argv[n]);
				break;
			}
                }
                fclose(in);
                if (mode<2) rename("__ASD__", argv[n]);
                n++;
        }
        printf("Done with %d lines\n", lines);
	return 0;
}
