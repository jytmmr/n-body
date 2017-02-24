/*
 * nbody.c
 *
 *  Created on: Feb 23, 2017
 *      Author: Tim Warntjes
 */

// delta t is measured in seconds

#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define G 6.67e-11 // the gravitational constant G
#define DELTA_T 3600 * 24// time slice

struct Body{
	//char name[] = "test";
	double px, py; //cartesian coordinates
	double vx, vy; //velocity
	double fx, fy; // force
	double accel_x, accel_y;
	double r; // radius
	double mass;
};


struct Body computePosition(struct Body body1, struct Body body2){
	
	double delta_x = body2.px - body1.px;
	double delta_y = body2.py - body1.py;
	double d = sqrt(pow(delta_x,2) + pow(delta_y, 2)); // distance between the bodies

	double F = G*(body2.mass * body1.mass)/ pow(d,2);
	double Fx = F*delta_x / d;
	double Fy = F*delta_y / d;

	body1.accel_x = Fx / body1.mass;
	body1.accel_y = Fy/ body1.mass;

	body1.vx = body1.vx + DELTA_T * body1.accel_x;
	body1.vy = body1.vy + DELTA_T * body1.accel_y;

	body1.px = body1.px + DELTA_T * body1.vx;
	body1.py = body1.py + DELTA_T * body1.vy;
	
	return body1;
}
int main(){


	//init the bodies
	struct Body earth;
	earth.px = 1.496e11;
	earth.py = 0;

	earth.vx = 0;
	earth.vy = 2.980* pow(10,4);
	earth.mass = 5.972e24;



	struct Body sun;
	sun.px, sun.py = 0;
	sun.vx, sun.vy = 0;
	sun.mass = 1.989e30;




	int i;
	for(i = 0; i < 365; i++){
		printf("%d\n",i);
		printf("   EARTH: p(%f,%f) with v(%f,%f) \n",earth.px, earth.py, earth.vx, earth.vy);
		printf("   SUN: p(%f,%f) with v(%f,%f) \n",sun.px, sun.py, sun.vx, sun.vy);
		struct Body tempEarth = earth;
		earth = computePosition(earth, sun);
		sun = computePosition(sun, tempEarth);
		

	}

}
