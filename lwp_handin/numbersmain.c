/*
 * snake:  This is a demonstration program to investigate the viability
 *         of a curses-based assignment.
 *
 * Author: Dr. Phillip Nico
 *         Department of Computer Science
 *         California Polytechnic State University
 *         One Grand Avenue.
 *         San Luis Obispo, CA  93407  USA
 *
 * Email:  pnico@csc.calpoly.edu
 *
 * Revision History:
 *         $Log: numbersmain.c,v $
 *         Revision 1.5  2023-01-28 14:35:46-08  pnico
 *         Summary: lwp_create() no longer takes a size
 *
 *         Revision 1.4  2023-01-28 14:27:44-08  pnico
 *         checkpointing as launched
 *
 *         Revision 1.3  2013-04-07 12:13:43-07  pnico
 *         Changed new_lwp() to lwp_create()
 *
 *         Revision 1.2  2013-04-02 17:04:17-07  pnico
 *         forgot to include the header
 *
 *         Revision 1.1  2013-04-02 16:39:24-07  pnico
 *         Initial revision
 *
 *         Revision 1.2  2004-04-13 12:31:50-07  pnico
 *         checkpointing with listener
 *
 *         Revision 1.1  2004-04-13 09:53:55-07  pnico
 *         Initial revision
 *
 *         Revision 1.1  2004-04-13 09:52:46-07  pnico
 *         Initial revision
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "lwp.h"

#define MAXSNAKES  100

static void indentnum(void *num);

int main(int argc, char *argv[]){
  int x = 1;
  printf("Launching LWPS\n");

  /* spawn a number of individual LWPs */
  lwp_create((lwpfun)indentnum, &x);
  lwp_start();
  int status;
  lwp_wait(&status);
  printf("Back from LWPS.\n");
  lwp_exit(0);
  return 0;
}

static void indentnum(void *num) {
  printf("hello world");
  printf("%d\n",*(int*)num);
}

