#include "ITypeLoader.h"

#include <glm/glm.hpp>

#include <string>

namespace
{
	// Time in seconds between checking to clear cache.
	const float TIME_TO_CLEAR_CACHE{ 120.f };
}

std::shared_ptr<IAssetType> ITypeLoader::loadFromCache(
	const std::string &name, int flag)
{
	// Check the cache for the asset.
	// If it already exists, then return it.
	std::string flaggedName;
	getFlaggedName(name, flag, flaggedName);
	auto it{ m_cache.find(flaggedName) };
	if (it != m_cache.end())
	{
		std::cout << "ITypeLoader::load: Loaded '" << flaggedName << "' from cache" << std::endl;
		return it->second;
	}

	return nullptr;
}

std::shared_ptr<IAssetType> ITypeLoader::load(
	const std::vector<IDataStream::Result> &streams,
	const std::string &name, int flag)
{
	std::shared_ptr<IAssetType> asset{ loadFromStream(streams, name, flag) };

	// Cache the loaded asset if valid.
	if (asset != nullptr)
	{
		std::string flaggedName;
		getFlaggedName(name, flag, flaggedName);
		m_cache.insert({ flaggedName, asset });
	}

	return asset;
}

int ITypeLoader::getNumStreamsRequired() const
{
	return m_numStreamsRequired;
}


void ITypeLoader::getFlaggedName(const std::string &name, int flag,
	std::string &output) const
{
	output = name;
	if (flag > 0)
	{
		output += ("/" + std::to_string(flag));
	}
}

void ITypeLoader::update(float deltaTime)
{
	m_clearCacheTime += deltaTime;
	m_clearCacheTime = glm::min(TIME_TO_CLEAR_CACHE, m_clearCacheTime);

	if (m_clearCacheTime == TIME_TO_CLEAR_CACHE)
	{
		for (auto it = m_cache.begin(); it != m_cache.end(); )
		{
			// If use_count is 1, then it is only being owned by the cache.
			if (it->second.unique())
			{
				//std::cout << "ITypeLoader::update: '" << it->first << "' deleted from cache" << std::endl;
				it = m_cache.erase(it);
			}
			else
				++it;
		}

		// Reset the timer.
		m_clearCacheTime = 0.f;
	}
}