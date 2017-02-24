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

#define G 6.67*pow(10,-11) // the gravitational constant G
#define DELTA_T 3600// time slice

struct Body{
	//char name[] = "test";
	double px, py; //cartesian coordinates
	double vx, vy; //velocity
	double fx, fy; // force
	double r; // radius
	double mass;

};

int main(){
	//printf("HI");


	//init the bodies
	struct Body earth;
	earth.px = 1.496 * pow(10,11);
	earth.py = 0;

	earth.vx = 0;
	earth.vy = 2.980* pow(10,11);
	earth.mass = 5.972 * pow(10,24);



	struct Body sun;
	sun.px, sun.py = 0;
	sun.vx, sun.vy = 0;
	sun.mass = 1.989 * pow(10,30);




	int i;
	for(i = 0; i < 24*365; i++){
		printf("\n");
		printf("   EARTH: p(%f,%f) with v(%f,%f) \n",earth.px, earth.py, earth.vx, earth.vy);
		printf("   SUN: p(%f,%f) with v(%f,%f) \n",sun.px, sun.py, sun.vx, sun.vy);

		double delta_x = sun.px - earth.px;
		double delta_y = sun.py - earth.py;
		double d = sqrt(pow(delta_x,2) + pow(delta_y, 2)); // distance between the bodies

		double F = G*(earth.mass *sun.mass)/ pow(d,2);
		double Fx = F*delta_x / d;
		double Fy = F*delta_y / d;

		double ax1 = Fx / earth.mass;
		double ay1 = Fy/ earth.mass;
		double ax2 = Fx / sun.mass;
		double ay2 = Fy/ sun.mass;

		earth.vx = earth.vx + DELTA_T * ax1;
		earth.vy = earth.vy + DELTA_T * ay1;

		earth.px = earth.px + DELTA_T * earth.vx;
		earth.py = earth.py + DELTA_T * earth.vy;

		sun.vx = sun.vx + DELTA_T * ax2;
		sun.vy = sun.vy + DELTA_T * ay2;

		sun.px = sun.px + DELTA_T * sun.vx;
		sun.py = sun.py + DELTA_T * sun.vy;
	}

}
