/*
 * nbody.c
 *
 *  Created on: Feb 23, 2017
 *      Author: Jay Timmer, Jacob Van Tol, Tim Warntjes
 */

// delta t is measured in seconds
/**
 * Included headers
 * Note <GL/glut.h> may not be included by default on Windows C installations
	To compile on linux:
	gcc -o nbody nbody.c -lpthread -lGL -lGLU -lglut -lm
 */
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <GL/glut.h>
#include <string.h>
#include <float.h>

/**
 * Constants
 */
#define HEIGHT 1080
#define WIDTH 1080
#define PI 3.1415926535897932384626433832795

#define G 6.67e-11 // the gravitational constant G
#define DELTA_T 3600* 240// time slice
#define THREAD_LIMIT 40

#define TIMER_DELAY 50 //delay for buffer in ms. 40 is about the slowest with minimal lags/jumps. 50 is good but slow
#define MAX_NAME_SIZE 20
/**
 * Globals
 */
pthread_mutex_t lock;
//pthread_mutex_t shortestLock;
int current_force_calculation_index = 0;
double bounds;
double largestM;
int currentIteration = 0;
int NUM_BODIES; // number of particles
int ITERATIONS;
//double shortestDistance = 1e10;
/**
 * Body struct for the program
 */
typedef struct {
	char name[MAX_NAME_SIZE];
	double px, py; // cartesian coordinates
	double vx, vy; // velocity
	double accel_x, accel_y; // acceleration
	GLfloat red, green, blue;
	double r; // radius
	double mass; // mass
} Body;
/**
 * Global body array
 */
Body *bodies;

//below are the function for calculating n-body problem
Body add_force(Body body1, Body body2) {

	double delta_x = body2.px - body1.px;
	double delta_y = body2.py - body1.py;
	double d = sqrt(pow(delta_x, 2) + pow(delta_y, 2)); // distance between the bodies
	
	/*
	pthread_mutex_lock(&shortestLock);
	if(d < shortestDistance)
	{
		shortestDistance = d;
	}
	pthread_mutex_unlock(&shortestLock);
	*/

	double F = G * (body2.mass * body1.mass) / pow(d, 2);
	double Fx = F * delta_x / d;
	double Fy = F * delta_y / d;

	body1.accel_x = Fx / body1.mass;
	body1.accel_y = Fy / body1.mass;
	
	
	body1.vx = body1.vx + DELTA_T * body1.accel_x;
	body1.vy = body1.vy + DELTA_T * body1.accel_y;

	return body1;
}

void* calculate_updated_velocities() {
	int k;
	int my_force_calculation_index;
	pthread_mutex_lock(&lock);
	my_force_calculation_index = current_force_calculation_index;
	current_force_calculation_index++;
	pthread_mutex_unlock(&lock);

	for (k = 0; k < NUM_BODIES; k++) {
		if (k != my_force_calculation_index) {
			bodies[my_force_calculation_index] = add_force(
					bodies[my_force_calculation_index], bodies[k]);
		}
	}
	return 0;
}

void update_body_positions() {
	int i;
	for (i = 0; i < NUM_BODIES; i++) {
		
		
		bodies[i].px = bodies[i].px + DELTA_T * bodies[i].vx;
		bodies[i].py = bodies[i].py + DELTA_T * bodies[i].vy;
	}

}

//above are the threaded functions for the n-body problem
//below are the openGL functions

/**
 * drawCircle draws the n=bodies
 */
void drawCircle(void) {
	int numBodies = NUM_BODIES;

	for (int nIndex = 0; nIndex < numBodies; nIndex++) {

		glPopMatrix();
		glColor3f(bodies[nIndex].red, bodies[nIndex].green,
				bodies[nIndex].blue);
		int num_segments = 50;
		float cx, cy, radius;
		//printf("large x is %f", largestX);
		//printf("large y is %f", largestY);
		//need to make px inside of -1 to 1
		cx = (bodies[nIndex].px / bounds) ;
		cy = (bodies[nIndex].py / bounds) ; //center of the circle
		radius = bodies[nIndex].r;
		//if(radius<.01)

		//printf("For n-body %i, cx is %f and cy is %f)\n", nIndex, cx, cy);
		glBegin(GL_TRIANGLE_FAN);
		for (int i = 0; i < num_segments; i++) {
			float theta = 2.0 * 3.1415926 * i / num_segments;
			float x = radius * cosf(theta);
			float y = radius * sinf(theta);
			glVertex2f(x + cx, y + cy);
		}
		glEnd();
		glPushMatrix();
	}

}

/**
 * This function advances the program appropriately and and redraws as needed
 */
