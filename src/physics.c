#include <physics.h>
#include <raylib/raymath.h>
#include <constants.h>

void BodyInit(Vertex *body) {
    body->position = Vector2Zero();
    body->velocity = Vector2Zero();
    body->mass = 0.1;
}

void ApplyForce(Vertex *body, Vector2 force, double duration) {
    if(body->mass == INFINITY) return;

    Vector2 frame_vel = (Vector2){
        force.x / body->mass * duration,
        force.y / body->mass * duration
    };
    body->velocity = Vector2Add(body->velocity, frame_vel);
}

void Inertia(Vertex *body, double delta_time) {
    body->position = Vector2Add(body->position, (Vector2){ body->velocity.x * delta_time, body->velocity.y * delta_time });
}
