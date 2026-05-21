#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <random>
#include <windows.h>

#include <thread>
#include <atomic>
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

void renderingThread(sf::RenderWindow& window, std::vector<Particle>& particles, SpatialGrid& grid, std::atomic<bool>& running)
{
    // Activate the window's context in this thread
    (void)window.setActive(true);

    sf::Clock clock;
    sf::Vector2i prevWindowPos = window.getPosition();

    static float timeAccumulator = 0.0f;
    static int fpsFrameCount = 0;
    std::string title = "Collision Simulator";

    while (running)
    {
        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f; // Prevent extreme physics updates on window stalls

        // 1. Calculate window movement relative to the desktop for inertial forces
        sf::Vector2i currWindowPos = window.getPosition();
        sf::Vector2i windowDelta = currWindowPos - prevWindowPos;
        prevWindowPos = currWindowPos;

        // Apply inertial effect: window acceleration/movement transfers force in the opposite direction
        if (windowDelta.x != 0 || windowDelta.y != 0)
        {
            // The multiplier converts the pixel translation into physical velocity impulse
            Vector2D inertialImpulse(
                static_cast<float>(-windowDelta.x) * 12.0f,
                static_cast<float>(-windowDelta.y) * 12.0f
            );

            // Cap the maximum velocity impulse to keep physics stable
            float maxImpulse = 800.0f;
            if (inertialImpulse.length() > maxImpulse)
            {
                inertialImpulse = inertialImpulse.normalize() * maxImpulse;
            }

            for (auto &obj : particles)
            {
                obj.addVelocity(inertialImpulse);
            }
        }

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

        // Update physics for all particles
        for (auto &obj : particles)
        {
            obj.update_physics(dt);
        }

        // Handle collisions between particles using Spatial Grid
        grid.handleCollisions(particles);

        // Render all particles
        for (auto &obj : particles)
        {
            obj.render(window);
        }

        window.display();

        // Yield slightly to prevent 100% core utilization when frame limit is off
        sf::sleep(sf::milliseconds(1));
    }
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
    
    // Set to 0 so we can display raw FPS and responsiveness in the background thread
    window.setFramerateLimit(0);

    std::vector<Particle> particles = generateParticles(100);
    SpatialGrid grid(800.0f, 800.0f, 20.0f); // 800x800 width/height, 20.0f max particle radius

    // Deactivate the OpenGL context on the main thread so the rendering thread can safely claim it
    (void)window.setActive(false);

    std::atomic<bool> running{true};
    std::thread renderThread(renderingThread, std::ref(window), std::ref(particles), std::ref(grid), std::ref(running));

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                running = false;
                window.close();
            }
        }

        // Yield main thread to avoid hogging CPU cycles during OS event polling
        sf::sleep(sf::milliseconds(10));
    }

    // Clean up the physics and rendering thread safely before exiting
    if (renderThread.joinable())
    {
        renderThread.join();
    }
}