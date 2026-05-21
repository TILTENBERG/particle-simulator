#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <random>
#include <windows.h>
#include <thread>
#include <atomic>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include "Particle.hpp"
#include "Vector2D.hpp"
#include "Partition.hpp"

// Define the static member from Particle
float Particle::restitution = 0.8f;

// Application states
enum class AppState
{
    SETUP,
    RUNNING
};

// Reusable Slider Class for Menu configuration (SFML 3 compliant)
class Slider
{
public:
    std::string label;
    float minValue;
    float maxValue;
    float currentValue;
    bool isInteger;

    // Visual elements
    sf::Vector2f position;
    float width;
    sf::RectangleShape track;
    sf::RectangleShape activeTrack;
    sf::CircleShape handle;
    sf::Text labelText;
    sf::Text valueText;
    bool isDragging;
    bool isHovered;
    
    // Keyboard input variables
    bool isEditing;
    std::string inputBuffer;

    Slider(const std::string &lbl, float minVal, float maxVal, float startVal, sf::Vector2f pos, float w, const sf::Font &font, bool isInt = false)
        : label(lbl), minValue(minVal), maxValue(maxVal), currentValue(startVal), isInteger(isInt), position(pos), width(w), isDragging(false),
          isHovered(false), isEditing(false), labelText(font), valueText(font)
    {
        // Track: a slim modern backdrop line
        track.setSize(sf::Vector2f(width, 6.0f));
        track.setFillColor(sf::Color(45, 48, 50));
        track.setPosition(position);

        // Active Track: shows filled progress
        activeTrack.setSize(sf::Vector2f(0.0f, 6.0f));
        activeTrack.setFillColor(sf::Color(0, 120, 220));
        activeTrack.setPosition(position);

        // Handle: a circular glowing knob
        handle.setRadius(10.0f);
        handle.setFillColor(sf::Color(200, 204, 209));
        handle.setOutlineThickness(1.5f);
        handle.setOutlineColor(sf::Color(70, 75, 80));
        handle.setOrigin(sf::Vector2f(10.0f, 10.0f)); // Center origin for perfect math

        // Set initial handle position
        updateHandlePosition();

        // Label Text
        labelText.setString(label);
        labelText.setCharacterSize(15);
        labelText.setFillColor(sf::Color(170, 178, 189));
        labelText.setPosition(sf::Vector2f(position.x, position.y - 28.0f));

        // Value Text
        valueText.setCharacterSize(15);
        valueText.setFillColor(sf::Color(0, 180, 255));
        updateValueText();
    }

    void updateHandlePosition()
    {
        float pct = (currentValue - minValue) / (maxValue - minValue);
        handle.setPosition(sf::Vector2f(position.x + pct * width, position.y + 3.0f));
    }

    void updateValueText()
    {
        if (isEditing) return; // Keep dynamic typing text visible

        if (isInteger)
        {
            valueText.setString(std::to_string(static_cast<int>(std::round(currentValue))));
        }
        else
        {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << currentValue;
            valueText.setString(ss.str());
        }

        // Right-align value text to the right edge of the slider track
        sf::FloatRect valBounds = valueText.getLocalBounds();
        valueText.setOrigin(sf::Vector2f(valBounds.position.x + valBounds.size.x, 0.0f));
        valueText.setPosition(sf::Vector2f(position.x + width, position.y - 28.0f));
    }

    void commitValue()
    {
        if (inputBuffer.empty())
        {
            updateValueText();
            return;
        }
        try
        {
            float val = std::stof(inputBuffer);
            currentValue = std::clamp(val, minValue, maxValue);
            if (isInteger)
            {
                currentValue = std::round(currentValue);
            }
        }
        catch (...)
        {
            // Revert back on parse exception
        }
        updateHandlePosition();
        updateValueText();
    }

    void handleEvent(const sf::Event &event, const sf::RenderWindow &window)
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

