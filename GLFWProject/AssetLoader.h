#pragma once
#ifndef AssetLoader_H
#define AssetLoader_h

#include "IDataStream.h"
#include "ITypeLoader.h"

#include <json/include/nlohmann/json_fwd.hpp>

#include <memory>
#include <unordered_map>
#include <string>
#include <typeindex>

class IAssetType;

class AssetLoader
{
public:
	AssetLoader(IDataStream *stream);

	// Register a loader with a given type key.
	template <typename T>
	void registerLoader(ITypeLoader *loader);

	// Load assets by their file paths, through its
	// relevant loader.
	template <typename T>
	std::shared_ptr<T> load(const std::string &name, 
		const std::vector<std::string> &filePaths,
		int flag = 0);

	template <typename T>
	std::shared_ptr<T> load(const std::string &name, int flag = 0);

	void update(float deltaTime);

private:
	// Load list for assets of a given type.
	void loadAssetList(nlohmann::json json, const std::string &assetLabel, 
		const std::type_index &assetType);

	// Hold registered loaders for all asset types.
	std::unordered_map<std::type_index, std::unique_ptr<ITypeLoader>> m_loaders;

	// Hold the stream to get data from.
	std::unique_ptr<IDataStream> m_stream;

	// 2-dimensional map to hold a list of all asset files.
	// First key is asset type. Second key is asset name, which is mapped to
	// a list of file paths for that asset.
	std::unordered_map<std::type_index, std::unordered_map<std::string, std::vector<std::string>>> m_assetsList;
};

template <typename T>
void AssetLoader::registerLoader(ITypeLoader *loader)
{
	std::type_index type{ std::type_index(typeid(T)) };
	m_loaders.insert({ type, std::unique_ptr<ITypeLoader>(loader) });
}

template <typename T>
std::shared_ptr<T> AssetLoader::load(const std::string &name,
	const std::vector<std::string> &filePaths, int flag)
{
	std::type_index type{ std::type_index(typeid(T)) };
	auto it{ m_loaders.find(type) };
	if (it == m_loaders.end())
	{
		std::cout << "AssetLoader::load: loader type not found for: " << name << std::endl;
		return nullptr;
	}

	// Check if there are the correct number of file paths.
	int numStreamsReq{ it->second->getNumStreamsRequired() };
	int numFiles{ static_cast<int>(filePaths.size()) };
	if (numFiles != numStreamsReq)
	{
		std::cout << "AssetLoader::load: loader for: '" << name << 
			"' requires " << numStreamsReq << " files (given: " <<
			numFiles << ")" << std::endl;
		return nullptr;
	}

	// Check the cache for the asset.
	// If it already exists, then return it.
	std::shared_ptr<IAssetType> asset{ it->second->loadFromCache(name, flag) };
	if (asset != nullptr)
		return std::dynamic_pointer_cast<T>(asset);

	// Get the stream and pass it to the loader.
	std::vector<IDataStream::Result> streams;
	m_stream->getStream(filePaths, streams);

	// Downcast result to the actual type.
	return std::dynamic_pointer_cast<T>(
		it->second->load(streams, name, flag));
}

template <typename T>
std::shared_ptr<T> AssetLoader::load(const std::string &name, int flag)
{
	// Get the file paths from the assets list.
	std::type_index type{ std::type_index(typeid(T)) };
	auto typeIt{ m_assetsList.find(type) };
	if (typeIt == m_assetsList.end())
	{
		std::cout << "AssetLoader::load: type listing not found for: " << name << std::endl;
		return nullptr;
	}

	auto assetIt{ typeIt->second.find(name) };
	if (assetIt == typeIt->second.end())
	{
		std::cout << "AssetLoader::load: asset listing not found for: " << name << std::endl;
		return nullptr;
	}

	return load<T>(name, assetIt->second, flag);
}

#endif