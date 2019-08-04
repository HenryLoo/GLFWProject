#include "DiskStream.h"

namespace
{
	std::string DATA_FILE{ "data.hl" };
}

DiskStream::DiskStream()
{
	// Initialize PhysFS.
	PhysFS::init(nullptr);
	PhysFS::mount(DATA_FILE, "", true);
}

DiskStream::~DiskStream()
{
	PhysFS::deinit();
	clearStream();
}

void DiskStream::getStream(const std::vector<std::string> &filePaths,
	std::vector<IDataStream::Result> &output)
{
	clearStream();

	for (const std::string &path : filePaths)
	{
		try
		{
			PhysFS::fstream *stream{ new PhysFS::fstream(path) };
			int size{ static_cast<int>(stream->length()) };
			m_streams.push_back({ stream, size });
			std::cout << "DiskStream::getStream: opened '" << path << "', size: " 
				<< size << std::endl;
		}
		catch (const std::exception &e)
		{
			std::cout << "DiskStream::getStream: " << e.what() << std::endl;
		}
	}

	output = m_streams;
}

void DiskStream::clearStream()
{
	for (auto str : m_streams)
	{
		if (str.stream != nullptr)
			delete str.stream;
	}

	m_streams.clear();
}