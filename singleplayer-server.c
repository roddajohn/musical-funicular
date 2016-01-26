#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

void server_function(int from_client, int to_client) {
  char line[100];
  int dx = 1;
  int dy = 0;
  struct drawing_information {
    int paddle_one_y;
    int paddle_two_y;
    float ball_speed;
    float ball_x;
    float ball_y;
    int score_one;
    int score_two;
  } drawing_information;
  
  drawing_information.ball_x = 360;
  drawing_information.ball_y = 240;
  drawing_information.paddle_one_y = 0;
  drawing_information.paddle_one_y = 0;
  drawing_information.paddle_two_y = 0;
  drawing_information.score_one = 0;
  drawing_information.score_two = 0;
  drawing_information.ball_speed = 1;

  while (1) {
    char line_to_write[100];
    sprintf(line_to_write,
            "%f,%f,%d,%d,%f", 
            drawing_information.ball_x, 
            drawing_information.ball_y, 
            drawing_information.score_one, 
            drawing_information.score_two,
	    drawing_information.ball_speed
            );
    //    printf("To Client[%s]\n",line_to_write);
    
    write(to_client, line_to_write, sizeof(line_to_write));
    
    read(from_client, line, 100);
    
    //    printf("From Client[%s]\n", line);

    char *temp;
    char *input[2];
    
    int i = 0;
    char *in = line;
    while(temp = strsep(&in, ",")) {
      input[i++] = temp;
    }

    drawing_information.paddle_one_y = atoi(input[0]);
    drawing_information.paddle_two_y = atoi(input[1]);

    drawing_information.ball_x += dx*drawing_information.ball_speed;
    drawing_information.ball_y += dy*drawing_information.ball_speed;

    float x = drawing_information.ball_x;
    float y = drawing_information.ball_y;

    if (y < 6) {
      dy *= -1;
    }
    else if (y > 470) {
      dy *= -1;
    }
    if (x < 32) {
      if (y < drawing_information.paddle_one_y || y > (drawing_information.paddle_one_y + 40)) {
	drawing_information.score_two++;
	drawing_information.ball_x = 360;
	drawing_information.ball_y = 240;
	dx = 1;
	dy = 0;
      }
      else if (y > (drawing_information.paddle_one_y + 20)) {
	dx = 1;
	dy = 1;
      }
      else if (y < (drawing_information.paddle_one_y + 20)) {
	dx = 1;
	dy = -1;
      }
      else {
	dx = 1;
	dy = 0;
      }
    }
    else if (x > 688) {
      if (y < drawing_information.paddle_two_y || y > (drawing_information.paddle_two_y + 40)) {
	drawing_information.score_one++;
	drawing_information.ball_x = 360;
	drawing_information.ball_y = 240;
	dx = -1;
	dy = 0;
      }
      else if (y > (drawing_information.paddle_two_y + 20)) {
	dx = -1;
	dy = 1;
      }
      else if (y < (drawing_information.paddle_two_y + 20)) {
	dx = -1;
	dy = -1;
      }
      else {
	dx = -1;
	dy = 0;
      }
    }
    
    usleep(10000);
  }
}

void server_connection_protocol(int *from_client) {
  int to_client;
  char line[100];
  
  // Loop to handle the client connections
  
  while (1) {
    // Create a Well Known Pipe
    mkfifo("mario", 0644);
    
    // Waits for a connection
    *from_client = open("mario", O_RDONLY);
    
    // Recieve message from client
    read(*from_client, line, sizeof(line));
    printf("<Server> Got connection request: [%s]\n", line);
    
    // Remove well known pipe
    remove("mario");
    
    // Fork a process to deal with this client
    int forkin = fork();
    
    if (forkin == 0) {
      // Connect to the client pipe
      to_client = open(line, O_WRONLY);				
      
      // Send acknowledgement message
      strncpy(line, "it's-a-me, mario!", sizeof(line));
      write(to_client, line, sizeof(line));
      
      // Call the server_function to handle the client
      server_function(*from_client, to_client);
      exit(1);
    }
  }
}

void sighandler(int signum) {
  printf("Closing nicely...\n");
  remove("mario");
  exit(0);
}

int main() {
  signal(2, sighandler);

  int to_client;
  int from_client;
  
  char line[100];
  
  server_connection_protocol(&from_client);
}
