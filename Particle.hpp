#pragma once
#include "Vector2D.hpp"
#include <iostream>
#include <iomanip>

static constexpr float width = 800.0f;
static constexpr float height = 800.0f;
static constexpr float gravity = 800.0f; // Downward gravity in pixels/s^2

class Particle
{
private:
    // Physical properties
    float radius;
    float mass;
    Vector2D cur_position;
    Vector2D velocity;
    Vector2D acceleration;
    // Visual properties
    sf::CircleShape shape;
    sf::Color color;

public:
    static float restitution;

    // constructors
    Particle() : radius(0.0f), mass(0.0f) {}
    Particle(const float r, const float m, const Vector2D &cur_pos, const Vector2D &acc, const Vector2D &vel, const sf::Color &col)
        : radius(r), mass(m), cur_position(cur_pos), acceleration(acc), velocity(vel), color(col)
    {
        shape.setRadius(radius);
        shape.setOrigin(sf::Vector2f(radius, radius));
        shape.setFillColor(col);
        shape.setOutlineColor(sf::Color::White);
        shape.setOutlineThickness(1.0f);
    }

    // getters
    float getRadius() const { return radius; }
    float getMass() const { return mass; }
    Vector2D getPosition() const { return cur_position; }
    Vector2D getVelocity() const { return velocity; }
    Vector2D getAcceleration() const { return acceleration; }

    void addVelocity(const Vector2D &v) { velocity = velocity + v; }

    // Draws the particle on the display
    void render(sf::RenderWindow &window)
    {
        shape.setPosition(sf::Vector2f(cur_position.getX(), cur_position.getY()));
        window.draw(shape);
    }

    // Updates the position of the particle
    void update_physics(float dt)
    {
        // Apply constant gravity downward
        velocity = velocity + Vector2D(0.0f, gravity) * dt;

        velocity = velocity + acceleration * dt;
        cur_position = cur_position + velocity * dt + (acceleration * dt * dt) / 2;
        collision_walls();
    }

    void collision_walls()
    {
        float x = cur_position.getX();
        float y = cur_position.getY();

        if (x + radius > width || x < radius)
        {
            if (x + radius > width)
                cur_position = Vector2D(width - radius, y);
            else
                cur_position = Vector2D(radius, y);
            velocity = Vector2D(-velocity.getX() * restitution, velocity.getY());
        }

        if (y + radius > height || y < radius)
        {
            if (y + radius > height)
                cur_position = Vector2D(x, height - radius);
            else
                cur_position = Vector2D(x, radius);
            velocity = Vector2D(velocity.getX() * 0.98f, -velocity.getY() * restitution); // 0.98 friction on sliding along floor/ceiling
        }
    }

    // Collision solving between two particles
    void collision_solve(Particle &other)
    {
        Vector2D diff = other.cur_position - cur_position;
        float distSq = diff.dot(diff);
        float minDist = radius + other.getRadius();
        float minDistSq = minDist * minDist;

        if (distSq < minDistSq && distSq > 0.0f)
        {
            float dist = std::sqrt(distSq);
            // Position correction
            float overlap = 0.5f * (minDist - dist);
            Vector2D normal = diff / dist;
            Vector2D correction = normal * overlap;
            cur_position -= correction;
            other.cur_position += correction;

            // Collision response
            Vector2D tangent = normal.tangent();

            // Project velocities onto normal and tangent
            float v1n = normal.dot(velocity);
            float v1t = tangent.dot(velocity);
            float v2n = normal.dot(other.velocity);
            float v2t = tangent.dot(other.velocity);

            float m1 = mass;
            float m2 = other.mass;

            float e = restitution; // Coefficient of restitution (inelastic collisions)

            // Calculate new normal velocities using inelastic collision formula
            float v1n_after = (v1n * (m1 - e * m2) + (1.0f + e) * m2 * v2n) / (m1 + m2);
            float v2n_after = (v2n * (m2 - e * m1) + (1.0f + e) * m1 * v1n) / (m1 + m2);

            // Reconstruct velocity vectors
            velocity = normal * v1n_after + tangent * v1t;
            other.velocity = normal * v2n_after + tangent * v2t;
        }
    }

    float kineticEnergy() const
    {
        float v2 = velocity.getX() * velocity.getX() + velocity.getY() * velocity.getY();
        return 0.5f * mass * v2;
    }

    void debugInfo() const
    {
        float speed = velocity.length();
        float kineticEnergy = 0.5f * mass * speed * speed;

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Particle Info:\n";
        std::cout << "  Position: (" << cur_position.getX() << ", " << cur_position.getY() << ")\n";
        std::cout << "  Velocity: (" << velocity.getX() << ", " << velocity.getY() << ")\n";
        std::cout << "  Speed: " << speed << "\n";
        std::cout << "  Kinetic Energy: " << kineticEnergy << "\n";
        std::cout << "--------------------------------\n";
    }
};