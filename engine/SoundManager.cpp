#include "SoundManager.h"

namespace ECSEngine {

void SoundManager::RegisterSound(const std::string& soundPath, const std::string& soundName) {
    sf::SoundBuffer buffer;
    if(!buffer.loadFromFile(soundPath)){
        throw std::runtime_error("Failed to load sound: " + soundPath);
    }

    mSoundBuffers.insert_or_assign(soundName, std::move(buffer));

    mSounds.insert_or_assign(soundName, sf::Sound(mSoundBuffers.at(soundName)));
}

sf::Sound& SoundManager::GetSound(const std::string& soundName) {
    auto it = mSounds.find(soundName);

    return it->second;
}

}  // namespace ECSEngine
