#pragma once
#include <memory>
#include "Game.h"

struct EvaluationTreeNode
{
	struct EvaluationTreeNodeTransition
	{
		std::shared_ptr<EvaluationTreeNode> node;
		Move move; 
	};

	int value;
	int depth;

	std::vector<EvaluationTreeNodeTransition> children;

	void AddChild(EvaluationTreeNode* node, const Move& move)
	{
		node->depth = depth + 1;
		children.push_back({ std::shared_ptr<EvaluationTreeNode>(node), move });
	}

	const EvaluationTreeNodeTransition* FindMove(const Move& move) const
	{
		for (const EvaluationTreeNodeTransition& transition : children)
		{
			if (transition.move == move)
			{
				return &transition;
			}
		}

		return nullptr;
	}

	void CopyChildren(const EvaluationTreeNode* node)
	{
		/* Copy constructor of shared_ptr will make this a second counted references.
		   It NEEDS to be like this. Raw C-ptr as source will lead to UB.
		   
		   This will also fuck up the depth hierarchy, but this should be no problem, since everything is calculated and parity is preserved anyways */
		children = node->children;
	}

	int CountRecursive() const
	{
		int count = 1;
		for (const EvaluationTreeNodeTransition& transition : children)
		{
			count += transition.node->CountRecursive();
		}

		return count;
	}

	~EvaluationTreeNode()
	{
		children.clear();
	}


};

class EvaluationTree
{
public:
	EvaluationTree() : Root(nullptr) {
	/* Init position cache */
	/* Cache for the position
	* 6 positions for each king (can't approach other king)
	* 4 positions for each knight (only one field color) + 1 for taken
	* 6 positions for each rook (-1 due to own king to the left, -1 due to enemy king to the right, no possibility to "overtake"), + 1 for taken
	* 2 in case there is same position with different turn
	* 2 for for repetition counter (third repetition does not need cache, since it is detected as draw)
	* Divide by 4 as we can store 4 eval results in one byte (3 different values for eval + 1 for not evaluated = 2 bits)
	* [6 * 5 * 7 * 7 * 5 * 6 * 2 * 2 / 4 = 44,100]
	*/
		m_positionCache = std::make_unique<unsigned char[]>(44100);
		m_cacheStat = std::make_unique<unsigned short[]>(44100 * 4);

		std::fill_n(m_positionCache.get(), 44100, 0);
	}
	~EvaluationTree() = default;

	/* Evaluate the position to the end */
	int Evaluate(const GameState& state);

	/* Returns evaluation for game state */
	int GetGameStateEvaluation(const GameState& state);

	std::unique_ptr<EvaluationTreeNode> Root;

private:

	enum class CachedEvaluation
	{
		Unknown,
		WhiteWins,
		Draw,
		BlackWins
	};

	int EvaluateRecursive(const GameState& state, EvaluationTreeNode* node);

	/* Cache for the position */
	std::unique_ptr<unsigned char[]> m_positionCache;

	std::unique_ptr<unsigned short[]> m_cacheStat;

	/* get cache entry */
	CachedEvaluation GetCacheEntry(const GameState& state);
	void SetCacheEntry(const GameState& state, CachedEvaluation value);

	/* Cache index calculation */
	/* Compute unambiguous value for a certain position/state */
	int GetPositionIndex(const GameState& state);

	/* Computes index in cache from position index */
	int GetCacheIndex(int positionIndex);

	/* Computes index of 2-bit group inside byte */
	int GetIntraByteIndex(int positionIndex);

	/* Some nice stats */
	int m_highestDepth = 0;
	int m_cacheHits = 0;
	int m_cacheSaved = 0;
};
