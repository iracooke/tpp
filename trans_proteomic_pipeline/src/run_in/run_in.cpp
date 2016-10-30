//
// DESCRIPTION:
//
// A hack to handle TPP GUI's unixy habit of "cd foo; program.exe arg1 arg2 etc"
// call "run_in foo; program.exe arg1 arg2 etc" to get the same effect in windows
//
// Copyright (c) 2006 Insilicos, LLC
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// 
// $Author: bpratt $
//
//
// NOTES:
//
//
// TODO:
//

#include "common/sysdepend.h"
#include "common/util.h"

int main(int argc, char *argv[]) {
   hooks_tpp hooks(argc,argv); // installdir issues etc 
   size_t len=0;
   for (int i=argc-1;i--;) {
      len+=(strlen(argv[i+1])+1);
   }
   char *cmdline=(char *)calloc(len+1,1);
   for (int j=1;j<argc;j++) {
      strcat(cmdline,argv[j]);
      strcat(cmdline," ");
   }
   cmdline[len-1]=0;
   char *semi = strchr(cmdline,';');
   char *command=NULL;
   int result = -1;
   if (semi) {
      char *dirEnd = semi;
      while ((' '==*(dirEnd-1)) && (dirEnd>cmdline)) {
         dirEnd--;
      }
      semi++;
      while (' '==*semi) {
         semi++;
      }
      command = semi;
      *dirEnd = 0;
   }
   char *slash=findRightmostPathSeperator(cmdline);
   if (slash==cmdline+strlen(cmdline)-1) {
      *slash=0;
   }
   verified_chdir(cmdline);
   if (command) {
#ifdef WINDOWS_NATIVE
	// this is going to be handled by windows shell - fix mingw drive spec if any
	if (('/'==*command) && isalpha(*(command+1)) && ('/'==*(command+2))) {
		*command = *(command+1);
		*(command+1) = ':';
	}
#endif
       result = tpplib_system(command); // run it
   }
   free(cmdline);
   return(result);
}