        // 1. Handle keyboard entries if this slider is actively in focus
        if (isEditing)
        {
            if (event.is<sf::Event::TextEntered>())
            {
                const auto* textEvent = event.getIf<sf::Event::TextEntered>();
                if (textEvent)
                {
                    char32_t unicode = textEvent->unicode;
                    if (unicode == 8) // Backspace
                    {
                        if (!inputBuffer.empty())
                        {
                            inputBuffer.pop_back();
                        }
                    }
                    else if (unicode == 13 || unicode == 10) // Enter / Return
                    {
                        commitValue();
                        isEditing = false;
                        return;
                    }
                    else if (unicode == 27) // Escape
                    {
                        isEditing = false;
                        updateValueText();
                        return;
                    }
                    else if (unicode >= '0' && unicode <= '9')
                    {
                        inputBuffer += static_cast<char>(unicode);
                    }
                    else if (unicode == '.' && !isInteger)
                    {
                        // Allow only one decimal point
                        if (inputBuffer.find('.') == std::string::npos)
                        {
                            inputBuffer += '.';
                        }
                    }

                    valueText.setString(inputBuffer + "_");
                    sf::FloatRect valBounds = valueText.getLocalBounds();
                    valueText.setOrigin(sf::Vector2f(valBounds.position.x + valBounds.size.x, 0.0f));
                    valueText.setPosition(sf::Vector2f(position.x + width, position.y - 28.0f));
                }
            }

            if (event.is<sf::Event::KeyPressed>())
            {
                const auto* keyEvent = event.getIf<sf::Event::KeyPressed>();
                if (keyEvent)
                {
                    if (keyEvent->code == sf::Keyboard::Key::Enter)
                    {
                        commitValue();
                        isEditing = false;
                        return;
                    }
                    else if (keyEvent->code == sf::Keyboard::Key::Escape)
                    {
                        isEditing = false;
                        updateValueText();
                        return;
                    }
                }
            }
        }

        // 2. Handle mouse clicks and slider dragging
        if (event.is<sf::Event::MouseButtonPressed>())
        {
            const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>();
            if (mouseEvent && mouseEvent->button == sf::Mouse::Button::Left)
            {
                // Check if clicking in the value text region (right aligned)
                float valLeft = position.x + width - std::max(valueText.getLocalBounds().size.x, 50.0f);
                float valRight = position.x + width;
                float valTop = position.y - 32.0f;
                float valBottom = position.y - 8.0f;
                bool clickedVal = (mousePosF.x >= valLeft && mousePosF.x <= valRight && mousePosF.y >= valTop && mousePosF.y <= valBottom);

                if (clickedVal)
                {
                    isEditing = true;
                    isDragging = false;
                    if (isInteger)
                    {
                        inputBuffer = std::to_string(static_cast<int>(std::round(currentValue)));
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << std::fixed << std::setprecision(2) << currentValue;
                        inputBuffer = ss.str();
                    }
                    valueText.setString(inputBuffer + "_");
                    sf::FloatRect valBounds = valueText.getLocalBounds();
                    valueText.setOrigin(sf::Vector2f(valBounds.position.x + valBounds.size.x, 0.0f));
                    valueText.setPosition(sf::Vector2f(position.x + width, position.y - 28.0f));
                }
                else
                {
                    if (isEditing)
                    {
                        commitValue();
                        isEditing = false;
                    }

                    // Check if clicking on or near the handle
                    float dx = mousePosF.x - handle.getPosition().x;
                    float dy = mousePosF.y - handle.getPosition().y;
                    if (dx * dx + dy * dy <= 225.0f) // Within comfortable click radius
                    {
                        isDragging = true;
                    }
                    // Or if clicking on the track itself
                    else if (mousePosF.x >= position.x && mousePosF.x <= position.x + width &&
                             mousePosF.y >= position.y - 8.0f && mousePosF.y <= position.y + 14.0f)
                    {
                        isDragging = true;
                        updateValueFromMouse(mousePosF.x);
                    }
                }
            }
        }

        if (event.is<sf::Event::MouseButtonReleased>())
        {
            const auto* mouseEvent = event.getIf<sf::Event::MouseButtonReleased>();
            if (mouseEvent && mouseEvent->button == sf::Mouse::Button::Left)
            {
                isDragging = false;
            }
        }

