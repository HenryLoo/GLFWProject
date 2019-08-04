#pragma once
#ifndef IDataStream_H
#define IDataStream_H

#include <iostream>
#include <vector>

class IDataStream
{
public:
	struct Result
	{
		std::iostream *stream;
		int length;
	};

	// Get file data via stream, given its type and name.
	virtual void getStream(const std::vector<std::string> &filePaths, 
		std::vector<IDataStream::Result> &output) = 0;
};

#endif