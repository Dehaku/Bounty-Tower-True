#include "Sounds.h"

#include "filesystemUtils.hpp"
#include <iostream>

SoundManager soundmanager;
//MusicManager musicmanager;

sf::SoundBuffer &SoundManager::getSound(std::string input)
{
    for (size_t i = 0; i != sounds.size(); i++)
    {
        if (sounds.at(i).name == input)
        {
            return sounds.at(i).soundstorage;
        }
    }
    for (size_t i = 0; i != sounds.size(); i++)
    {
        if (sounds.at(i).name == "Error.bmp")
        {
            return sounds.at(i).soundstorage;
        }
    }
    throw std::runtime_error("GetSound: Couldn't find sound.");
}

void SoundManager::playSound(std::string input)
{
    for (size_t i = 0; i != sounds.size(); i++)
    {
        if (sounds.at(i).name == input)
        {
            auto sound = std::make_unique<sf::Sound>(sounds[i].soundstorage);
            sound->setVolume(gvars::soundVolume);
            playSounds.push_back(std::move(sound));
            playSounds.back()->play();
            return;
        }
    }
}

void SoundManager::cleanSounds()
{
    auto it = std::remove_if(playSounds.begin(), playSounds.end(), [](auto const & s) {
        return s->getStatus() == sf::Sound::Stopped;
    });
    playSounds.erase(it, playSounds.end());
}

void SoundManager::init()
{
    using namespace std;
    string dir = string("./data/sfx");
    vector<string> files = vector<string>();
    getdir(dir, files);
    for (auto &file : files)
    {
        string line("data/sfx/");
        string ending(file);
        if (file != "." && file != "..")
        {
            line.append(ending);
            SoundHolder sound;
            sound.soundstorage.loadFromFile(line);

            std::string namebit = file;
            sound.name = namebit;
            sounds.push_back(sound);
        }
    }
}

std::vector<MusicHolder*> musics;

void initializeMusic()
{
    using namespace std;
    string dir = string("./data/music");
    vector<string> files = vector<string>();
    getdir(dir, files);
    for (auto &file : files)
    {
        string line("data/music/");
        string ending(file);
        if (file != "." && file != "..")
        {
            line.append(ending);
            MusicHolder * music;
            music = new MusicHolder;
            music->musictrack.openFromFile(line);

            std::string namebit = file;
            music->name = namebit;
            musics.push_back(music);
        }
    }
}

void playMusic(std::string track)
{
    for (size_t i = 0; i != musics.size(); i++)
    {
        if (musics[i]->name == track)
        {
            musics[i]->musictrack.setVolume(gvars::musicVolume);
            musics[i]->musictrack.play();
            return;
        }
    }
}

