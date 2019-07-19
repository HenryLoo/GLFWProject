#include "CollisionBroadPhase.h"

void CollisionBroadPhase::updateAABBList(
	const GameComponent::Collision(&cols)[GameEngine::MAX_ENTITIES],
	const GameComponent::Attack(&atks)[GameEngine::MAX_ENTITIES],
	const GameComponent::Physics(&phys)[GameEngine::MAX_ENTITIES],
	const std::vector<int> colIds, const std::vector<int> atkIds)
{
	// Update values of existing AABB's.
	int totalSize{ static_cast<int>(colIds.size() + atkIds.size()) };
	m_aabbList.resize(totalSize);

	// Iterate through the collision and attack components to get their AABBs.
	int index{ 0 };
	for (int id : colIds)
	{
		AABBSource &src{ m_aabbList[index] };
		src.entityId = id;
		src.type = AABBSource::Type::Collision;
		src.pos = phys[id].pos;
		src.scale = phys[id].scale;
		src.aabb = cols[id].aabb;
		++index;
	}

	for (int id : atkIds)
	{
		AABBSource &src{ m_aabbList[index] };
		src.entityId = id;
		src.type = AABBSource::Type::Attack;
		src.pos = phys[id].pos;
		src.scale = phys[id].scale;
		src.aabb = atks[id].pattern.aabb;
		++index;
	}

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
			Endpoint &pointX{ m_endpointsX[endpointIndex] };
			Endpoint &pointY{ m_endpointsY[endpointIndex] };

			pointX.init(aabbIndex, Endpoint::Type::Minimum);
			pointY.init(aabbIndex, Endpoint::Type::Minimum);
			++endpointIndex;

			pointX.init(aabbIndex, Endpoint::Type::Maximum);
			pointY.init(aabbIndex, Endpoint::Type::Maximum);
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


void CollisionBroadPhase::generateOverlapList(std::vector<std::pair<int, int>> &output)
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
		}
		// Insert event.
		else
		{
			m_overlapsSet.insert(std::make_pair(event.getIndex1(), event.getIndex2()));
		}
	}

	// Output the overlapping endpoints.
	for (const std::pair<int, int> &overlap : m_overlapsSet)
	{
		output.push_back(overlap);
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
	for (int i = 0; i < endpoints.size(); ++i)
	{
		// Sort the endpoints by value, in increasing order.
		Endpoint thisPoint{ endpoints[i] };
		int j{ i - 1 };
		while (j >= 0 && thisPoint < endpoints[j])
		{
			Endpoint prevPoint{ endpoints[j] };
			Endpoint nextPoint{ endpoints[j + 1] };

			// No overlap between intervals, so remove it from the overlaps set.
			if (prevPoint.getType() == Endpoint::Type::Minimum &&
				nextPoint.getType() == Endpoint::Type::Maximum)
			{
				int prevIndex{ prevPoint.getAABBIndex() };
				int nextIndex{ nextPoint.getAABBIndex() };
				if (prevIndex != nextIndex)
				{
					m_events.push_back(Event(prevIndex, nextIndex, Event::Remove));
				}
			}
			// Overlap found between intervals.
			else if (prevPoint.getType() == Endpoint::Type::Maximum &&
				thisPoint.getType() == Endpoint::Type::Minimum)
			{
				int prevIndex{ prevPoint.getAABBIndex() };
				int nextIndex{ nextPoint.getAABBIndex() };
				if (prevIndex != nextIndex &&
					isOverlapping(prevIndex, nextIndex, m_lookupX, m_endpointsX) &&
					isOverlapping(prevIndex, nextIndex, m_lookupY, m_endpointsY))
				{
					m_events.push_back(Event(prevIndex, nextIndex, Event::Insert));
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