#include "SoundLoader.h"

#include "Sound.h"

#include <SoLoud/soloud.h>
#include <SoLoud/soloud_wav.h>

std::shared_ptr<IAssetType> SoundLoader::loadFromStream(
	const std::vector<IDataStream::Result> &streams,
	const std::string &name, int flag)
{
	const IDataStream::Result &theResult{ streams[0] };
	int length{ theResult.length };
	char *buffer{ new char[length] };
	theResult.stream->read(buffer, length);

	SoLoud::Wav *wav{ new SoLoud::Wav() };
	if (wav != nullptr)
		wav->loadMem((unsigned char *)buffer, length, true);

	delete[] buffer;

	if (wav != nullptr)
	{
		std::shared_ptr<Sound> sound{
			std::make_shared<Sound>(wav) };
		if (sound != nullptr)
		{
			std::cout << "SoundLoader::loadFromStream: Loaded '" << name << "'\n" << std::endl;
			return sound;
		}
	}

	return nullptr;
}