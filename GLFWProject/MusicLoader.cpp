#include "MusicLoader.h"

#include "Music.h"

#include <SoLoud/soloud.h>
#include <SoLoud/soloud_wavstream.h>

std::shared_ptr<IAssetType> MusicLoader::loadFromStream(
	const std::vector<IDataStream::Result> &streams,
	const std::string &name, int flag)
{
	const IDataStream::Result &theResult{ streams[0] };
	int length{ theResult.length };
	char *buffer{ new char[length] };
	theResult.stream->read(buffer, length);

	SoLoud::WavStream *wav{ new SoLoud::WavStream() };
	if (wav != nullptr)
		wav->loadMem((unsigned char *)buffer, length, true);

	delete[] buffer;

	if (wav != nullptr)
	{
		std::shared_ptr<Music> music{
			std::make_shared<Music>(wav) };
		if (music != nullptr)
		{
			std::cout << "MusicLoader::loadFromStream: Loaded '" << name << "'\n" << std::endl;
			return music;
		}
	}

	return nullptr;
}