//#pragma once
//
//#include <array>
//#include <vector>
//#include <queue>
//#include <algorithm>
//#include <cmath>
//#include <limits>
//#include <cstddef>
//
////Tog bort grid specific data
////difference is distance now, floats are used
//
//namespace CommonUtilities
//{
//
//
//	namespace AStarInternal
//	{
//		enum class NodeStatus
//		{
//			Unvisited,
//			Open,
//			Closed
//		};
//
//		struct PathNodeData
//		{
//			int parent;
//			float g;
//			float h;
//			NodeStatus status;
//
//			PathNodeData()
//				: parent(-1)
//				, g((std::numeric_limits<float>::max)())
//				, h(0)
//				, status(NodeStatus::Unvisited)
//			{
//			}
//		};
//
//		struct OpenEntry
//		{
//			int index;
//			float f;
//			float g;
//
//			OpenEntry()
//				: index(-1)
//				, f(0)
//				, g(0)
//			{
//			}
//
//			OpenEntry(const int aIndex, const float aF, const float aG)
//				: index(aIndex)
//				, f(aF)
//				, g(aG)
//			{
//			}
//		};
//
//		struct CompareOpenEntry
//		{
//			bool operator()(const OpenEntry& aLeft, const OpenEntry& aRight) const
//			{
//				if (aLeft.f == aRight.f)
//				{
//					return aLeft.g > aRight.g;
//				}
//
//				return aLeft.f > aRight.f;
//			}
//		};
//
//		template <typename TNodes>
//		inline float Heuristic(const TNodes& aNodes, const int aFromIndex, const int aToIndex)
//		{
//			/*const auto& from = aNodes[aFromIndex].position;
//			const auto& to = aNodes[aToIndex].position;*/
//
//			const float x = aNodes[aFromIndex].position.X;
//			const float y = aNodes[aToIndex].position.Y;
//
//			return std::sqrt(x * x + y * y); //distance between nodes
//		}
//
//
//		inline std::vector<int> ReconstructPath(const std::vector<PathNodeData>& aNodes, const int aStartIndex, const int anEndIndex)
//		{
//			std::vector<int> path;
//			int current = anEndIndex;
//
//			while (current != -1)
//			{
//				path.push_back(current);
//
//				if (current == aStartIndex)
//				{
//					break;
//				}
//
//				current = aNodes[static_cast<std::size_t>(current)].parent;
//			}
//
//			if (path.empty() || path.back() != aStartIndex)
//			{
//				return std::vector<int>();
//			}
//
//			std::reverse(path.begin(), path.end());
//			return path;
//		}
//	}
//
//	template <typename TNodes>
//	inline std::vector<int> AStarTemplate(const TNodes& aNodes, const int aStartIndex, const int anEndIndex)
//	{
//		using namespace AStarInternal;
//
//		if (aStartIndex < 0 || anEndIndex < 0 || aStartIndex >= static_cast<int>(aNodes.size()) || anEndIndex >= static_cast<int>(aNodes.size()))
//		{
//			return std::vector<int>(); //does the triangle exist?
//		}
//
//		if (aStartIndex == anEndIndex)
//		{
//			return std::vector<int>(1, aStartIndex);
//		}
//
//		std::vector<PathNodeData> nodes(aNodes.size());
//		std::priority_queue<OpenEntry, std::vector<OpenEntry>, CompareOpenEntry> openSet;
//
//		nodes[static_cast<std::size_t>(aStartIndex)].g = 0;
//		nodes[static_cast<std::size_t>(aStartIndex)].h = Heuristic(aNodes, aStartIndex, anEndIndex);
//		nodes[static_cast<std::size_t>(aStartIndex)].parent = -1;
//		nodes[static_cast<std::size_t>(aStartIndex)].status = NodeStatus::Open;
//
//		openSet.push(OpenEntry(
//			aStartIndex,
//			nodes[static_cast<std::size_t>(aStartIndex)].g + nodes[static_cast<std::size_t>(aStartIndex)].h,
//			nodes[static_cast<std::size_t>(aStartIndex)].g));
//
//		while (!openSet.empty())
//		{
//			const OpenEntry currentEntry = openSet.top();
//			openSet.pop();
//
//			const int currentIndex = currentEntry.index;
//			PathNodeData& currentNode = nodes[static_cast<std::size_t>(currentIndex)];
//
//			if (currentNode.status == NodeStatus::Closed)
//			{
//				continue;
//			}
//
//			if (currentEntry.f != currentNode.g + currentNode.h)
//			{
//				continue;
//			}
//
//			if (currentIndex == anEndIndex)
//			{
//				return ReconstructPath(nodes, aStartIndex, anEndIndex);
//			}
//
//			currentNode.status = NodeStatus::Closed;
//
//
//			for (const unsigned neighbour : aNodes[currentIndex].connectingNodes)
//			{
//				const int neighbourIndex = static_cast<int>(neighbour);
//
//				PathNodeData& neighbourNode = nodes[static_cast<std::size_t>(neighbourIndex)];
//
//				if (neighbourNode.status == NodeStatus::Closed)
//				{
//					continue;
//				}
//
//				const auto& currentPosition = aNodes[currentIndex].position;
//				const auto& neighbourPosition = aNodes[neighbourIndex].position;
//
//				const float x = neighbourPosition.x - currentPosition.x;
//				const float y = neighbourPosition.y - currentPosition.y; //each node has a position, so we can calculate the distance between them
//
//				const float distance = std::sqrt(x * x + y * y);
//
//				const float tentativeG = currentNode.g + distance;
//				if (neighbourNode.status == NodeStatus::Unvisited)
//				{
//					neighbourNode.h = Heuristic(aNodes, neighbourIndex, anEndIndex);
//				}
//
//				if (tentativeG < neighbourNode.g)
//				{
//					neighbourNode.g = tentativeG;
//					neighbourNode.parent = currentIndex;
//					neighbourNode.status = NodeStatus::Open;
//
//					openSet.push(OpenEntry(
//						neighbourIndex,
//						neighbourNode.g + neighbourNode.h,
//						neighbourNode.g));
//				}
//			}
//		}
//
//		return std::vector<int>();
//	}
//
//
//	template <typename TNodes>
//	inline std::vector<int> AStar(const TNodes& aNodes, int aStartIndex, int anEndIndex)
//	{
//		return AStarTemplate(aNodes, aStartIndex, anEndIndex);
//	}
//}
