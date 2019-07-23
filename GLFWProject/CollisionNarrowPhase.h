// TODO: Unused narrow phase for collision detection.
// Could be used for different collision shapes like circles.
// Maybe delete this?

//#pragma once
//#ifndef CollisionNarrowPhase_H
//#define CollisionNarrowPhase_H
//
//#include <vector>
//
//// TODO: define these.
//struct Collision;
//
//class CollisionNarrowPhase
//{
//public:
//	CollisionNarrowPhase();
//
//	// Iterate through the list of overlapping endpoints and determine whether or
//	// not there are collisions.
//	const std::vector<Collision> &detectCollisions(
//		const std::vector<std::pair<int, int>> &overlappingList, 
//		const std::vector<int> &entities);
//
//private:
//	// Hold a list of all collisions.
//	std::vector<Collision> m_collisions;
//};
//
//#endif