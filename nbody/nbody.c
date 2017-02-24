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
#define NUM_BODIES 2 // number of particles


typedef struct {
	//char name[] = "test";
	double px, py; //cartesian coordinates
	double vx, vy; //velocity
	double fx, fy; // force
	double accel_x, accel_y;
	double r; // radius
	double mass;
}Body;
Body bodies[NUM_BODIES];
//Body updated_bodies[NUM_BODIES];

Body add_force(Body body1, Body body2){


	
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


	
	return body1;
}

void calculate_updated_velocity(int body_index){
	int k;
	for(k = 0; k < NUM_BODIES; k++){
		if(body_index !=  k){
			bodies[body_index] = add_force(bodies[body_index], bodies[k]);
		}
	}
	
	
	
}


void update_body_positions(){
	int i;
	for(i = 0; i < NUM_BODIES; i++){
		bodies[i].px = bodies[i].px + DELTA_T * bodies[i].vx;
		bodies[i].py = bodies[i].py + DELTA_T * bodies[i].vy;		
	}
}


int main(){


	//init the bodies
	Body earth;
	earth.px = 1.496e11;
	earth.py = 0;

	earth.vx = 0;
	earth.vy = 2.980* pow(10,4);
	earth.mass = 5.972e24;



	Body sun;
	sun.px, sun.py = 0;
	sun.vx, sun.vy = 0;
	sun.mass = 1.989e30;




	bodies[0] = sun;

	bodies[1] = earth;



	int i;
	for(i = 0; i < 366; i++){
		printf("%d\n",i);
		printf("   bodies[0]: p(%f,%f) with v(%f,%f) a(%f,%f)\n",bodies[0].px, bodies[0].py, bodies[0].vx, bodies[0].vy, bodies[0].accel_x, bodies[0].accel_y);
		printf("   bodies[1]: p(%f,%f) with v(%f,%f) a(%f,%f)\n",bodies[1].px, bodies[1].py, bodies[1].vx, bodies[1].vy, bodies[1].accel_x, bodies[1].accel_y);



		int j;
		// likely the for loop that we will thread
		for(j = 0; j < NUM_BODIES; j++){

			calculate_updated_velocity(j);

		}


		update_body_positions();
			
	}

}
