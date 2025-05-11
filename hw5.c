#include "elevator.h"

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static struct passenger {
    int from_floor;
    int to_floor;
} Passenger[PASSENGERS];

int passenger_ready_for_pickup[PASSENGERS];
int passenger_in_elevator[PASSENGERS];
int passenger_exited_elevator[PASSENGERS];

int elevator_at_pickup[PASSENGERS];
int elevator_at_destination[PASSENGERS];

static pthread_mutex_t lock;
static pthread_cond_t  cond;

int serving_elevator[PASSENGERS];
int trips_remaining;

void reset_flags(int i)
{
    passenger_ready_for_pickup[i] = 0;
    passenger_in_elevator[i] = 0;
    passenger_exited_elevator[i] = 0;
    elevator_at_pickup[i] = 0;
    elevator_at_destination[i] = 0;
    serving_elevator[i] = -1;
}

void initializer(void)
{
    // initialize some data structures
    memset(&Passenger, 0x00, sizeof(Passenger));
    for (int i = 0; i < PASSENGERS; i++)
        reset_flags(i);
    trips_remaining = PASSENGERS * TRIPS_PER_PASSENGER;

    // initialize mutex and coniditon
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init (&cond, NULL);
}


void passenger_controller(int passenger, int from_floor, int to_floor,
                       void (*enter_elevator)(int, int), void (*exit_elevator)(int, int))
{
    pthread_mutex_lock(&lock);

    // inform elevator of floor
    Passenger[passenger].from_floor = from_floor;
    Passenger[passenger].to_floor = to_floor;

    reset_flags(passenger);

    // signal ready and wait
    passenger_ready_for_pickup[passenger] = 1;
    pthread_cond_broadcast(&cond);

    while (!elevator_at_pickup[passenger])
        pthread_cond_wait(&cond, &lock);
    int curr_serving_elevator = serving_elevator[passenger];
    pthread_mutex_unlock(&lock);

    // enter elevator and wait
    enter_elevator(passenger, curr_serving_elevator);
    pthread_mutex_lock(&lock);

    passenger_in_elevator[passenger] = 1;
    pthread_cond_broadcast(&cond);

    while (!elevator_at_destination[passenger])
        pthread_cond_wait(&cond, &lock);
    pthread_mutex_unlock(&lock);

    // exit elevator and signal
    exit_elevator(passenger, curr_serving_elevator);
    pthread_mutex_lock(&lock);

    passenger_exited_elevator[passenger] = 1;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&lock);
}


// example procedure that makes it easier to work with the API
// move elevator from source floor to destination floor
// you will probably have to modify this in the future ...

static void move2dest(int elevator, int source, int destination, void (*move_elevator)(int, int))
{
    int direction, steps;
    int distance = destination - source;
    if (distance > 0) {
        direction = 1;
        steps = distance;
    } else {
        direction = -1;
        steps = -1*distance;
    }
    for (; steps > 0; steps--)
        move_elevator(elevator, direction);
}


void elevator_controller(int elevator, int at_floor,
                    void (*move_elevator)(int, int), void (*open_door)(int),
                    void (*close_door)(int))
{
    int pid = -1;
    pthread_mutex_lock(&lock);

    while (1) {
        int found = 0;
        for (int i = 0; i < PASSENGERS; i++) {
            if (passenger_ready_for_pickup[i]) {
                pid = i;
                passenger_ready_for_pickup[i] = 0;
                serving_elevator[pid] = elevator;
                trips_remaining--;
                found = 1;
                break;
            }
        }

        if (found)
            break;
    
        // no request yet: check if we’re done, else sleep
        if (trips_remaining <= 0) {
            pthread_mutex_unlock(&lock);
            return;
        }
        pthread_cond_wait(&cond, &lock);
    }

    pthread_mutex_unlock(&lock);
    move2dest(elevator, at_floor, Passenger[pid].from_floor, move_elevator);

    // open door and signal passenger
    open_door(elevator);
    pthread_mutex_lock(&lock);
    elevator_at_pickup[pid] = 1;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&lock);

    // wait for passenger to enter then close and move
    pthread_mutex_lock(&lock);
    while (!passenger_in_elevator[pid])
        pthread_cond_wait(&cond, &lock);
    pthread_mutex_unlock(&lock);
    close_door(elevator);
    move2dest(elevator, Passenger[pid].from_floor, Passenger[pid].to_floor, move_elevator);

    // open door then signal arrival
    open_door(elevator);
    pthread_mutex_lock(&lock);
    elevator_at_destination[pid] = 1;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&lock);

    // wait for passenger to leave and close door
    pthread_mutex_lock(&lock);
    while (!passenger_exited_elevator[pid])
        pthread_cond_wait(&cond, &lock);
    pthread_mutex_unlock(&lock);
    close_door(elevator);
}