void advanceProgram() {


	//printf("%d\n", currentIteration);
	//printf("   bodies[0]: p(%f,%f) with v(%f,%f) a(%f,%f)\n", bodies[1].px,
		//bodies[1].py, bodies[1].vx, bodies[1].vy, bodies[1].accel_x,
		//bodies[1].accel_y);
	//printf("   bodies[1]: p(%f,%f) with v(%f,%f) a(%f,%f)\n", bodies[5].px,
		//bodies[5].py, bodies[5].vx, bodies[5].vy, bodies[5].accel_x,
		//bodies[5].accel_y);

	//Index that is sent to each thread telling it which body to update
	current_force_calculation_index = 0;
	pthread_t threads[NUM_BODIES];
	if (NUM_BODIES < THREAD_LIMIT) {
		int j;
		for (j = 0; j < NUM_BODIES; j++) {
			pthread_create(&threads[j], NULL, calculate_updated_velocities,
			NULL);
		}
		for (j = 0; j < NUM_BODIES; j++) {
			pthread_join(threads[j], NULL);
		}
	}		//else{
//			int j;
//			for(j = 0; j < NUM_BODIES; j++){
//
//				pthread_create(&threads[j], NULL, calculate_updated_velocity, (void *) &bodies[j] );
//			}
//
//			for(j = 0; j < NUM_BODIES; j++){
//				pthread_join(threads[j], NULL);
//			}
//		}

	//double tScale = DELTA_T * (shortestDistance /1e10);
	update_body_positions();
	//shortestDistance = 1e10;
	currentIteration++; 
}

/**
 * Display function is the function called byt the glMainLoop and the redisplay loop, it
 * calls draw circle, which draws the n-bodies
 */
void display(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix(); //saves current state
	drawCircle();
	glPopMatrix();
	glutSwapBuffers();
	advanceProgram();
	//without the timer function, glutPostRedisplay() goes here
	//but with timer for smoother animation it goes in timer function, timer is called in main
}

/**
 * Timer function that dictates fastest rendering for consistent, smooth animation
 */
void timer(int callBackValue) {
	glutPostRedisplay();
	glutTimerFunc(TIMER_DELAY, timer, callBackValue);
}

/**
 * Init function sets up the open GL
 */
void init(void) {
	glEnable(GL_DEPTH_TEST); //makes sure to show closer figure in z-space, not just last drawn
//glCullFace(GL_BACK);
//glEnable(GL_CULL_FACE); // these are for drawing - otherwise clockwise leads to back facing image, these make that not happen

	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat) WIDTH / (GLfloat) HEIGHT, .1f, 100000.0f);
	glMatrixMode(GL_MODELVIEW);		// default is GL_MODELVIEW, try GL_TEXTURE
	glLoadIdentity();
	glClearColor(1.0, 1.0, 1.0, 0.0);
}

int main(int argc, char **argv) {
	//init the mutex for threading
	pthread_mutex_init(&lock, NULL);
	//pthread_mutex_init(&shortestLock,NULL);

	//read from file for the data
	FILE *file;
	file = fopen("nbodyData.txt", "r");
	/*
	if (file == NULL) {
		int errno;
		perror("Error opening file");
		printf("Error code opening file: %d\n", errno);
		printf("Error opening file: %s\n", strerror( errno));
		exit(-1);
	}
	*/

	//get the number of bodies and create the array
	fscanf(file, "%d", &NUM_BODIES);
	bodies = malloc(sizeof(*bodies) * NUM_BODIES);
	if (bodies == NULL) {
		printf("bodies allocation failed");
	}

	//read in the number of iterations to do
	fscanf(file, "%d", &ITERATIONS);

	//read the nbody data into the body objects
	for (int i = 0; i < NUM_BODIES; i++) {
		fscanf(file, "%s", &bodies[i].name[0]);	//if name > 20 characters, errors will happen
		fscanf(file, "%lf", &bodies[i].px);
		fscanf(file, "%lf", &bodies[i].py);
		fscanf(file, "%lf", &bodies[i].vx);
		fscanf(file, "%lf", &bodies[i].vy);
		fscanf(file, "%lf", &bodies[i].mass);
		fscanf(file, "%f", &bodies[i].red);
		fscanf(file, "%f", &bodies[i].green);
		fscanf(file, "%f", &bodies[i].blue);
		fscanf(file, "%lf", &bodies[i].r);
	}

	//find the largest x and Y, in order to have an appropriate window to view
	//the model through openGL
	double thisLargestX = 0;
	double thisLargestY = 0;
	double thisLargestM = 0;
	for (int i = 0; i < NUM_BODIES; i++) {
		if (thisLargestX < bodies[i].px) {
			thisLargestX = bodies[i].px;
		}
		if (thisLargestY < bodies[i].py) {
			thisLargestY = bodies[i].py;
		}
		if (thisLargestM < bodies[i].mass) {
			thisLargestM = bodies[i].mass;
		}
		
	}

	//set globals for mass and coordinates
	//onenGL fits things into a -1 to 1 plane, which can be adjusted but
	//it is easier to just fit everything into that plane
	//so we later divide all numbers by the largest to give a value between 0 and 1
	if (thisLargestX > thisLargestY) {
		bounds = thisLargestX + thisLargestX * .20;	//add 20% because max radius is .20 and largest possible is 1
	} else {
		bounds = thisLargestY + thisLargestY * .20;
	}
	largestM = thisLargestM;

//init the openGL stuff
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(10, 10);

	glutCreateWindow("N-Body");
	glutDisplayFunc(display); //this is the one continuously called, calls our nbody functions
	timer(0);
	init();
	glColor3f(.7, 0.8, .2);
	glutMainLoop();

	free(bodies);
	return 0;
}
