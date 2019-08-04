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
	virtual void getStream(const std::vector<std::string> &filePaths,
		std::vector<IDataStream::Result> &output);

private:
	// Clear existing stream.
	void clearStream();

	std::vector<IDataStream::Result>m_streams;
};

#endif