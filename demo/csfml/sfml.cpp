#include "sfml.hpp"

#include <SFML/Graphics.hpp>

#include <array>

int main()
{
    constexpr unsigned int width = 800;
    constexpr unsigned int height = 600;
    constexpr float radius = 28.0f;
    constexpr float dt = 1.0f / 60.0f;
    constexpr float jitter_strength = 3.5f;

    sf::RenderWindow window(
        sf::VideoMode(sf::Vector2u { width, height }), "Glu + SFML demo"
    );
    window.setFramerateLimit(60);

    sf::CircleShape circle(radius);

    std::array<float, 2> pos = { 120.0f, 160.0f };
    std::array<float, 2> vel = { 180.0f, 140.0f };
    std::array<float, 2> bounds
        = { static_cast<float>(width), static_cast<float>(height) };

    std::array<uint8_t, 4> rgba = { 255, 255, 255, 255 };
    sf::Color const background(18, 20, 28, 255);
    uint32_t frame = 0;

    while (window.isOpen()) {
        while (auto const event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        glu_update_motion(pos.data(), vel.data(), bounds.data(), radius, dt);
        glu_color_from_frame(frame, rgba.data());

        float wiggle[2] = { 0.0f, 0.0f };
        glu_jitter(frame, jitter_strength, wiggle);

        circle.setPosition(
            sf::Vector2f { pos[0] + wiggle[0], pos[1] + wiggle[1] }
        );
        circle.setFillColor(sf::Color(rgba[0], rgba[1], rgba[2], rgba[3]));

        window.clear(background);
        window.draw(circle);
        window.display();

        frame += 1;
    }

    return 0;
}
