#ifndef __SENSE_HAT__
#define __SENSE_HAT__

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <time.h>
#include <poll.h>
#include <dirent.h>
#include <linux/input.h>
#include <linux/fb.h>

typedef struct
 {
  struct pollfd evpoll;
  int           descripteurDalle;
  uint16_t*     adresseDuMappage;
 }
 SenseHat;

SenseHat* SenseHat_creer                 (             );
void      SenseHat_detruire              ( SenseHat* s );
int       SenseHat_recupererEtatJoystick ( SenseHat* s );
void      SenseHat_modifierEtatLED       ( SenseHat* s ,
                                           int       x ,
                                           int       y ,
                                           int       r , 
                                           int       v ,
                                           int       b );
void      SenseHat_initialiserDalle      (             );

#endif
