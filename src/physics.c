#include "raylib/raymath.h"
#include <physics.h>

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

int CollideWith(Body *a, Body *b) {
    Vector2 d = Vector2Subtract(*a->position, *b->position);
    return Vector2LengthSqr(d) <= 4 * VERTEX_RADIUS * VERTEX_RADIUS;
}

void ResolveCollision(Body *a, Body *b, const double restitution) {
    Vector2 d = Vector2Subtract(*b->position, *a->position);
    float distance_squared = Vector2LengthSqr(d);
    float radius_sum = 2 * VERTEX_RADIUS;

    if(distance_squared >= radius_sum * radius_sum)
        return;

    Vector2 normal = Vector2Normalize(d);

    Vector2 relative_velocity = Vector2Subtract(a->velocity, b->velocity);
    float impulse_magnitude = -(1.0f + restitution) * Vector2DotProduct(relative_velocity, normal);
    impulse_magnitude /= (1.0f / a->mass + 1.0f / b->mass);
    Vector2 impulse = (Vector2){ normal.x * impulse_magnitude, normal.y * impulse_magnitude };

    if(a->mass != INFINITY) {
        a->velocity = Vector2Add(
            a->velocity,
            (Vector2){ impulse.x / a->mass, impulse.y / a->mass }
        );
    }
    if(b->mass != INFINITY) {
        b->velocity = Vector2Add(
            b->velocity,
            (Vector2){ impulse.x / b->mass, impulse.y / b->mass }
        );
    }

    // avoid overlapping
    float penetration_depth = radius_sum - sqrt(distance_squared);
    Vector2 correction = (Vector2){
        normal.x * (penetration_depth / (1.0f / a->mass + 1.0f / b->mass)) * 1.0f,
        normal.y * (penetration_depth / (1.0f / a->mass + 1.0f / b->mass)) * 1.0f
    };
    if(a->mass != INFINITY) {
        *a->position = Vector2Subtract(*a->position, (Vector2){ correction.x / a->mass, correction.y / a->mass });
    }
    if(b->mass != INFINITY) {
        *b->position = Vector2Add(*b->position, (Vector2){ correction.x / b->mass, correction.y / b->mass });
    }
}
