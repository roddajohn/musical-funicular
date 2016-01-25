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

static void do_drawing(cairo_t *);
void parse_input(char *buffer);

struct drawing_information {
    int paddle_one_y;
    int paddle_two_y;
    int ball_x;
    int ball_y;
    int score_one;
    int score_two;
} drawing_information;

static struct drawing_information* di = &drawing_information;

static void do_drawing(cairo_t *cr) {

    // Where we draw stuff

    cairo_rectangle(cr, 20, di->paddle_one_y, 7, 40);
    cairo_fill(cr);

    cairo_rectangle(cr, 693, di->paddle_two_y, 7, 40);
    cairo_fill(cr);

    cairo_arc(cr, di->ball_x, di->ball_y, 5, 0, 2*M_PI);
    cairo_fill(cr);

    cairo_stroke(cr);
}


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
    di->ball_x = 360;
    di->ball_y = 240;
    di->paddle_one_y = 0;
    di->paddle_two_y = 0;
    di->score_one = 0;
    di->score_two = 0;

    int pid = fork();
    if (!pid) {
        int width = 720;
        int height = 480;
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

            cairo_stroke(cr);

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
            parse_input(buffer);
            write(to_server, buffer, sizeof(buffer));
        }
    }
}

void parse_input(char *buffer) {
    char *input[10];
    char *temp;

    int i = 0;
    printf("[%s]\n", buffer);

    while(temp = strsep(&buffer, ",")) {
        input[i++] = temp;
    }

    di->ball_x = atoi(input[0]);
    di->ball_y = atoi(input[1]);
    di->paddle_one_y = atoi(input[2]);
    di->paddle_two_y = atoi(input[3]);
    di->score_one = atoi(input[4]);
    di->score_two = atoi(input[5]);

    return;
}
