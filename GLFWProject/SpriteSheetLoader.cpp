#include "SpriteSheetLoader.h"

#include "SpriteSheet.h"
#include "CharStates.h"
#include "EffectTypes.h"

namespace
{
	// TODO: Initialize effects. Remove this later for more flexible approach.
	const std::unordered_map<std::string, SpriteAnimation> EFFECT_ANIMS{
		{EffectType::EVADE_SMOKE, { 0, 9, false, glm::vec2(0.f), {0.05f}}},
		{EffectType::HIT_SPARK, { 9, 4, false, glm::vec2(0.f), {0.05f}}},
	};
	const glm::ivec2 EFFECT_SIZE{ 64, 64 };
	const std::string EFFECT_NAME{ "effects" };

	const std::unordered_map<std::string, SpriteAnimation> PLAYER_ANIMS{
		{CharState::IDLE, { 0, 8, true, glm::vec2(0.f), {3.f, 0.07f, 0.07f, 0.07f, 0.07f, 1.f, 0.07f, 0.07f}}},
		{CharState::RUN, { 11, 10, true, glm::vec2(0.f), {0.07f} }},
		{CharState::JUMP_ASCEND, { 22, 4, false, glm::vec2(0.f), {0.07f} }},
		{CharState::JUMP_PEAK, { 26, 6, false, glm::vec2(0.f), {0.07f} }},
		{CharState::JUMP_DESCEND, { 33, 4, true, glm::vec2(0.f), {0.07f} }},
		{CharState::JUMP_LAND, { 37, 1, false, glm::vec2(0.f), {0.1f} }},
		{CharState::RUN_START, { 38, 5, false, glm::vec2(0.f), {0.07f} }},
		{CharState::RUN_STOP, { 44, 4, false, glm::vec2(0.f), {0.07f} }},
		{CharState::ALERT, { 48, 1, false, glm::vec2(0.f), {3.f} }},
		{CharState::ALERT_STOP, { 49, 2, false, glm::vec2(0.f), {0.07f} }},
		{CharState::TURN, { 51, 4, false, glm::vec2(0.f), {0.07f} }},
		{CharState::CROUCH, { 55, 4, false, glm::vec2(0.f), {0.07f} }},
		{CharState::CROUCH_STOP, { 59, 2, false, glm::vec2(0.f), {0.07f} }},
		{CharState::ATTACK, { 61, 10, false, glm::vec2(0.f), {0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.1f} }},
		{CharState::ATTACK_AIR, { 71, 9, false, glm::vec2(0.f), {0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.07f} }},
		{CharState::ATTACK_CROUCH, { 80, 9, false, glm::vec2(0.f), {0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.07f} }},
		{CharState::EVADE_START, { 89, 2, false, glm::vec2(0.f), {0.05f} }},
		{CharState::EVADE, { 91, 4, true, glm::vec2(0.f), {0.05f} }},
		{CharState::ATTACK2, { 95, 10, false, glm::vec2(0.f), {0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.1f} }},
		{CharState::ATTACK3, { 105, 11, false, glm::vec2(0.f), {0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.2f} }},
		{CharState::SKILL1, { 116, 8, false, glm::vec2(0.f), {0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.2f} }},
		{CharState::ATTACK_EVADE, { 124, 10, false, glm::vec2(0.f), {0.05f} }},
	};
	const glm::ivec2 PLAYER_SIZE{ 32, 32 };
	const std::string PLAYER_NAME{ "serah_sheet" };

	const std::unordered_map<std::string, SpriteAnimation> SWORD_ANIMS{
		{CharState::ATTACK, { 0, 10, false, glm::vec2(8.f, 8.f) }},
		{CharState::ATTACK_AIR, { 11, 9, false, glm::vec2(4.f, 8.f) }},
		{CharState::ATTACK_CROUCH, { 22, 9, false, glm::vec2(5.f, 0.f) }},
		{CharState::ATTACK_EVADE, { 33, 10, false, glm::vec2(10.f, -3.f) }},
		{CharState::ATTACK2, { 55, 10, false, glm::vec2(5.f, 8.f) }},
		{CharState::ATTACK3, { 66, 11, false, glm::vec2(10.f, 8.f) }},
		{CharState::SKILL1, { 44, 8, false, glm::vec2(0.f, 8.f) }},
	};
	const glm::ivec2 SWORD_SIZE{ 48, 48 };
	const std::string SWORD_NAME{ "serah_sword" };

	const std::unordered_map<std::string, SpriteAnimation> ENEMY_ANIMS{
		{CharState::IDLE, { 0, 1, false, glm::vec2(0.f),  {1.f}}},
		{CharState::RUN, { 4, 4, true, glm::vec2(0.f), {0.07f} }},
		{CharState::ALERT, { 8, 1, false, glm::vec2(0.f), {1.f} }},
		{CharState::HURT, { 12, 2, false, glm::vec2(0.f), {0.07f} }},
		{CharState::HURT_AIR, { 16, 3, false, glm::vec2(0.f), {0.07f} }},
		{CharState::FALLEN, { 20, 1, false, glm::vec2(0.f), {1.f} }},
		{CharState::ATTACK, { 24, 4, false, glm::vec2(0.f), {0.05f} }},
	};
	const glm::ivec2 ENEMY_SIZE{ 32, 32 };
	const std::string ENEMY_NAME{ "clamper_sheet" };

	const glm::ivec2 TILESET_SIZE{ 16, 16 };
	const std::string TILESET_NAME{ "tileset" };

	const int NUM_STREAMS_REQUIRED{ 1 };
}

int SpriteSheetLoader::getNumStreamsRequired() const
{
	return NUM_STREAMS_REQUIRED;
}

std::shared_ptr<IAssetType>SpriteSheetLoader::load(
	const std::vector<IDataStream::Result> &streams,
	const std::string &name)
{
	// If successfully created texture, instantiate the asset and return it.
	GLuint textureId;
	GLint width, height, numChannels;
	const IDataStream::Result &theResult{ streams[0] };
	if (loadValues(theResult, textureId, width, height, numChannels))
	{
		// TODO: replace these hardcoded resources later.
		std::unordered_map<std::string, SpriteAnimation> anims;
		glm::ivec2 clipSize;

		if (name == EFFECT_NAME)
		{
			anims = EFFECT_ANIMS;
			clipSize = EFFECT_SIZE;
		}
		else if (name == PLAYER_NAME)
		{
			anims = PLAYER_ANIMS;
			clipSize = PLAYER_SIZE;
		}
		else if (name == SWORD_NAME)
		{
			anims = SWORD_ANIMS;
			clipSize = SWORD_SIZE;
		}
		else if (name == ENEMY_NAME)
		{
			anims = ENEMY_ANIMS;
			clipSize = ENEMY_SIZE;
		}
		else if (name == TILESET_NAME)
		{
			clipSize = TILESET_SIZE;
		}

		std::shared_ptr<SpriteSheet> spriteSheet{ 
			std::make_shared<SpriteSheet>(textureId, width, height, 
			numChannels, anims, clipSize, name) };

		if (spriteSheet != nullptr)
		{
			std::cout << "SpriteSheetLoader::load: Loaded '" << name << "'\n" << std::endl;
			return spriteSheet;
		}
	}

	return nullptr;
}