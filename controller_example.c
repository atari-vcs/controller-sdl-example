/*
 * Copyright 2022 Collabora, Ltd.
 *
 * SPDX-License-Identifier: MIT
 */

/* This is a simple SDL controller read example, that can identify the
   Atari VCS' own controllers.

   One thing to be aware of when testing Atari controllers on another
   machine, is that the Classic controller's buttons 2 and 3 can be
   flipped on other machines, compared to the VCS. You will get the
   published mapping, and the correct GameController mapping, on the
   VCS, but you may see BACK and START interchanged if you connect the
   controller to a PC.
*/

#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>

/* These are used to identify the classic controller for special
   treatment: it CAN be opened as a GameController, but to take
   advantage of the twist controls, it must be opened as a Joystick.
*/
#define ATARI_MANUFACTURER_ID  0x3250
#define ATARI_CLASSIC_CONTROLLER_ID 0x1001
#define ATARI_MODERN_CONTROLLER_ID 0x1002

/* This is just simple management of the collection of attached
   controllers, suitable for use in an example only. */
#define MAX_CONTROLLERS 4
#define MAX_JOYSTICKS 4
static SDL_GameController *controllers[MAX_CONTROLLERS];
static int num_controllers;
static SDL_Joystick *joysticks[MAX_JOYSTICKS];
static int num_joysticks;

static int find_joystick(SDL_JoystickID id) {
  int i;
  for( i=0; i<num_joysticks; ++i ) {
    if( SDL_JoystickInstanceID(joysticks[i]) == id ) {
      return i;
    }
  }
  return -1;
}

static int find_controller(SDL_JoystickID id) {
  int i;
  for( i=0; i<num_controllers; ++i ) {
    if( SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controllers[i])) == id ) {
      return i;
    }
  }
  return -1;
}

static const char *hat_to_string(Uint8 value) {
  switch( value ) {
  case SDL_HAT_LEFTUP: return "left-up";
  case SDL_HAT_UP: return "up";
  case SDL_HAT_RIGHTUP: return "right-up";
  case SDL_HAT_LEFT: return "left";
  case SDL_HAT_CENTERED: return "center";
  case SDL_HAT_RIGHT: return "right";
  case SDL_HAT_LEFTDOWN: return "left-down";
  case SDL_HAT_DOWN: return "down";
  case SDL_HAT_RIGHTDOWN: return "right-down";
  }
  return "<invalid value>";
}

