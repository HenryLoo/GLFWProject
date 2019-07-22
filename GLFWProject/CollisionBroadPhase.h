#pragma once
#ifndef CollisionBroadPhase_H
#define CollisionBroadPhase_H

#include "AABB.h"
#include "EntityConstants.h"
#include "GameComponent.h"

#include <set>
#include <vector>

// Uses a sort and sweep approach to detect collisions.
class CollisionBroadPhase
{
	struct AABBData
	{
		AABBSource src;

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
			m_value = EntityConstants::ENDPOINT_NOT_SET;
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
	CollisionBroadPhase();

	// Update the list of AABB's.
	// colIds contains a list of entity IDs that have collision components.
	// atkIds contains a list of entity IDs that have attack components.
	void updateAABBList(int numEntities,
		const std::vector<unsigned long> &entities,
		const std::vector<GameComponent::Collision> &cols,
		const std::vector<GameComponent::Attack> &atks,
		const std::vector<GameComponent::Physics> &phys);

	// Generate the list of overlapping endpoints.
	// The resulting list is passed into the output param.
	// This is the "sweep" phase of the sort/sweep procedure.
	void generateOverlapList(std::vector<std::pair<AABBSource, AABBSource>> &output);

private:
	// Update the position values in the list of endpoints with the values of the
	// list of AABB's.
	void updateEndpoints();

	// Sort the endpoints and produce events to update the set of
	// overlapping intervals. Intervals are defined by the area bounded
	// by their 2 endpoints in one axis.
	// This is the "sort" phase of the sort/sweep procedure.
	void updateIntervals(std::vector<Endpoint> &endpoints, std::vector<int> &lookup);

	// Check if two endpoints are overlapping.
	bool isOverlapping(int endpointIndex1, int endpointIndex2,
		const std::vector<int> &lookup, const std::vector<Endpoint> &endpoints);

	// An unordered list of all AABB's to consider.
	std::vector<AABBData> m_aabbList;

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