#pragma once
#ifndef Sound_H
#define Sound_H

#include "IAssetType.h"

#include <memory>

namespace SoLoud
{
	class Soloud;
	class Wav;
}

class Sound : public IAssetType
{
public:
	Sound(SoLoud::Wav *wav);
	~Sound();

	void play(SoLoud::Soloud &engine) const;

private:
	virtual void cleanup() {};

	std::unique_ptr<SoLoud::Wav> m_sound;
};

#endif