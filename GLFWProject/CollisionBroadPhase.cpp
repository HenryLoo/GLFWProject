#include "CollisionBroadPhase.h"

#include <iostream>

CollisionBroadPhase::CollisionBroadPhase()
{
	// Initialize AABB list's size.
	m_aabbList.resize(EntityConstants::MAX_ENTITIES);
}

void CollisionBroadPhase::updateAABBList(int numEntities,
	const std::vector<unsigned long> &entities,
	const std::vector<GameComponent::Collision> &cols,
	const std::vector<GameComponent::Attack> &atks,
	const std::vector<GameComponent::Physics> &phys)
{
	int totalSize{ 0 };
	for (int i = 0; i < numEntities; ++i)
	{
		bool hasPhysics{ GameComponent::hasComponent(entities[i], 
			GameComponent::COMPONENT_PHYSICS) };
		bool hasCollision{ GameComponent::hasComponent(entities[i], 
			GameComponent::COMPONENT_COLLISION) };
		bool hasAttack{ GameComponent::hasComponent(entities[i], 
			GameComponent::COMPONENT_ATTACK) };
		if (!hasPhysics) continue;

		if (hasCollision)
		{
			AABBData &data = m_aabbList[totalSize];
			data.src.entityId = i;
			data.src.type = AABBSource::Type::Collision;
			data.pos = phys[i].pos;
			data.scale = phys[i].scale;
			data.aabb = cols[i].aabb;
			++totalSize;
		}

		if (hasAttack)
		{
			AABBData &data = m_aabbList[totalSize];
			data.src.entityId = i;
			data.src.type = AABBSource::Type::Attack;
			data.pos = phys[i].pos;
			data.scale = phys[i].scale;
			data.aabb = atks[i].pattern.aabb;
			++totalSize;
		}
	}

	// Update the size of the AABB list.
	m_aabbList.resize(totalSize);

	// Update endpoints if new points were added or points were removed.
	int numPoints{ 2 * totalSize };
	if (m_endpointsX.size() != numPoints)
	{
		// Hold the endpoints of all intervals. Each interval has 2 endpoints.
		m_endpointsX.resize(2 * totalSize);
		m_endpointsY.resize(2 * totalSize);

		// Update endpoint based on the AABB list.
		int aabbIndex{ 0 }, endpointIndex{ 0 };
		for (auto it = m_aabbList.begin(); it != m_aabbList.end(); ++it, ++aabbIndex)
		{
			m_endpointsX[endpointIndex].init(aabbIndex, Endpoint::Type::Minimum);
			m_endpointsY[endpointIndex].init(aabbIndex, Endpoint::Type::Minimum);
			++endpointIndex;

			m_endpointsX[endpointIndex].init(aabbIndex, Endpoint::Type::Maximum);
			m_endpointsY[endpointIndex].init(aabbIndex, Endpoint::Type::Maximum);
			++endpointIndex;
		}

		// Update lookup tables.
		m_lookupX.resize(numPoints);
		m_lookupY.resize(numPoints);
		endpointIndex = 0;
		for (auto it = m_endpointsX.begin(); it != m_endpointsX.end(); ++it, ++endpointIndex)
		{
			int index{ it->getLookupIndex() };
			m_lookupX[index] = endpointIndex;
			m_lookupY[index] = endpointIndex;
		}
	}
}


void CollisionBroadPhase::generateOverlapList(
	std::vector<std::pair<AABBSource, AABBSource>> &output)
{
	updateEndpoints();

	// Repopulate list of events to perform.
	m_events.clear();
	updateIntervals(m_endpointsX, m_lookupX);
	updateIntervals(m_endpointsY, m_lookupY);

	// Iterate through events and perform them to get an updated overlaps set.
	for (const Event &event : m_events)
	{
		if (event.getType() == Event::Remove)
		{
			m_overlapsSet.erase(std::make_pair(event.getIndex1(), event.getIndex2()));
			//std::cout << "remove " << event.getIndex1() << " " << event.getIndex2() << ", size: " << m_overlapsSet.size() << std::endl;
		}
		// Insert event.
		else
		{
			m_overlapsSet.insert(std::make_pair(event.getIndex1(), event.getIndex2()));
			//std::cout << "insert " << event.getIndex1() << " " << event.getIndex2() << ", size: " << m_overlapsSet.size() << std::endl;
		}
	}

	// Output the AABBSource of the overlapping endpoints.
	for (const std::pair<int, int> &overlap : m_overlapsSet)
	{
		output.push_back(std::make_pair(
			m_aabbList[overlap.first].src,
			m_aabbList[overlap.second].src));
	}
}

