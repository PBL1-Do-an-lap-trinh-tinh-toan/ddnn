#ifndef PHYSICS_H
#define PHYSICS_H

#include <raylib/raymath.h>

typedef struct {
    Vector2 position; // meter
    Vector2 velocity; // meter/second
    float mass; // kilogram
} Body;

void BodyInit(Body *body, Vector2 **pos_vec);

void ApplyForce(Body *body, Vector2 force, double duration);

void Inertia(Body *body, double delta_time);

#endif
