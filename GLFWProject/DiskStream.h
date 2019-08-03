#pragma once
#ifndef DiskStream_H
#define DiskStream_H

#include "IDataStream.h"

#include <PhysFS++/physfs.hpp>

// Load data from the disk.
class DiskStream : public IDataStream
{
public:
	DiskStream();
	~DiskStream();

	// Get file data via stream, given its type and name.
	virtual std::iostream *getStream(std::string type, std::string name);

	// Get the length of the streamed data.
	virtual int getLength() const;

private:
	// Clear existing stream.
	void clearStream();

	PhysFS::fstream *m_stream;
};

#endif