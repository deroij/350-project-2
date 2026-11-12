#include "SpriteManager.h"

namespace ECSEngine {

SpriteID SpriteManager::RegisterTexture(const std::string& texturePath, const Rect& sourceRect) {
    auto search = mTextures.find(texturePath);
    if(search == mTextures.end()){
        sf::Texture texture;
        if(!texture.loadFromFile(texturePath)){
            throw std::runtime_error("Failed to load texture: " + texturePath);
        }
        search = mTextures.emplace(texturePath, std::move(texture)).first;
    } 

    sf::Sprite sprite(search->second);
    sprite.setTextureRect(sf::IntRect({static_cast<int>(sourceRect.topLeft.x), static_cast<int>(sourceRect.topLeft.y)},
                                      {static_cast<int>(sourceRect.width), static_cast<int>(sourceRect.height)}));
    mSprites.push_back(sprite);
    return mSprites.size() - 1;
}                                  
                                    

sf::Sprite& SpriteManager::GetSprite(SpriteID id) {
    return mSprites.at(id);
}

}  // namespace ECSEngine
