#define EXTERN extern

#include <stdlib.h>
#include <stdio.h>

#include <termios.h>
#include <string.h>

#include "ir2.h" 

/*-----------------------------------------------------------------------
**   set_keypress() - setup for non-block. Return current setting for reset.
** reset_keypress() - reset(restores)  settings.
**
**   These function allow you to put stdin in a non-blocking mode so you
**   can read 1 charater at a time.
**   Got the code from unix.programmer.faq.
**------------------------------------------------------------------------
*/

void set_keypress(struct termios *term )
{
    struct termios new;
 
    tcgetattr(0, term);
 
    memcpy(&new, term, sizeof(struct termios));
 
    /* Disable canonical mode, and set buffer size to 1 byte */
    new.c_lflag &= (~ICANON);
    new.c_cc[VTIME] = 0;
    new.c_cc[VMIN] = 0;
 
    tcsetattr(0,TCSANOW,&new);
    return;
}
 
void reset_keypress( struct termios *term )
{
    tcsetattr(0,TCSANOW, term);
    return;
}

