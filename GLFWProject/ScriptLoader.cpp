#include "ScriptLoader.h"

#include "Script.h"

#include <sstream>

std::shared_ptr<IAssetType> ScriptLoader::loadFromStream(
	const std::vector<IDataStream::Result> &streams,
	const std::string &name, int flag)
{
	// Read the script string from the buffer.
	std::stringstream ss;
	ss << streams[0].stream->rdbuf();

	std::shared_ptr<Script> script{
		std::make_shared<Script>(ss.str()) };
	if (script != nullptr)
	{
		std::cout << "ScriptLoader::loadFromStream: Loaded '" << name << "'\n" << std::endl;
		return script;
	}

	return nullptr;
}