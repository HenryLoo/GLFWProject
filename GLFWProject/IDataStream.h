#pragma once
#ifndef IDataStream_H
#define IDataStream_H

#include <iostream>

class IDataStream
{
public:
	// Get file data via stream, given its type and name.
	virtual std::iostream *getStream(std::string type, std::string name) = 0;

	// Get the length of the streamed data.
	virtual int getLength() const = 0;
};

#endif