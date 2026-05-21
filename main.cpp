#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <random>
#include <windows.h>

#include "Particle.hpp"
#include "Vector2D.hpp"
#include "Partition.hpp"

std::vector<Particle> generateParticles(int count)
{
    std::vector<Particle> particles;
    particles.reserve(count);

    const float width = 800.0f;
    const float height = 800.0f;
    const float max_radius = 20.0f;

    // Create a random device and Mersenne Twister engine
    std::random_device rd;
    std::mt19937 gen(rd());

    // Distributions with fixed ranges
    std::uniform_real_distribution<float> radiusDist(1.0f, max_radius);
    std::uniform_real_distribution<float> velDist(-200.0f, 200.0f);
    std::uniform_real_distribution<float> massDist(1.0f, 20.0f);
    std::uniform_int_distribution<int> colorDist(0, 255);

    for (int i = 0; i < count; ++i)
    {
        // Generate the radius for THIS particle first
        // float r = radiusDist(gen);

        float r = 20.0;
        // Create temporary position distributions
        // using the generated radius 'r' to define their range.
        std::uniform_real_distribution<float> posXDist(r, width - r);
        std::uniform_real_distribution<float> posYDist(r, height - r);

        // Generate the properties for this particle
        float x = posXDist(gen);
        float y = posYDist(gen);
        Vector2D cur_pos(x, y);

        float vx = velDist(gen);
        float vy = velDist(gen);
        Vector2D velocity(vx, vy);

        float mass = massDist(gen);
        Vector2D acceleration(0.0f, 0.0f);

        sf::Color color(colorDist(gen), colorDist(gen), colorDist(gen));

        // Add the particle to vector
        particles.emplace_back(r, mass, cur_pos, acceleration, velocity, color);
    }

    return particles;
}

int main()
{
    /* AllocConsole();
     FILE *f;
     freopen_s(&f, "CONOUT$", "w", stdout);
     std::cout << "Debug info will now appear here!\n";
 */
    std::string title = "Collision Simulator";
    sf::RenderWindow window(sf::VideoMode({800, 800}), title);

    // Disable frame rate limit to see the true potential of our optimized physics,
    // but we can also keep VSync or limit it if preferred. Let's keep it unlimited/uncapped
    // so we can see the raw FPS, or set it to 60 as requested.
    window.setFramerateLimit(0); // Set to 0 to measure raw performance!

    std::vector<Particle> particles = generateParticles(100);
    SpatialGrid grid(800.0f, 800.0f, 20.0f); // 800x800 width/height, 20.0f max particle radius
    sf::Clock clock;

    static int frameCount = 0;
    static float timeAccumulator = 0.0f;
    static int fpsFrameCount = 0;

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        float dt = clock.restart().asSeconds();

        // Update FPS in title
        timeAccumulator += dt;
        fpsFrameCount++;
        if (timeAccumulator >= 0.5f)
        {
            float fps = fpsFrameCount / timeAccumulator;
            window.setTitle(title + " | Particles: " + std::to_string(particles.size()) + " | FPS: " + std::to_string(static_cast<int>(std::round(fps))));
            fpsFrameCount = 0;
            timeAccumulator = 0.0f;
        }

        window.clear();

        // Calculate total energy before physics update
        float energyBefore = 0.0f;
        for (auto &obj : particles)
        {
            energyBefore += obj.kineticEnergy();
        }

        // Update physics for all particles
        for (auto &obj : particles)
        {
            obj.update_physics(dt);
        }

        // Handle collisions between particles using Spatial Grid
        grid.handleCollisions(particles);
        /*
        // Calculate total energy after physics update
        float energyAfter = 0.0f;
        for (auto &obj : particles)
        {
            energyAfter += obj.kineticEnergy();
        }

        // Print debug info every 60 frames (once per second at 60 FPS)
        if (frameCount % 60 == 0)
        {
            std::cout << "\n=== Frame " << frameCount << " ===\n";
            std::cout << "Energy before: " << energyBefore
                      << " | after: " << energyAfter
                      << " | diff = " << (energyAfter - energyBefore)
                      << std::endl;

            // Print info for first particle as example
            if (!particles.empty())
            {
                std::cout << "\nFirst particle details:\n";
                particles[0].debugInfo();
            }
        }
        frameCount++;
        */

        // Render all particles
        for (auto &obj : particles)
        {
            obj.render(window);
        }

        window.display();
    }
}