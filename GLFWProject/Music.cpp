#include "Music.h"

#include <SoLoud/soloud.h>
#include <SoLoud/soloud_wavstream.h>

#include <iostream>

Music::Music(SoLoud::WavStream *wav)
{
	m_music = std::unique_ptr<SoLoud::WavStream>(wav);
	m_music->setLooping(true);
}

Music::~Music()
{

}

void Music::play(SoLoud::Soloud &engine)
{
	if (m_music != nullptr && !m_isPlaying)
	{
		// TODO: change fixed volume.
		const float VOLUME{ 0.08f };
		engine.playBackground(*m_music, VOLUME);
		m_isPlaying = true;
	}
}

void Music::stop()
{
	if (m_music != nullptr && m_isPlaying)
	{
		m_music->stop();
		m_isPlaying = false;
	}
}