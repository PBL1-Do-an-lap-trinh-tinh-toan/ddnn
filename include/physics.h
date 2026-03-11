#ifndef PHYSICS_H
#define PHYSICS_H

#include <raylib/raymath.h>

#define VERTEX_RADIUS 20

typedef struct {
    Vector2 *position; // meter
    Vector2 velocity; // meter/second
    float mass; // kilogram
} Body;

void BodyInit(Body *body, Vector2 *pos_vec);

void ApplyForce(Body *body, Vector2 force, double duration);

void Inertia(Body *body, double delta_time);

int CollideWith(Body *a, Body *b);

void ResolveCollision(Body *a, Body *b, const double restitution);

#endif
