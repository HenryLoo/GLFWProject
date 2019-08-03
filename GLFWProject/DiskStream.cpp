#include "DiskStream.h"

namespace
{
	std::string DATA_FILE{ "data.hl" };
	std::string SEPARATOR{ "/" };
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

std::iostream *DiskStream::getStream(std::string type, std::string name)
{
	clearStream();

	try
	{
		std::string filePath{ SEPARATOR + type + SEPARATOR + name };
		m_stream = new PhysFS::fstream(filePath);
		std::cout << "DiskStream::getStream: opened " << filePath << std::endl;
	}
	catch (std::invalid_argument e)
	{
		std::cout << "DiskStream::getStream: " << e.what() << std::endl;
	}

	return m_stream;
}

int DiskStream::getLength() const
{
	return static_cast<int>(m_stream->length());
}

void DiskStream::clearStream()
{
	if (m_stream != nullptr)
		delete m_stream;
}