#include "RoomCollisionSystem.h"

#include "PlayState.h"
#include "Room.h"

namespace
{
	// Distance in pixels to subtract from collision boxes to
	// prevent overlapping between horizontal and vertical tests.
	const float COLLISION_THRESHOLD{ 1.f };
}

RoomCollisionSystem::RoomCollisionSystem(EntityManager &manager,
	std::vector<GameComponent::Physics> &physics,
	std::vector<GameComponent::Collision> &collisions,
	std::vector<GameComponent::Character> &characters) :
	GameSystem(manager, { GameComponent::COMPONENT_PHYSICS,
		GameComponent::COMPONENT_COLLISION }),
	m_physics(physics), m_collisions(collisions), 
	m_characters(characters)
{

}

void RoomCollisionSystem::process(float deltaTime, int entityId,
	unsigned long &entityMask)
{
	// Skip this if the entity is hit stopped.
	GameComponent::Character &character{ m_characters[entityId] };
	if (GameComponent::hasHitStop(entityMask, character))
		return;

	// Skip this if the current room has not been initialized.
	Room *room{ PlayState::instance()->getCurrentRoom() };
	if (room == nullptr)
		return;

	GameComponent::Physics &phys{ m_physics[entityId] };
	GameComponent::Collision &col{ m_collisions[entityId] };

	// Reset collision flags.
	col.wasOnGround = GameComponent::isOnGround(phys, col);
	col.isCollidingFloor = false;
	col.isCollidingGhost = false;
	col.isCollidingHorizontal = false;
	
	const AABB &aabb{ col.aabb };
	glm::vec2 speed{ phys.speed };
	glm::vec2 halfSize{ aabb.halfSize };
	glm::vec2 pos{ phys.pos.x + aabb.offset.x,
		phys.pos.y + aabb.offset.y };
	
	// Check for horizontal collisions.
	if (speed.x != 0)
	{
		// Extend the halfsize of the entity to see how much distance it will
		// cover this frame.
		float distX = halfSize.x + abs(speed.x) * deltaTime;
		glm::ivec2 currentTile{ room->getTileCoord(pos) };
	
		// 1 = moving left, -1 = moving right.
		int direction{ speed.x < 0 ? 1 : -1 };
	
		// Get the vertical tile bounds at the maximum X-distance.
		float maxDistX{ pos.x - direction * distX };
		float halfSizeY{ halfSize.y - COLLISION_THRESHOLD };
		glm::vec2 minYPos{ maxDistX, pos.y - halfSizeY };
		glm::vec2 maxYPos{ maxDistX, pos.y + halfSizeY };
		glm::ivec2 minYTile{ room->getTileCoord(minYPos) };
		glm::ivec2 maxYTile{ room->getTileCoord(maxYPos) };
	
		// If the entity is on a slope, ignore horizontal collisions for the
		// bottom-most tile. This will stop the collision box from
		// "catching" onto the tile when transitioning from slope onto floor.
		if (col.isCollidingSlope)
		{
			minYTile.y = glm::min(minYTile.y + 1, maxYTile.y);
		}
	
		// Set up bounds for the loop.
		// minYTile.x == maxYTile.x should be true.
		glm::ivec2 tileRangeToCheck{ currentTile.x, minYTile.x };
		tileRangeToCheck.y -= direction;
	
		// Check all potential collisions before applying velocity.
		// We check in order of closest to furthest tiles.
		int currentTileX{ tileRangeToCheck.x };
		while (currentTileX != tileRangeToCheck.y)
		{
			// Check all tiles at this height.
			for (int i = minYTile.y; i <= maxYTile.y; ++i)
			{
				glm::ivec2 thisTileCoord{ currentTileX, i };
				RoomData::TileType type{ room->getTileType(thisTileCoord) };
				if (type == RoomData::TILE_WALL)
				{
					float tileEdgePos{ room->getTilePos(thisTileCoord).x
						+ direction * Room::TILE_SIZE / 2.f };
					phys.pos.x = tileEdgePos + direction * halfSize.x - aabb.offset.x;
					col.isCollidingHorizontal = true;
	
					// Collision was found, so there is no need to keep checking.
					break;
				}
			}
	
			// Collision was found, so there is no need to keep checking. 
			if (col.isCollidingHorizontal)
				break;
	
			currentTileX -= direction;
		}
	
		// If not colliding, then just apply velocity as usual.
		if (!col.isCollidingHorizontal)
			phys.pos.x += phys.speed.x * deltaTime;

		float roomWidth{ static_cast<float>(room->getSize().x * Room::TILE_SIZE) };
		phys.pos.x = glm::clamp(phys.pos.x, 0.f, roomWidth);
	}
	
	// Reset the colliding-slope flag after checking for horizontal collisions,
	// because we need to check it to properly transition between slopes and
	// floor tiles.
	col.isCollidingSlope = false;
	
	// Check for vertical collisions.
	if (speed.y != 0 || !phys.hasGravity)
	{
		// Extend the halfsize of the entity to see how much distance it will
		// cover this frame.
		float distY = halfSize.y + abs(speed.y) * deltaTime;
		glm::ivec2 currentTile{ room->getTileCoord(pos) };
	
		// 1 = moving down, -1 = moving up.
		int direction{ speed.y <= 0 ? 1 : -1 };
	
		// Get the horizontal tile bounds at the maximum Y-distance.
		float maxDistY{ pos.y - direction * distY };
		float halfSizeX{ halfSize.x - COLLISION_THRESHOLD };
		glm::vec2 minXPos{ pos.x - halfSizeX, maxDistY };
		glm::vec2 maxXPos{ pos.x + halfSizeX, maxDistY };
		glm::ivec2 minXTile{ room->getTileCoord(minXPos) };
		glm::ivec2 maxXTile{ room->getTileCoord(maxXPos) };
	
		// Set up bounds for the loop.
		// minXTile.y == maxXTile.y should be true.
		glm::ivec2 tileRangeToCheck{ currentTile.y, minXTile.y };
		tileRangeToCheck.y -= direction;
	
		// Check all potential collisions before applying velocity.
		// We check in order of closest to furthest tiles.
		int currentTileY{ tileRangeToCheck.x };
	
		// The entity's new y-position if it has a vertical collision.
		float newYPos{ -1 };
	
		// Flag for if the entity has almost fallen onto a slope.
		// Since slopes are shorter than regular tlies, we need this flag to check if
		// the entity is within the slope's tile, but not yet colliding with it.
		bool isAlmostSlope{ false };
	
		while (currentTileY != tileRangeToCheck.y)
		{
			// Check all tiles at this height.
			for (int i = minXTile.x; i <= maxXTile.x; ++i)
			{
				glm::ivec2 thisTileCoord{ i, currentTileY };
				RoomData::TileType type{ room->getTileType(thisTileCoord) };
	
				// The edge of the tile to check entity collision against.
				glm::vec2 thisTilePos{ room->getTilePos(thisTileCoord) };
				const static int tileHalfSize{ Room::TILE_SIZE / 2 };
				float tileEdgePos{ thisTilePos.y + direction * tileHalfSize };
	
				// Check for collisions against slopes first.
				if (Room::isSlope(type) && speed.y <= 0)
				{
					// Get the distance from the left edge of the tile to the entity's position.
					float slopeRad{ atanf((float)Room::SLOPE_HEIGHT / Room::TILE_SIZE) };
					float xDist{ phys.pos.x - (thisTilePos.x - tileHalfSize) };
	
					// Not close enough onto the slope yet.
					if (xDist < 0 || xDist > Room::TILE_SIZE)
					{
						continue;
					}
	
					// Adjust y-distance to displace the entity, based on the type of slope.
					float yDist{ tanf(slopeRad) * xDist };
	
					if (type == RoomData::TILE_SLOPE_LEFT_UPPER || type == RoomData::TILE_SLOPE_LEFT_LOWER)
						yDist = Room::TILE_SIZE - yDist;
	
					if (type == RoomData::TILE_SLOPE_RIGHT_UPPER)
						yDist += Room::SLOPE_HEIGHT;
					else if (type == RoomData::TILE_SLOPE_LEFT_LOWER)
						yDist -= Room::SLOPE_HEIGHT;
	
					tileEdgePos = thisTilePos.y - tileHalfSize + yDist;
	
					// Slopes are shorter than regular tiles, so we need to check more precisely
					// for collisions. The -1 helps with 1-pixel off errors.
					if (((phys.pos.y - halfSize.y + aabb.offset.y + speed.y * deltaTime) - 1.f) > tileEdgePos)
					{
						isAlmostSlope = true;
						continue;
					}
	
					col.isCollidingSlope = true;
	
					// Set the new y-position if it isn't set already, or if it is at a lower
					// y-position than the current one to set.
					float yPosToSet = tileEdgePos + halfSize.y - aabb.offset.y;
					newYPos = newYPos == -1 ? yPosToSet : glm::min(newYPos, yPosToSet);
				}
				// If the tile is a wall or the entity is colliding against the
				// top edge of a ghost platform.
				// Unlike horizontal collisions, we don't break early here because
				// we need to set all possible collision flags.
				else if (type == RoomData::TILE_WALL || (type == RoomData::TILE_GHOST && speed.y <= 0 &&
					(phys.pos.y - halfSize.y + aabb.offset.y) >= tileEdgePos))
				{
					// Set the new y-position if it isn't set already, or if it is at a lower
					// y-position than the current one to set.
					float yPosToSet = tileEdgePos + direction * halfSize.y - aabb.offset.y;
					newYPos = newYPos == -1 ? yPosToSet : glm::min(newYPos, yPosToSet);
	
					if (type == RoomData::TILE_WALL)
					{
						col.isCollidingFloor = true;
					}
					else if (type == RoomData::TILE_GHOST)
					{
						col.isCollidingGhost = true;
					}
				}
			}
	
			// Collision was found, so there is no need to keep checking. 
			if (GameComponent::isColliding(col))
			{
				// If the entity is within a slope's tile but not yet 
				// colliding with it, then discard any floor collisions that 
				// would have occurred. Any floor collisions would have been 
				// at the same tile y-position as the slope. Since slopes are 
				// shorter than regular tiles, this would incorrectly set the 
				// new y-position to the floor, even though it should be at the
				// slope.
				if (isAlmostSlope && !col.isCollidingSlope)
				{
					col.isCollidingFloor = false;
				}
	
				break;
			}
	
			currentTileY -= direction;
		}
	
		// If not colliding, then just apply velocity as usual.
		if (!GameComponent::isColliding(col))
		{
			phys.pos.y += phys.speed.y * deltaTime;
		}
		// Colliding against ceiling.
		else if (col.isCollidingFloor && phys.speed.y > 0)
		{
			phys.speed.y = 0;
		}
		else
		{
			phys.pos.y = newYPos;
		}
	
		// Round to two decimal places to reduce sprite artifacts.
		phys.pos *= 100.f;
		phys.pos = glm::round(phys.pos);
		phys.pos /= 100.f;
	}
}