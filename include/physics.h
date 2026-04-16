#ifndef PHYSICS_H
#define PHYSICS_H

#include <graph.h>

void BodyInit(Vertex *body);

void ApplyForce(Vertex *body, Vector2 force, double duration);

void Inertia(Vertex *body, double delta_time);

#endif
