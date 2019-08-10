#include "SpriteSheetLoader.h"

#include "SpriteSheet.h"
#include "CharStates.h"
#include "EffectTypes.h"
#include "JSONUtilities.h"
#include "SpriteAnimation.h"

#include <json/single_include/nlohmann/json.hpp>

using json = nlohmann::json;

namespace
{
	const int NUM_STREAMS_REQUIRED{ 2 };

	const std::string PROPERTY_CLIPSIZE{ "clipSize" };
	const std::string PROPERTY_X{ "x" };
	const std::string PROPERTY_Y{ "y" };
	const std::string PROPERTY_ANIMATIONS{ "animations" };
	const std::string PROPERTY_ANIM_TYPE{ "type" };
	const std::string PROPERTY_ANIM_FIRST{ "firstIndex" };
	const std::string PROPERTY_ANIM_NUM{ "numSprites" };
	const std::string PROPERTY_ANIM_LOOP{ "isLooping" };
	const std::string PROPERTY_ANIM_OFFSET{ "offset" };
	const std::string PROPERTY_ANIM_DURATION{ "durations" };
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
	glm::ivec2 clipSize;
	std::unordered_map<std::string, SpriteAnimation> anims;
	loadAnimations(sheetResult, clipSize, anims);

	std::shared_ptr<SpriteSheet> spriteSheet{ 
		std::make_shared<SpriteSheet>(textureId, width, height, 
		numChannels, anims, clipSize, name) };

	if (spriteSheet != nullptr)
	{
		std::cout << "SpriteSheetLoader::loadFromStream: Loaded '" << name << "'\n" << std::endl;
		return spriteSheet;
	}

	return nullptr;
}

void SpriteSheetLoader::loadAnimations(const IDataStream::Result &streamedData,
	glm::ivec2 &clipSize,
	std::unordered_map<std::string, SpriteAnimation> &anims) const
{
	try
	{
		// Read the contents into json.
		json j;
		*(streamedData.stream) >> j;

		// Get clip size.
		if (JSONUtilities::hasEntry(PROPERTY_CLIPSIZE, j))
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

		// Get animations.
		if (JSONUtilities::hasEntry(PROPERTY_ANIMATIONS, j))
		{
			json animJson{ j.at(PROPERTY_ANIMATIONS) };
			for (const auto &thisAnim : animJson)
			{
				SpriteAnimation anim;

				// Type property.
				std::string type;
				if (JSONUtilities::hasEntry(PROPERTY_ANIM_TYPE, thisAnim))
				{
					type = thisAnim.at(PROPERTY_ANIM_TYPE).get<std::string>();
				}
				else
				{
					// No type attribute, so skip this.
					continue;
				}

				// First index property.
				if (JSONUtilities::hasEntry(PROPERTY_ANIM_FIRST, thisAnim))
				{
					anim.firstIndex = thisAnim.at(PROPERTY_ANIM_FIRST).get<int>();
				}

				// Number of sprites property.
				if (JSONUtilities::hasEntry(PROPERTY_ANIM_NUM, thisAnim))
				{
					anim.numSprites = thisAnim.at(PROPERTY_ANIM_NUM).get<int>();
				}

				// Looping property.
				if (JSONUtilities::hasEntry(PROPERTY_ANIM_LOOP, thisAnim))
				{
					anim.isLooping = thisAnim.at(PROPERTY_ANIM_LOOP).get<bool>();
				}

				// Offset property.
				if (JSONUtilities::hasEntry(PROPERTY_ANIM_OFFSET, thisAnim))
				{
					json offsetJson{ thisAnim.at(PROPERTY_ANIM_OFFSET) };
					if (JSONUtilities::hasEntry(PROPERTY_X, offsetJson))
					{
						anim.offset.x = offsetJson.at(PROPERTY_X).get<float>();
					}

					if (JSONUtilities::hasEntry(PROPERTY_Y, offsetJson))
					{
						anim.offset.y = offsetJson.at(PROPERTY_Y).get<float>();
					}
				}

				// Durations property.
				if (JSONUtilities::hasEntry(PROPERTY_ANIM_DURATION, thisAnim))
				{
					anim.durations = thisAnim.at(PROPERTY_ANIM_DURATION).get<std::vector<float>>();
				}

				// Create the animation and insert it.
				anims.insert({ type, anim });
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