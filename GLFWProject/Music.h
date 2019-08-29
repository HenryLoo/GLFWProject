#pragma once
#ifndef Music_H
#define Music_H

#include "IAssetType.h"

#include <memory>

namespace SoLoud
{
	class Soloud;
	class WavStream;
}

class Music : public IAssetType
{
public:
	Music(SoLoud::WavStream *wav);
	virtual ~Music();

	void play(SoLoud::Soloud &engine);
	void stop();

private:
	std::unique_ptr<SoLoud::WavStream> m_music;

	// Flag for if this music is already playing.
	bool m_isPlaying{ false };
};

#endif