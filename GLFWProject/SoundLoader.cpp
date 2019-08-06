#include "SoundLoader.h"

#include "Sound.h"

#include <SoLoud/soloud.h>
#include <SoLoud/soloud_wav.h>

namespace
{
	const int NUM_STREAMS_REQUIRED{ 1 };
}

int SoundLoader::getNumStreamsRequired() const
{
	return NUM_STREAMS_REQUIRED;
}

std::shared_ptr<IAssetType> SoundLoader::load(
	const std::vector<IDataStream::Result> &streams,
	const std::string &name)
{
	const IDataStream::Result &theResult{ streams[0] };
	int length{ theResult.length };
	char *buffer{ new char[length] };
	theResult.stream->read(buffer, length);

	SoLoud::Wav *wav{ new SoLoud::Wav() };
	if (wav != nullptr)
		wav->loadMem((unsigned char *)buffer, length, true);

	delete buffer;

	if (wav != nullptr)
	{
		std::shared_ptr<Sound> sound{
			std::make_shared<Sound>(wav) };
		if (sound != nullptr)
		{
			std::cout << "SoundLoader::load: Loaded '" << name << "'\n" << std::endl;
			return sound;
		}
	}

	return nullptr;
}