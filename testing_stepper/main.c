/* 
 * File:   main.c
 * Author: Robert
 *
 * Created on 11 październik 2012, 17:18
 */

#include <stdio.h>
#include <stdlib.h>

#define STEPS 100
/*
 * 
 */
int main(int argc, char** argv) {

int steps_state = 0;
int steps_received = 0;
int steps_todo = 0;
int oversteps = 0;
int steps_left = 0;
int steps_right = 0;
int steps_dir = 0;

while(1){
    puts("Podaj kierunek 1-prawo, 2-lewo");
    scanf("%d", &steps_dir);
    fputs("Dir: ", stdout);
    printf("%d", steps_dir);
    putchar('\n');
    if(steps_dir == 1){
        puts("Podaj liczbe krokow w prawo: ");
        scanf("%d", &steps_received);
        printf("%d", steps_state);
        printf("%d", steps_received);
        steps_state += steps_received;
        fputs("State: ", stdout);
        printf("%d", steps_state);
        putchar('\n');
        if(steps_state > STEPS){
            oversteps = steps_state % STEPS;
            steps_state = STEPS;
            steps_todo = steps_received - oversteps;
            fputs("TODO: ", stdout);
            printf("%d", steps_todo);
            putchar('\n');
        }
        else{
            steps_todo = steps_received;
            fputs("TODO1: ", stdout);
            printf("%d", steps_todo);
            putchar('\n');
        }
    }
    if(steps_dir == 2){
        if(steps_state > 100){
            steps_state = STEPS;
        }
        puts("Podaj liczbe krokow w lewo: ");
        scanf("%d", &steps_received);
        steps_state -= steps_received;
        fputs("State: ", stdout);
        printf("%d", steps_state);
        putchar('\n');
        if(steps_state < 0){
            oversteps = (steps_state*-1) % STEPS;
            steps_state = 0;
            steps_todo = steps_received - oversteps;
            fputs("TODO: ", stdout);
            printf("%d", steps_todo);
            putchar('\n');
        }
        else{
            steps_todo = steps_received;
            fputs("TODO1: ", stdout);
            printf("%d", steps_todo);
            putchar('\n');
        }
    }
    
}
    
    
    return (EXIT_SUCCESS);
}

