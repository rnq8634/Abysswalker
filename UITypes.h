#ifndef __UITYPES_H__
#define __UITYPES_H__

// Local includes
#include "PlayerStats.h" 

// Lib includes
#include <string>

class Sprite;
class Texture;

struct UIElemRect
{
    float x, y, width, height;
};

struct UIButton
{
    UIElemRect rect;
    StatType statToUpgrade;
    bool isHovered;
    std::string identifier;

    Sprite* textSprite;
    Texture* textTexture;

    UIButton() : textSprite(nullptr), textTexture(nullptr), isHovered(false) {}

    bool IsMouseOver(float mouseX, float mouseY) const
    {
        return mouseX >= rect.x && mouseX <= rect.x + rect.width &&
            mouseY >= rect.y && mouseY <= rect.y + rect.height;
    }

};

#endif // __UITYPES_H__