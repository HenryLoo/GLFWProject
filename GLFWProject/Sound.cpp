#include "Sound.h"

#include <SoLoud/soloud.h>
#include <SoLoud/soloud_wav.h>

#include <iostream>

Sound::Sound(SoLoud::Wav *wav)
{
	m_sound = std::unique_ptr<SoLoud::Wav>(wav);
}

Sound::~Sound()
{

}

void Sound::play(SoLoud::Soloud &engine) const
{
	engine.play(*m_sound);
}