        if (isDragging && !isEditing)
        {
            updateValueFromMouse(mousePosF.x);
        }
    }

    void updateValueFromMouse(float mouseX)
    {
        float newX = std::clamp(mouseX, position.x, position.x + width);
        float pct = (newX - position.x) / width;
        currentValue = minValue + pct * (maxValue - minValue);
        if (isInteger)
        {
            currentValue = std::round(currentValue);
        }
        updateHandlePosition();
        updateValueText();
    }

    void update(const sf::RenderWindow &window)
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

        float dx = mousePosF.x - handle.getPosition().x;
        float dy = mousePosF.y - handle.getPosition().y;
        bool hoverHandle = (dx * dx + dy * dy <= 196.0f); // ~14px radius
        bool hoverTrack = (mousePosF.x >= position.x && mousePosF.x <= position.x + width &&
                           mousePosF.y >= position.y - 8.0f && mousePosF.y <= position.y + 14.0f);

        // Check if hovered over the value box
        float valLeft = position.x + width - std::max(valueText.getLocalBounds().size.x, 50.0f);
        float valRight = position.x + width;
        float valTop = position.y - 32.0f;
        float valBottom = position.y - 8.0f;
        bool hoverValue = (mousePosF.x >= valLeft && mousePosF.x <= valRight && mousePosF.y >= valTop && mousePosF.y <= valBottom);

        isHovered = hoverHandle || hoverTrack;

        // Visual hover/drag animations
        if (isEditing)
        {
            valueText.setFillColor(sf::Color(255, 200, 0)); // Amber text
            handle.setFillColor(sf::Color(120, 125, 130)); // Dimmed handle
            handle.setOutlineColor(sf::Color(70, 75, 80));
            handle.setRadius(9.0f);
            handle.setOrigin(sf::Vector2f(9.0f, 9.0f));
        }
        else
        {
            if (hoverValue)
            {
                valueText.setFillColor(sf::Color(255, 255, 255)); // Highlight on hover
            }
            else
            {
                valueText.setFillColor(sf::Color(0, 180, 255)); // Standard neon cyan
            }

            if (isDragging)
            {
                handle.setFillColor(sf::Color(255, 255, 255));
                handle.setOutlineColor(sf::Color(0, 180, 255));
                handle.setRadius(12.0f);
                handle.setOrigin(sf::Vector2f(12.0f, 12.0f));
            }
            else if (isHovered)
            {
                handle.setFillColor(sf::Color(230, 235, 240));
                handle.setOutlineColor(sf::Color(0, 150, 255));
                handle.setRadius(11.0f);
                handle.setOrigin(sf::Vector2f(11.0f, 11.0f));
            }
            else
            {
                handle.setFillColor(sf::Color(200, 204, 209));
                handle.setOutlineColor(sf::Color(70, 75, 80));
                handle.setRadius(9.0f);
                handle.setOrigin(sf::Vector2f(9.0f, 9.0f));
            }
        }

        // Active track progress width
        float pct = (currentValue - minValue) / (maxValue - minValue);
        activeTrack.setSize(sf::Vector2f(pct * width, 6.0f));

        if (isDragging || isHovered)
        {
            activeTrack.setFillColor(sf::Color(0, 150, 255)); // Bright neon blue
        }
        else
        {
            activeTrack.setFillColor(sf::Color(0, 120, 220)); // Subdued blue
        }
    }

    void render(sf::RenderWindow &window)
    {
        window.draw(track);
        window.draw(activeTrack);
        window.draw(handle);
        window.draw(labelText);
        window.draw(valueText);

        // Draw an underline for the value input when editing
        if (isEditing)
        {
            float textW = std::max(valueText.getLocalBounds().size.x, 50.0f);
            sf::RectangleShape underline(sf::Vector2f(textW, 2.0f));
            underline.setFillColor(sf::Color(0, 180, 255)); // Bright neon cyan
            underline.setPosition(sf::Vector2f(position.x + width - textW, position.y - 6.0f));
            window.draw(underline);
        }
    }
};

// Reusable Button Class for Menu configuration (SFML 3 compliant)
class Button
{
public:
    sf::RectangleShape shape;
    sf::Text text;
    sf::Vector2f position;
    sf::Vector2f size;

