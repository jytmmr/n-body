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
 */
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <GL/glut.h>

/**
 * Constants
 */
#define HEIGHT 600
#define WIDTH 768
#define PI 3.1415926535897932384626433832795

#define G 6.67e-11 // the gravitational constant G
#define DELTA_T 3600 * 24// time slice
#define NUM_BODIES 2 // number of particles
#define THREAD_LIMIT 40
#define ITERATIONS 366
#define TIMER_DELAY 40 //delay for buffer in ms. 40 is about the slowest with minimal lags/jumps. 50 is good but slow
/**
 * Globals
 */
pthread_mutex_t lock;
int current_force_calculation_index = 0;
double bounds;
double largestM;
int currentIteration = 0;

/**
 * Body struct for the program
 */
typedef struct {
	//char name[] = "test";
	double px, py; // cartesian coordinates
	double vx, vy; // velocity
	double fx, fy; // force
	double accel_x, accel_y; // acceleration
	GLfloat red,green,blue;
	double r; // radius
	double mass; // mass
} Body;
/**
 * Global body array
 */
Body bodies[NUM_BODIES];

//below are the function for calculating n-body problem
Body add_force(Body body1, Body body2) {

	double delta_x = body2.px - body1.px;
	double delta_y = body2.py - body1.py;
	double d = sqrt(pow(delta_x, 2) + pow(delta_y, 2)); // distance between the bodies

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
	int numBodies = 2;

	for (int nIndex = 0; nIndex < numBodies; nIndex++) {

		glPopMatrix();
		glColor3f(bodies[nIndex].red,bodies[nIndex].green,bodies[nIndex].blue);
		int num_segments = 50;
		float cx, cy, radius;
		//printf("large x is %f", largestX);
		//printf("large y is %f", largestY);
		//need to make px inside of -1 to 1
		cx = bodies[nIndex].px / bounds;
		cy = bodies[nIndex].py / bounds; //center of the circle
		radius = .05; // (bodies[nIndex].mass / largestM) * .2; // largest mass will have radius of .3, all others scaled down from that
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

	//printf("HI I GOT HERE %d\n", currentIteration);
	//fflush(stdout);

//	printf("%d\n", currentIteration);
	//printf("   bodies[0]: p(%f,%f) with v(%f,%f) a(%f,%f)\n", bodies[0].px,
		//	bodies[0].py, bodies[0].vx, bodies[0].vy, bodies[0].accel_x,
		//	bodies[0].accel_y);
	//printf("   bodies[1]: p(%f,%f) with v(%f,%f) a(%f,%f)\n", bodies[1].px,
		//	bodies[1].py, bodies[1].vx, bodies[1].vy, bodies[1].accel_x,
		//	bodies[1].accel_y);

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
	update_body_positions();
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
void timer(int callBackValue){
	glutPostRedisplay();
	glutTimerFunc(TIMER_DELAY,timer,callBackValue);
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
	pthread_mutex_init(&lock, NULL);

//init the bodies
	Body earth;
	earth.px = 1.496e11;
	earth.py = 0;
	earth.vx = 0;
	earth.vy = 2.980 * pow(10, 4);
	earth.mass = 5.972e24;
	earth.red = .885; //can find color rgb at prideout.net/archive/colors.php
	earth.green = .439;
	earth.blue = .839;

	Body sun;
	sun.px = 0;
	sun.py = 0;
	sun.vx = 0;
	sun.vy = 0;
	sun.mass = 1.989e30;
	sun.red = 1.0;
	sun.green = .871;
	sun.blue = .0;

	bodies[0] = earth;
	bodies[1] = sun;



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
	if (thisLargestX > thisLargestY) {
		bounds = thisLargestX + thisLargestX * .20;
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
	glutDisplayFunc(display); //this is the one continuously called
	timer(0);
	init();
	glColor3f(.7, 0.8, .2);
	glutMainLoop();

	return 0;
}