void CollisionBroadPhase::updateEndpoints()
{
	int aabbIndex{ 0 };
	for (auto it = m_aabbList.begin(); it != m_aabbList.end(); ++it, ++aabbIndex)
	{
		int idMin{ 2 * aabbIndex };
		int idMax{ idMin + 1 };
		m_endpointsX[m_lookupX[idMin]].setValue(it->pos.x - it->aabb.halfSize.x 
			+ it->scale.x * it->aabb.offset.x);
		m_endpointsY[m_lookupY[idMin]].setValue(it->pos.y - it->aabb.halfSize.y
			+ it->scale.y * it->aabb.offset.y);
		m_endpointsX[m_lookupX[idMax]].setValue(it->pos.x + it->aabb.halfSize.x
			+ it->scale.x * it->aabb.offset.x);
		m_endpointsY[m_lookupY[idMax]].setValue(it->pos.y + it->aabb.halfSize.y
			+ it->scale.y * it->aabb.offset.y);
	}
}

void  CollisionBroadPhase::updateIntervals(std::vector<Endpoint> &endpoints,
	std::vector<int> &lookup)
{
	//std::cout << "---" << std::endl;
	for (int i = 0; i < endpoints.size(); ++i)
	{
		// Sort the endpoints by value, in increasing order.
		Endpoint thisPoint{ endpoints[i] };
		//std::string type{ thisPoint.getType() == Endpoint::Type::Minimum ? "min" : "max" };
		//std::cout << thisPoint.getAABBIndex() << " " << type << " " << thisPoint.getValue() << std::endl;
		int j{ i - 1 };
		while (j >= 0 && thisPoint < endpoints[j] &&
			thisPoint.getValue() != EntityConstants::ENDPOINT_NOT_SET)
		{
			Endpoint prevPoint{ endpoints[j] };
			Endpoint nextPoint{ endpoints[j + 1] };

			// No overlap between intervals, so remove it from the overlaps set.
			if (prevPoint.getType() == Endpoint::Type::Minimum &&
				nextPoint.getType() == Endpoint::Type::Maximum)
			{
				int index1{ glm::min(prevPoint.getAABBIndex(), nextPoint.getAABBIndex()) };
				int index2{ glm::max(prevPoint.getAABBIndex(), nextPoint.getAABBIndex()) };
				if (index1 != index2)
				{
					m_events.push_back(Event(index1, index2, Event::Remove));
				}
			}
			// Overlap found between intervals.
			else if (prevPoint.getType() == Endpoint::Type::Maximum &&
				thisPoint.getType() == Endpoint::Type::Minimum)
			{
				int index1{ glm::min(prevPoint.getAABBIndex(), nextPoint.getAABBIndex()) };
				int index2{ glm::max(prevPoint.getAABBIndex(), nextPoint.getAABBIndex()) };
				if (index1 != index2 &&
					isOverlapping(index1, index2, m_lookupX, m_endpointsX) &&
					isOverlapping(index1, index2, m_lookupY, m_endpointsY))
				{
					m_events.push_back(Event(index1, index2, Event::Insert));
				}
			}

			// Swap the endpoints.
			endpoints[j] = nextPoint;
			endpoints[j + 1] = prevPoint;
			lookup[nextPoint.getLookupIndex()] = j;
			lookup[prevPoint.getLookupIndex()] = j + 1;
			--j;
		}

		endpoints[j + 1] = thisPoint;
		lookup[thisPoint.getLookupIndex()] = j + 1;
	}
}

bool CollisionBroadPhase::isOverlapping(int index1, int index2,
	const std::vector<int> &lookup, const std::vector<Endpoint> &endpoints)
{
	float max1 = endpoints[lookup[2 * index1 + 1]].getValue();
	float min2 = endpoints[lookup[2 * index2]].getValue();

	// No overlap if the maximum endpoint of the first interval is at a lower
	// position than the minimum endpoint of the second interval.
	if (max1 < min2)
	{
		return false;
	}

	float min1 = endpoints[lookup[2 * index1]].getValue();
	float max2 = endpoints[lookup[2 * index2 + 1]].getValue();
	return min1 <= max2;
}