    Button(const std::string &txtStr, sf::Vector2f pos, sf::Vector2f sz, const sf::Font &font)
        : position(pos), size(sz), text(font)
    {
        shape.setSize(size);
        shape.setFillColor(sf::Color(20, 23, 27)); // Premium dark charcoal fill
        shape.setPosition(position);
        shape.setOutlineThickness(2.0f);
        shape.setOutlineColor(sf::Color(0, 120, 255)); // Electric blue border

        text.setString(txtStr);
        text.setCharacterSize(16);
        text.setFillColor(sf::Color(0, 150, 255)); // Cyan text
        text.setStyle(sf::Text::Bold);
        
        // Center text inside button (SFML 3 rect property style)
        sf::FloatRect textRect = text.getLocalBounds();
        text.setOrigin(sf::Vector2f(textRect.position.x + textRect.size.x / 2.0f, textRect.position.y + textRect.size.y / 2.0f));
        text.setPosition(sf::Vector2f(position.x + size.x / 2.0f, position.y + size.y / 2.0f));
    }

    bool isHovered(const sf::RenderWindow &window) const
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        return (mousePos.x >= position.x && mousePos.x <= position.x + size.x &&
                mousePos.y >= position.y && mousePos.y <= position.y + size.y);
    }

    void handleEvent(const sf::Event &event, const sf::RenderWindow &window, bool &clicked)
    {
        if (event.is<sf::Event::MouseButtonPressed>())
        {
            const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>();
            if (mouseEvent && mouseEvent->button == sf::Mouse::Button::Left)
            {
                if (isHovered(window))
                {
                    clicked = true;
                }
            }
        }
    }

    void update(const sf::RenderWindow &window)
    {
        if (isHovered(window))
        {
            shape.setFillColor(sf::Color(0, 120, 255)); // Solid glow fill
            shape.setOutlineColor(sf::Color(0, 200, 255)); // Brighter cyan outline
            text.setFillColor(sf::Color::White); // White text on hover
        }
        else
        {
            shape.setFillColor(sf::Color(20, 23, 27)); // Premium dark charcoal
            shape.setOutlineColor(sf::Color(0, 120, 255)); // Electric blue outline
            text.setFillColor(sf::Color(0, 150, 255)); // Cyan text
        }
    }

    void render(sf::RenderWindow &window)
    {
        window.draw(shape);
        window.draw(text);
    }
};

// Generates particles with custom settings
std::vector<Particle> generateParticles(int count, float maxRadius, float maxMass)
{
    std::vector<Particle> particles;
    particles.reserve(count);

    const float width = 800.0f;
    const float height = 800.0f;

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<float> radiusDist(5.0f, maxRadius);
    std::uniform_real_distribution<float> velDist(-200.0f, 200.0f);
    std::uniform_real_distribution<float> massDist(1.0f, maxMass);
    std::uniform_int_distribution<int> colorDist(0, 255);

    for (int i = 0; i < count; ++i)
    {
        float r = radiusDist(gen);

        // Clamp spawn position inside screen boundaries
        std::uniform_real_distribution<float> posXDist(r, width - r);
        std::uniform_real_distribution<float> posYDist(r, height - r);

        float x = posXDist(gen);
        float y = posYDist(gen);
        Vector2D cur_pos(x, y);

        float vx = velDist(gen);
        float vy = velDist(gen);
        Vector2D velocity(vx, vy);

        float mass = massDist(gen);
        Vector2D acceleration(0.0f, 0.0f);

        sf::Color color(colorDist(gen), colorDist(gen), colorDist(gen));

        particles.emplace_back(r, mass, cur_pos, acceleration, velocity, color);
    }

    return particles;
}

