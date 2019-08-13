#include "SpriteSheetLoader.h"

#include "CharStates.h"
#include "EffectTypes.h"
#include "JSONUtilities.h"

#include <json/single_include/nlohmann/json.hpp>

using json = nlohmann::json;

namespace
{
	const int NUM_STREAMS_REQUIRED{ 2 };

	const std::string PROPERTY_CLIPSIZE{ "clipSize" };
	const std::string PROPERTY_X{ "x" };
	const std::string PROPERTY_Y{ "y" };
	const std::string PROPERTY_SPRITES{ "sprites" };
	const std::string PROPERTY_TYPE{ "type" };
	const std::string PROPERTY_FIRSTINDEX{ "firstIndex" };
	const std::string PROPERTY_NUMSPRITES{ "numSprites" };
	const std::string PROPERTY_ISLOOPING{ "isLooping" };
	const std::string PROPERTY_OFFSET{ "offset" };
	const std::string PROPERTY_DURATIONS{ "durations" };
	const std::string PROPERTY_CLIPS{ "clips" };
	const std::string PROPERTY_TOPLEFT{ "topLeft" };
}

SpriteSheetLoader::SpriteSheetLoader()
{
	m_numStreamsRequired = NUM_STREAMS_REQUIRED;
}

std::shared_ptr<IAssetType>SpriteSheetLoader::loadFromStream(
	const std::vector<IDataStream::Result> &streams,
	const std::string &name, int flag)
{
	// If successfully created texture, instantiate the asset and return it.
	GLuint textureId;
	GLint width, height, numChannels;
	const IDataStream::Result &textureResult{ streams[0] };
	if (!loadValues(textureResult, textureId, width, height, numChannels))
	{
		return nullptr;
	}

	const IDataStream::Result &sheetResult{ streams[1] };
	std::unordered_map<std::string, SpriteSheet::SpriteSet> sprites;
	loadAnimations(sheetResult, width, height, sprites);

	std::shared_ptr<SpriteSheet> spriteSheet{ 
		std::make_shared<SpriteSheet>(textureId, width, height, 
		numChannels, sprites, name) };

	if (spriteSheet != nullptr)
	{
		std::cout << "SpriteSheetLoader::loadFromStream: Loaded '" << name << "'\n" << std::endl;
		return spriteSheet;
	}

	return nullptr;
}