int main(int arg __attribute__((unused)), char **argv __attribute__((unused))) {
  /* If you set this to a nonzero value, classic controllers will be opened
     as GameControllers, showing you the default mapping. Otherwise, they will
     be opened as joysticks, so that you can see the twist axis too. 
  */
  int open_classic_as_controller = 0;
  int done;

  num_joysticks = 0;
  num_controllers = 0;
  /* This also initializes the joystick subsystem. */
  if( SDL_Init(SDL_INIT_GAMECONTROLLER) != 0 ) {
    fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  /* Now we scan all attached devices. This simple example doesn't try to account
     for hotplug. Disconnecting and reconnecting controllers isn't handled. 
  */
  {
    int i;
    for( i=0; i<SDL_NumJoysticks(); ++i ) {
      enum {
        OPEN_AS_JOYSTICK,
        OPEN_AS_CONTROLLER
      } mode;
      printf("Scanning attached joystick %d: %s\n", i, SDL_JoystickNameForIndex(i));
      if( SDL_JoystickGetDeviceVendor(i) == ATARI_MANUFACTURER_ID ) {
        /* This is the recommended way to identify the VCS's own
           controllers; the IDs used are given at the top of the file.
        */
        switch( SDL_JoystickGetDeviceProduct(i) ) {
        case ATARI_CLASSIC_CONTROLLER_ID:
          printf("Identified Atari Classic controller\n");
          /* This is just to demonstrate both ways of handling the
             classic controller in a single example; set this value at
             the top of the file to choose between opening as a
             controller or opening as a joystick.
          */
          if( open_classic_as_controller ) {
            mode = OPEN_AS_CONTROLLER;
          } else {
            mode = OPEN_AS_JOYSTICK;
          }
          break;
        case ATARI_MODERN_CONTROLLER_ID:
          printf("Identified Atari Modern controller\n");
          mode = OPEN_AS_CONTROLLER;
          break;
        default:
          fprintf(stderr, "Unknown Atari controller %s, "
                  "please update example for this controller type.\n", SDL_JoystickNameForIndex(i));
          exit(EXIT_FAILURE);
        }
      } else {
        if( SDL_IsGameController(i) ) {
          mode = OPEN_AS_CONTROLLER;
        } else {
          /* You might choose to ignore non-controller joysticks (that
             aren't specifically VCS controllers) in a real
             application, of course. We'll open them in this example,
             because it's fairly easy to do, but you would need your
             own handling for user joystick mapping to do this for
             arbitrary controllers. You don't have that problem for
             the Atari Classic controller, because we have documented
             the mappings.
          */
          mode = OPEN_AS_JOYSTICK;
        }
      }

      if( mode == OPEN_AS_CONTROLLER && num_controllers < MAX_CONTROLLERS ) {
        SDL_GameController *ctrl = SDL_GameControllerOpen(i);
        if( !ctrl ) {
          fprintf(stderr, "Failed to open controller %s: %s\n", SDL_JoystickNameForIndex(i), SDL_GetError());
          exit(EXIT_FAILURE);
        }
        controllers[num_controllers++] = ctrl;
      } else if (mode == OPEN_AS_JOYSTICK && num_joysticks < MAX_JOYSTICKS ) {
        SDL_Joystick *stick = SDL_JoystickOpen(i);
        if( !stick ) {
          fprintf(stderr, "Failed to open joystick %s: %s\n", SDL_JoystickNameForIndex(i), SDL_GetError());
          exit(EXIT_FAILURE);
        }
        joysticks[num_joysticks++] = stick;
      }
    }
  }

  done = 0;
  while(!done) {
    SDL_Event e;
    while( SDL_PollEvent(&e) ) {
      switch( e.type ) {
      case SDL_QUIT:
        done = 1;
        break;
      case SDL_KEYDOWN:
        if( e.key.keysym.sym == SDLK_ESCAPE ) {
          done = 1;
        }
        break;

      case SDL_JOYBUTTONDOWN:
      case SDL_JOYBUTTONUP: {
        const int index = find_joystick(e.jbutton.which);
        if( index >=0 ) {
          printf("%s | button %d %s\n", SDL_JoystickName(joysticks[index]), e.jbutton.button,
                 e.jbutton.type == SDL_JOYBUTTONDOWN ? "DOWN" : "UP");
        }
        break;
      }

      case SDL_JOYAXISMOTION: {
        const int index = find_joystick(e.jaxis.which);
        if( index >=0 ) {
          printf("%s | axis %d value %d\n", SDL_JoystickName(joysticks[index]), e.jaxis.axis, e.jaxis.value);
        }
        break;
      }

      case SDL_JOYHATMOTION: {
        const int index = find_joystick(e.jaxis.which);
        if( index >=0 ) {
          printf("%s | hat %d position %s\n", SDL_JoystickName(joysticks[index]), e.jhat.hat, hat_to_string(e.jhat.value));
        }
        break;
      }

      case SDL_CONTROLLERBUTTONDOWN:
      case SDL_CONTROLLERBUTTONUP: {
        const int index = find_controller(e.cbutton.which);
        if( index >=0 ) {
          printf("%s | button %s %s\n",
                 SDL_GameControllerName(controllers[index]),
                 SDL_GameControllerGetStringForButton(e.cbutton.button),
                 e.cbutton.type == SDL_CONTROLLERBUTTONDOWN ? "DOWN" : "UP");
        }
        break;
      }

      case SDL_CONTROLLERAXISMOTION: {
        const int index = find_controller(e.caxis.which);
        if( index >=0 ) {
          printf("%s | axis %s value %d\n",
                 SDL_GameControllerName(controllers[index]),
                 SDL_GameControllerGetStringForAxis(e.caxis.axis),
                 e.caxis.value);
        }
        break;
      }
      }
    }
  }

  SDL_Quit();
  exit(EXIT_SUCCESS);
}
