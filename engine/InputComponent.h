#pragma once

#include <bitset>
#include <unordered_map>

#include "MathUtil.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Keyboard.hpp>

namespace ECSEngine {

struct InputComponent {
    InputComponent(EntityID whichEntity) : entity(whichEntity), lastDash(0) {}
    EntityID entity;
    int lastDash;
    int wallJumpProtection = 0;
    std::bitset<sf::Keyboard::ScancodeCount> keydown;
    std::bitset<sf::Keyboard::ScancodeCount> lastKeydown;

    //Added to help with wall control
    int wallJumpedFrom = 0;
};

}  // namespace ECSEngine
