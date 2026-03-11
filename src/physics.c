#include <raylib/raymath.h>
#include <physics.h>
#include <constants.h>

void BodyInit(Body *body, Vector2 *pos_vec) {
    body->position = pos_vec;
    body->velocity = Vector2Zero();
    body->mass = 1.0;
}

void ApplyForce(Body *body, Vector2 force, double duration) {
    if(body->mass == INFINITY) return;

    Vector2 frame_vel = (Vector2){
        force.x / body->mass * duration,
        force.y / body->mass * duration
    };
    body->velocity = Vector2Add(body->velocity, frame_vel);

}

void Inertia(Body *body, double delta_time) {
    *body->position = Vector2Add(*body->position, (Vector2){ body->velocity.x * delta_time, body->velocity.y * delta_time });
}