void SpriteSheetLoader::loadAnimations(const IDataStream::Result& streamedData,
	GLint textureWidth, GLint textureHeight,
	std::unordered_map<std::string, SpriteSheet::SpriteSet>& sprites) const
{
	try
	{
		// Read the contents into json.
		json j;
		*(streamedData.stream) >> j;

		// Check if the sprite sheet has uniform clip size.
		bool hasUniformClipSize{ JSONUtilities::hasEntry(PROPERTY_CLIPSIZE, j) };
		glm::ivec2 clipSize{ 0 };
		if (hasUniformClipSize)
		{
			json sizeJson{ j.at(PROPERTY_CLIPSIZE) };
			if (JSONUtilities::hasEntry(PROPERTY_X, sizeJson))
			{
				clipSize.x = sizeJson.at(PROPERTY_X).get<int>();
			}
			if (JSONUtilities::hasEntry(PROPERTY_Y, sizeJson))
			{
				clipSize.y = sizeJson.at(PROPERTY_Y).get<int>();
			}
		}

		// Get sprites.
		if (JSONUtilities::hasEntry(PROPERTY_SPRITES, j))
		{
			const json &spritesJson{ j.at(PROPERTY_SPRITES) };
			if (!spritesJson.is_array())
			{
				std::cout << "SpriteSheetLoader::loadAnimations: '" << 
					PROPERTY_SPRITES << "' property is not an array" << std::endl;
				return;
			}

			for (const auto &thisSprite : spritesJson)
			{
				SpriteSheet::SpriteSet spriteSet;

				// Type property.
				std::string type;
				if (JSONUtilities::hasEntry(PROPERTY_TYPE, thisSprite))
				{
					type = thisSprite.at(PROPERTY_TYPE).get<std::string>();
				}
				else
				{
					// No type attribute, so skip this.
					continue;
				}

				// Looping property.
				if (JSONUtilities::hasEntry(PROPERTY_ISLOOPING, thisSprite))
				{
					spriteSet.isLooping = thisSprite.at(PROPERTY_ISLOOPING).get<bool>();
				}

				// Durations property.
				std::vector<float> durations;
				if (JSONUtilities::hasEntry(PROPERTY_DURATIONS, thisSprite))
				{
					durations = thisSprite.at(PROPERTY_DURATIONS).get<std::vector<float>>();
				}
				int numDurations{ static_cast<int>(durations.size()) };

				// If the sprite sheet has uniform clip size, check for the 
				// following properties.
				if (hasUniformClipSize)
				{
					// First index property.
					int firstIndex{ 0 };
					if (JSONUtilities::hasEntry(PROPERTY_FIRSTINDEX, thisSprite))
					{
						firstIndex = thisSprite.at(PROPERTY_FIRSTINDEX).get<int>();
					}

					// Number of sprites property.
					int numSprites{ 0 };
					if (JSONUtilities::hasEntry(PROPERTY_NUMSPRITES, thisSprite))
					{
						numSprites = thisSprite.at(PROPERTY_NUMSPRITES).get<int>();
					}

					// Offset property.
					glm::vec2 offset{ 0.f };
					if (JSONUtilities::hasEntry(PROPERTY_OFFSET, thisSprite))
					{
						const json &offsetJson{ thisSprite.at(PROPERTY_OFFSET) };
						if (JSONUtilities::hasEntry(PROPERTY_X, offsetJson))
						{
							offset.x = offsetJson.at(PROPERTY_X).get<float>();
						}

						if (JSONUtilities::hasEntry(PROPERTY_Y, offsetJson))
						{
							offset.y = offsetJson.at(PROPERTY_Y).get<float>();
						}
					}

					// Create the sprite clips.
					for (int i = 0; i < numSprites; ++i)
					{
						SpriteSheet::SpriteClip clip;
						clip.clipSize = clipSize;
						clip.offset = offset;

						// If there are not enough duration values, set this 
						// clip's duration to be equal to the last duration available.
						if (numDurations > 0)
						{
							int durationIndex{ glm::min(i, numDurations - 1) };
							clip.duration = durations[durationIndex];
						}

						int spriteIndex{ firstIndex + i };
						int numSpritesPerRow{ static_cast<int>(glm::max(1, textureWidth / clipSize.x)) };
						glm::vec2 rowColIndex{ spriteIndex % numSpritesPerRow, glm::floor(spriteIndex / numSpritesPerRow) };
						clip.topLeft = glm::ivec2(
							rowColIndex.x * clipSize.x, 
							(rowColIndex.y + 1) * clipSize.y);

						// Add the clip to the sprite set.
						spriteSet.clips.push_back(clip);
					}
				}
				// Otherwise, if each clip has its own defined properties.
				else if (JSONUtilities::hasEntry(PROPERTY_CLIPS, thisSprite))
				{
					const json &clipsJson = thisSprite.at(PROPERTY_CLIPS);
					if (!clipsJson.is_array())
					{
						continue;
					}

					int i{ 0 };
					for (const auto &thisClip : clipsJson)
					{
						SpriteSheet::SpriteClip clip;

						// Clip size property.
						if (JSONUtilities::hasEntry(PROPERTY_CLIPSIZE, thisClip))
						{
							const json &clipSizeJson{ thisClip.at(PROPERTY_CLIPSIZE) };
							if (JSONUtilities::hasEntry(PROPERTY_X, clipSizeJson))
							{
								clip.clipSize.x = clipSizeJson.at(PROPERTY_X).get<int>();
							}

							if (JSONUtilities::hasEntry(PROPERTY_Y, clipSizeJson))
							{
								clip.clipSize.y = clipSizeJson.at(PROPERTY_Y).get<int>();
							}
						}

						// Offset property.
						if (JSONUtilities::hasEntry(PROPERTY_OFFSET, thisClip))
						{
							const json &offsetJson{ thisClip.at(PROPERTY_OFFSET) };
							if (JSONUtilities::hasEntry(PROPERTY_X, offsetJson))
							{
								clip.offset.x = offsetJson.at(PROPERTY_X).get<float>();
							}

							if (JSONUtilities::hasEntry(PROPERTY_Y, offsetJson))
							{
								clip.offset.y = offsetJson.at(PROPERTY_Y).get<float>();
							}
						}

						// Top-left property.
						if (JSONUtilities::hasEntry(PROPERTY_TOPLEFT, thisClip))
						{
							const json &topLeftJson{ thisClip.at(PROPERTY_TOPLEFT) };
							if (JSONUtilities::hasEntry(PROPERTY_X, topLeftJson))
							{
								clip.topLeft.x = topLeftJson.at(PROPERTY_X).get<int>();
							}

							if (JSONUtilities::hasEntry(PROPERTY_Y, topLeftJson))
							{
								clip.topLeft.y = topLeftJson.at(PROPERTY_Y).get<int>() + clip.clipSize.y;
							}
						}

						// If there are not enough duration values, set this 
						// clip's duration to be equal to the last duration available.
						if (numDurations > 0)
						{
							int durationIndex{ glm::min(i, numDurations - 1) };
							clip.duration = durations[durationIndex];
						}

						// Add the clip to the sprite set.
						spriteSet.clips.push_back(clip);
						++i;
					}
				}

				// Insert the sprite set.
				sprites.insert({ type, spriteSet });
			}
		}
	}
	catch (const nlohmann::json::parse_error &e)
	{
		std::cout << "SpriteSheetLoader::loadAnimations: " << e.what() << std::endl;
	}
	catch (const nlohmann::json::invalid_iterator &e)
	{
		std::cout << "SpriteSheetLoader::loadAnimations: " << e.what() << std::endl;
	}
	catch (const nlohmann::json::type_error &e)
	{
		std::cout << "SpriteSheetLoader::loadAnimations: " << e.what() << std::endl;
	}
	catch (const nlohmann::json::out_of_range &e)
	{
		std::cout << "SpriteSheetLoader::loadAnimations: " << e.what() << std::endl;
	}
	catch (const nlohmann::json::other_error &e)
	{
		std::cout << "SpriteSheetLoader::loadAnimations: " << e.what() << std::endl;
	}
}