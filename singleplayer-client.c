#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cairo.h>
#include <math.h>
#include <SDL.h>
#include <sys/shm.h>
#include <sys/ipc.h>

typedef struct drawing_information {
  int paddle_one_y;
  int paddle_two_y;
  float ball_speed;
  float ball_x;
  float ball_y;
  int score_one;
  int score_two;
} drawing_information;

char *parse_input(char *buffer, drawing_information *);

int client_handshake(int *from_server) {
    int to_server;
    char buffer[100];

    // Create a private named pipe
    // Use the pid as a unique pipe name
    sprintf(buffer, "%d", getpid());
    mkfifo(buffer, 0644);

    // Opens the well-known pipe to the server
    // This sends the name of the pipe to the server
    to_server = open("mario", O_WRONLY);
    write(to_server, buffer, sizeof(buffer));

    // Reads a message from the server
    *from_server = open(buffer, O_RDONLY);
    read(*from_server, buffer, sizeof(buffer));

    // Prints out the message
    printf("<Client> Connect message: [%s]\n", buffer);

    return to_server;
}

int main(int argc, char *argv[]) {
    int pid = fork();
    if (!pid) {

      drawing_information dinformation;
      drawing_information* di = &dinformation;

      di->ball_x = 360;
      di->ball_y = 240;
      di->paddle_one_y = 0;
      di->paddle_two_y = 0;
      di->score_one = 0;
      di->score_two = 0;
      di->ball_speed = 1;

      int shmid;
      key_t key = 25568;

      if ((shmid = shmget(key, sizeof(drawing_information), IPC_CREAT | 0666)) < 0) {
	printf("Error shmgetting...\n");
	exit(1);
      }
      
      int width = 720;
      int height = 520;
      int videoFlags = SDL_SWSURFACE | SDL_RESIZABLE | SDL_DOUBLEBUF;
      int bpp = 32;
      
      if (SDL_Init(SDL_INIT_VIDEO) < 0) {
	return -1;
      }
      
      SDL_WM_SetCaption("Pong", "Pong");
      
      SDL_Surface *screen;
      screen = SDL_SetVideoMode(width, height, bpp, videoFlags);
      
      SDL_EnableKeyRepeat(300, 130);
      SDL_EnableUNICODE(1);
      
      SDL_Surface *sdl_surface = SDL_CreateRGBSurface ( 
						       videoFlags, width, height, 32, 
						       0x00ff0000, 
						       0x0000ff00, 
						       0x000000ff, 
						       0
							);
      
      int done = 0;
      
      while (!done) {
	di = (drawing_information *)shmat(shmid, NULL, 0);
	
	SDL_FillRect(sdl_surface, NULL, 0);
	
	cairo_surface_t *cairo_surface = cairo_image_surface_create_for_data((unsigned char *)sdl_surface->pixels, CAIRO_FORMAT_RGB24, sdl_surface->w, sdl_surface->h, sdl_surface->pitch);
	
	cairo_t *cr = cairo_create(cairo_surface);
	
	//do_drawing(cr);
	cairo_set_source_rgb(cr,255,255,255);
	cairo_rectangle(cr, 20, di->paddle_one_y, 7, 40);
	cairo_fill(cr);
	
	cairo_rectangle(cr, 693, di->paddle_two_y, 7, 40);
	cairo_fill(cr);
	
	cairo_arc(cr, di->ball_x, di->ball_y, 5, 0, 2*M_PI);
	cairo_fill(cr);

	cairo_move_to(cr, 0, 480);
	cairo_line_to(cr, 720, 480);

	cairo_stroke(cr);
	
	cairo_select_font_face(cr, "Courier",
			       CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_BOLD);

	cairo_set_font_size(cr, 38);

	cairo_move_to(cr, 20, 510);
	char *to_send = (char *)malloc(100);
	sprintf(to_send,
	  "%d",
	  di->score_one);

	cairo_show_text(cr, to_send);

	cairo_move_to(cr, 693, 510);
	sprintf(to_send,
		"%d",
		di->score_two);

	cairo_show_text(cr, to_send); 
		
	SDL_BlitSurface(sdl_surface, NULL, screen, NULL);
	SDL_Flip(screen);
	
	//dont with cairo surface
	cairo_surface_destroy(cairo_surface);
	cairo_destroy(cr);
	
	//Event handling
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
	  switch(event.type) {
	  case SDL_KEYDOWN:
	    if (event.key.keysym.sym == SDLK_ESCAPE) {
	      done = 1;
	    }
	    else if (event.key.keysym.sym == SDLK_w) {
	      if (di->paddle_one_y > 15) {
		di->paddle_one_y -= 15;
	      }
	    }
	    else if (event.key.keysym.sym == SDLK_s) {
	      if (di->paddle_one_y < 410) {
		di->paddle_one_y += 15;
	      }
	    }
	    else if (event.key.keysym.sym == SDLK_UP) {
	      if (di->paddle_two_y > 15) {
		di->paddle_two_y -= 15;
	      }
	    }
	    else if (event.key.keysym.sym == SDLK_DOWN) {
	      if (di->paddle_two_y < 410) {
		di->paddle_two_y += 15;
	      }
	    }
	    break;
	  case SDL_QUIT:
	    done = 1;
	    break;
	  }
	}
	
	SDL_Delay(1);
	
      }
      
      //cleanup
      SDL_FreeSurface(sdl_surface);
      SDL_Quit();
      
      return 0;
      
    }
    else {
      drawing_information dinformation;
      drawing_information* di = &dinformation;

      di->ball_x = 360;
      di->ball_y = 240;
      di->paddle_one_y = 0;
      di->paddle_two_y = 0;
      di->score_one = 0;
      di->score_two = 0;

      int to_server;
      int from_server;
      char buffer[100];
      
      to_server = client_handshake(&from_server);
      
      while (1) {
	// Reading loop
	
	// Take input from user
	
	// Write to server
	
	// If the message is exit, then shut down
	// write(to_server, buffer, sizeof(buffer));
	
	// Get message from server
	read(from_server, buffer, sizeof(buffer));
	// Print confirmation
	char *to_send = parse_input(buffer, di);
	write(to_server, to_send, 100);
      }
    }
}

char *parse_input(char *buffer, drawing_information *di) {
  char *input[10];
  char *temp;
  
  int i = 0;

  while(temp = strsep(&buffer, ",")) {
    input[i++] = temp;
  }

  int shmid;
  key_t key = 25568;
  
  if ((shmid = shmget(key, sizeof(drawing_information), IPC_CREAT | 0666)) < 0) {
    printf("Error shmgetting...\n");
    exit(1);
  }
  
  if ((di = shmat(shmid, NULL, 0)) == (drawing_information *) -1) {
    printf("Error shmatting...\n");
    exit(1);
  }
  
  di->ball_x = atof(input[0]);
  di->ball_y = atof(input[1]);
  di->ball_speed = atof(input[4]);
  di->score_one = atoi(input[2]);
  di->score_two = atoi(input[3]);

  //printf("[%d][%d]\n", di->paddle_one_y, di->paddle_two_y);
  char *to_send = (char *)malloc(100);
  sprintf(to_send,
	  "%d,%d",
	  di->paddle_one_y,
	  di->paddle_two_y);
  //printf("to_send: [%s]\n", to_send);
  
  return to_send;
}
