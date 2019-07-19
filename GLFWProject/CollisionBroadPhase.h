#pragma once
#ifndef CollisionBroadPhase_H
#define CollisionBroadPhase_H

#include "AABB.h"
#include "GameEngine.h"

#include <set>
#include <vector>

// Uses a sort and sweep approach to detect collisions.
class CollisionBroadPhase
{
	// An AABB augmented with details about its source.
	struct AABBSource
	{
		enum class Type
		{
			Collision,
			Attack
		};

		// The id of the entity that this endpoint belongs to.
		int entityId;

		// The type of the endpoint: collision box or attack hit box.
		Type type;

		// The position of the AABB.
		glm::vec2 pos;

		// The scale of the AABB.
		glm::vec2 scale;

		AABB aabb;
	};

	class Endpoint
	{
	public:
		// Defines whether the endpoint is a minimum or maximum point.
		enum class Type
		{
			Minimum,
			Maximum
		};

		void init(int aabbIndex, Type type)
		{
			m_aabbIndex = aabbIndex;
			m_type = type;
			m_value = -1.f;
		}

		void setValue(float val)
		{
			m_value = val;
		}

		int getAABBIndex() const
		{
			return m_aabbIndex;
		}

		int getLookupIndex() const
		{
			return 2 * m_aabbIndex + (m_type == Type::Maximum ? 1 : 0);
		}

		Type getType() const
		{
			return m_type;
		}

		float getValue() const
		{
			return m_value;
		}

		bool operator < (const Endpoint &other) const
		{
			if (m_value == other.m_value)
			{
				return m_type < other.getType();
			}
			else
			{
				return m_value < other.getValue();
			}
		};

	private:
		// The endpoint's index in the list of AABB's.
		int m_aabbIndex;

		// The position value of the endpoint in one axis.
		float m_value;

		Type m_type;
	};

	class Event
	{
	public:
		enum Type
		{
			Remove,
			Insert
		};

		Event(int index1, int index2, Type type) :
			m_index1(index1), m_index2(index2), m_type(type)
		{

		};

		Type getType() const
		{
			return m_type;
		}

		int getIndex1() const
		{
			return m_index1;
		}

		int getIndex2() const
		{
			return m_index2;
		}

	private:
		Type m_type;
		int m_index1;
		int m_index2;
	};

public:
	// Update the list of AABB's.
	// colIds contains a list of entity IDs that have collision components.
	// atkIds contains a list of entity IDs that have attack components.
	void updateAABBList(
		const GameComponent::Collision(&cols)[GameEngine::MAX_ENTITIES],
		const GameComponent::Attack(&atks)[GameEngine::MAX_ENTITIES], 
		const GameComponent::Physics(&phys)[GameEngine::MAX_ENTITIES],
		const std::vector<int> colIds, const std::vector<int> atkIds);

	// Generate the list of overlapping endpoints.
	// The resulting list is passed into the output param.
	void generateOverlapList(std::vector<std::pair<int, int>> &output);

private:
	// Update the position values in the list of endpoints with the values of the
	// list of AABB's.
	void updateEndpoints();

	// Sort the endpoints and produce events to update the set of
	// overlapping intervals. Intervals are defined by the area bounded
	// by their 2 endpoints in one axis.
	void updateIntervals(std::vector<Endpoint> &endpoints, std::vector<int> &lookup);

	// Check if two endpoints are overlapping.
	bool isOverlapping(int endpointIndex1, int endpointIndex2,
		const std::vector<int> &lookup, const std::vector<Endpoint> &endpoints);

	// An unordered list of all AABB's to consider.
	std::vector<AABBSource> m_aabbList;

	// Hold all endpoints in the x and y-directions.
	// Endpoints refer to the bounds of an AABB, projected onto an axis.
	std::vector<Endpoint> m_endpointsX;
	std::vector<Endpoint> m_endpointsY;

	// Lookup tables for endpoints. This allows reverse searching an entity from
	// its endpoint.
	std::vector<int> m_lookupX;
	std::vector<int> m_lookupY;

	// Hold a list of actions to perform on the set of overlapping endpoints.
	// Endpoints may be removed or inserted.
	std::vector<Event> m_events;

	// Set of overlapping endpoints.
	std::set<std::pair<int, int>> m_overlapsSet;
};

#endif