// Background thread function for Zero-Stall rendering and physics
void renderingThread(sf::RenderWindow& window, std::vector<Particle>& particles, SpatialGrid& grid, std::atomic<bool>& running)
{
    (void)window.setActive(true);

    sf::Clock clock;
    sf::Vector2i prevWindowPos = window.getPosition();

    static float timeAccumulator = 0.0f;
    static int fpsFrameCount = 0;
    std::string title = "Collision Simulator";

    while (running)
    {
        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f; // Cap time step on stutter

        // 1. Calculate window desktop translation to apply physical inertia
        sf::Vector2i currWindowPos = window.getPosition();
        sf::Vector2i windowDelta = currWindowPos - prevWindowPos;
        prevWindowPos = currWindowPos;

        if (windowDelta.x != 0 || windowDelta.y != 0)
        {
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

        window.clear(sf::Color(15, 15, 18)); // Smooth dark simulation backdrop

        // Update physics
        for (auto &obj : particles)
        {
            obj.update_physics(dt);
        }

        // Broad-phase collisions
        grid.handleCollisions(particles);

        // Draw
        for (auto &obj : particles)
        {
            obj.render(window);
        }

        window.display();
        sf::sleep(sf::milliseconds(1));
    }
}

int main()
{
    std::string title = "Collision Simulator";
    sf::RenderWindow window(sf::VideoMode({800, 800}), title);
    window.setFramerateLimit(60); // 60 FPS for setup page interactions

    // Load standard Windows system font (SFML 3 compliant openFromFile)
    sf::Font font;
    if (!font.openFromFile("C:\\Windows\\Fonts\\arial.ttf"))
    {
        std::cerr << "Error loading system font arial.ttf!\n";
    }

    // Header Text Setup (SFML 3 compliant constructor and rect calls)
    sf::Text menuTitle(font);
    menuTitle.setString("COLLISION SIMULATOR");
    menuTitle.setCharacterSize(36);
    menuTitle.setFillColor(sf::Color::White);
    menuTitle.setStyle(sf::Text::Bold);
    sf::FloatRect titleRect = menuTitle.getLocalBounds();
    menuTitle.setOrigin(sf::Vector2f(titleRect.position.x + titleRect.size.x / 2.0f, titleRect.position.y + titleRect.size.y / 2.0f));
    menuTitle.setPosition(sf::Vector2f(400.0f, 80.0f));

    sf::Text menuSubtitle(font);
    menuSubtitle.setString("Configure parameters and click start");
    menuSubtitle.setCharacterSize(16);
    menuSubtitle.setFillColor(sf::Color(140, 150, 160));
    sf::FloatRect subtitleRect = menuSubtitle.getLocalBounds();
    menuSubtitle.setOrigin(sf::Vector2f(subtitleRect.position.x + subtitleRect.size.x / 2.0f, subtitleRect.position.y + subtitleRect.size.y / 2.0f));
    menuSubtitle.setPosition(sf::Vector2f(400.0f, 130.0f));

    // Decorative line
    sf::RectangleShape separator(sf::Vector2f(500.0f, 2.0f));
    separator.setFillColor(sf::Color(50, 55, 60));
    separator.setPosition(sf::Vector2f(150.0f, 160.0f));

    // Slider configurations (horizontal sliders like volume controls)
    Slider sliderCount("Particle Count", 10.0f, 1000.0f, 100.0f, sf::Vector2f(200.0f, 220.0f), 400.0f, font, true);
    Slider sliderRadius("Max Radius (px)", 5.0f, 45.0f, 20.0f, sf::Vector2f(200.0f, 320.0f), 400.0f, font, false);
    Slider sliderMass("Max Mass (kg)", 1.0f, 100.0f, 20.0f, sf::Vector2f(200.0f, 420.0f), 400.0f, font, false);
    Slider sliderRestitution("Restitution (Bounciness)", 0.0f, 1.0f, 0.8f, sf::Vector2f(200.0f, 520.0f), 400.0f, font, false);

    // Button Setup
    Button startButton("START SIMULATION", sf::Vector2f(280.0f, 620.0f), sf::Vector2f(240.0f, 50.0f), font);

    // Escape hint text (will be displayed during simulation)
    sf::Text escapeHint(font);
    escapeHint.setString("Press ESC to return to setup");
    escapeHint.setCharacterSize(14);
    escapeHint.setFillColor(sf::Color(100, 110, 120));
    sf::FloatRect hintRect = escapeHint.getLocalBounds();
    escapeHint.setOrigin(sf::Vector2f(hintRect.position.x + hintRect.size.x / 2.0f, hintRect.position.y + hintRect.size.y / 2.0f));
    escapeHint.setPosition(sf::Vector2f(400.0f, 780.0f));

    // Simulation variables
    std::vector<Particle> particles;
    SpatialGrid grid(800.0f, 800.0f, 20.0f);
    std::atomic<bool> running{false};
    std::thread renderThread;

    AppState state = AppState::SETUP;

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                running = false;
                if (renderThread.joinable())
                {
                    renderThread.join();
                }
                window.close();
            }

            // Keyboard navigation
            if (event->is<sf::Event::KeyPressed>())
            {
                const auto* keyEvent = event->getIf<sf::Event::KeyPressed>();
                if (keyEvent && keyEvent->code == sf::Keyboard::Key::Escape)
                {
                    // Escape key exits RUNNING state and returns to SETUP safely
                    if (state == AppState::RUNNING)
                    {
                        running = false;
                        if (renderThread.joinable())
                        {
                            renderThread.join();
                        }
                        (void)window.setActive(true);
                        window.setFramerateLimit(60);
                        state = AppState::SETUP;
                    }
                }
            }

            // Menu state inputs
            if (state == AppState::SETUP)
            {
                sliderCount.handleEvent(*event, window);
                sliderRadius.handleEvent(*event, window);
                sliderMass.handleEvent(*event, window);
                sliderRestitution.handleEvent(*event, window);

                bool clicked = false;
                startButton.handleEvent(*event, window, clicked);
                if (clicked)
                {
                    // Start the simulation with the slider settings
                    int count = static_cast<int>(sliderCount.currentValue);
                    float maxRadius = sliderRadius.currentValue;
                    float maxMass = sliderMass.currentValue;
                    float restitution = sliderRestitution.currentValue;

                    Particle::restitution = restitution;
                    particles = generateParticles(count, maxRadius, maxMass);
                    grid = SpatialGrid(800.0f, 800.0f, maxRadius);

                    state = AppState::RUNNING;
                    window.setFramerateLimit(0); // Uncap FPS during simulation

                    (void)window.setActive(false); // Yield context to the thread
                    running = true;
                    renderThread = std::thread(renderingThread, std::ref(window), std::ref(particles), std::ref(grid), std::ref(running));
                }
            }
        }

        // Render loop for setup page
        if (state == AppState::SETUP)
        {
            // Update UI elements
            sliderCount.update(window);
            sliderRadius.update(window);
            sliderMass.update(window);
            sliderRestitution.update(window);
            startButton.update(window);

            window.clear(sf::Color(25, 27, 29)); // Premium sleek dark charcoal background

            // Draw a high-tech glowing grid background
            sf::VertexArray gridLines(sf::PrimitiveType::Lines);
            sf::Color gridColor(32, 34, 38);
            for (float x = 0.0f; x < 800.0f; x += 40.0f)
            {
                gridLines.append(sf::Vertex{sf::Vector2f(x, 0.0f), gridColor});
                gridLines.append(sf::Vertex{sf::Vector2f(x, 800.0f), gridColor});
            }
            for (float y = 0.0f; y < 800.0f; y += 40.0f)
            {
                gridLines.append(sf::Vertex{sf::Vector2f(0.0f, y), gridColor});
                gridLines.append(sf::Vertex{sf::Vector2f(800.0f, y), gridColor});
            }
            window.draw(gridLines);

            // Draw card panel container for sliders
            sf::RectangleShape panel(sf::Vector2f(500.0f, 515.0f));
            panel.setPosition(sf::Vector2f(150.0f, 175.0f));
            panel.setFillColor(sf::Color(18, 20, 23, 230)); // Glassmorphism dark panel (semi-transparent)
            panel.setOutlineThickness(1.5f);
            panel.setOutlineColor(sf::Color(45, 48, 52));
            window.draw(panel);

            window.draw(menuTitle);
            window.draw(menuSubtitle);
            window.draw(separator);

            sliderCount.render(window);
            sliderRadius.render(window);
            sliderMass.render(window);
            sliderRestitution.render(window);

            startButton.render(window);
            window.draw(escapeHint);

            window.display();
        }
        else
        {
            // If running, we sleep on the main thread to prevent 100% CPU usage
            sf::sleep(sf::milliseconds(10));
        }
    }

    if (renderThread.joinable())
    {
        renderThread.join();
    }
    return 0;
}