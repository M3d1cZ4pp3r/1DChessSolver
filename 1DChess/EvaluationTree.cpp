#include "EvaluationTree.h"


int EvaluationTree::Evaluate(const GameState& state)
{
    /* Evaluation strategy: 
       1. Iterate depth-first through the moves, creating nodes for each moves (Don't store gamestate in tree to be memory-efficient.). 
       2. If an end position is reached, rewind one step according to DFS rules and continue
       3. If you finish a non-leaf node, calculate the value according to minimax algorithm
       4. Return the value of the root node
   */

    /* Create the root node */
	Root = std::make_unique<EvaluationTreeNode>();
	Root->depth = 0;
	Root->value = 0;





	/* Evaluate the position */
	Root->value = EvaluateRecursive(state, Root.get());

	std::cout << "----------------" << std::endl;
	std::cout << "Evaluation stats:" << std::endl;
	std::cout << "Total node number: " << Root->CountRecursive() << std::endl;
	std::cout << "Highest depth: " << m_highestDepth << std::endl;
	std::cout << "Cache hits: " << m_cacheHits << std::endl;
	std::cout << "Saved node evaluations through caching: " << m_cacheSaved << std::endl;

	return Root->value;
}

int EvaluationTree::GetGameStateEvaluation(const GameState& state)
{
	CachedEvaluation eval = GetCacheEntry(state);
	switch (eval)
	{
		case CachedEvaluation::WhiteWins:
			return 1;
		case CachedEvaluation::BlackWins:
			return -1;
		case CachedEvaluation::Draw:
			return 0;
		default:
			return -2;
	}
}

int EvaluationTree::EvaluateRecursive(const GameState& state, EvaluationTreeNode* node)
{
    /* If the game is over, this is a leaf node. Return the value of the game */
	if (state.IsGameOver())
	{
		/* Stat: Check if depth record */
		if (node->depth > m_highestDepth)
		{
			m_highestDepth = node->depth;
		}

		if (state.IsMate())
		{
			return state.GetWinner() == Color::White ? 1 : -1;
		}
		else if (state.IsDraw())
		{
			return 0;
		}
	}

	/* Check if we already calculated that position */
	CachedEvaluation result = GetCacheEntry(state);
	if (result != CachedEvaluation::Unknown)
	{
		for (int i = 0; i < node->depth; i++)
		{
			std::cout << "  ";
		}
		std::cout << "Cache hit" << std::endl;
		m_cacheHits++;

		m_cacheSaved += m_cacheStat[GetPositionIndex(state)];

		switch (result)
		{
		case CachedEvaluation::WhiteWins:
			return 1;
		case CachedEvaluation::BlackWins:
			return -1;
		case CachedEvaluation::Draw:
			return 0;
		}
	}

	/* Enumerate moves */
	const std::vector<Move>& moves = state.GetMoves();
	for (const Move& move : moves)
	{
		/* Make the move */
		GameState newState = state;
		newState.MakeMove(move);
		newState.CalculateStateFull();

		/* Create a new node */
		EvaluationTreeNode* newNode = new EvaluationTreeNode();
		node->AddChild(newNode, move);

		for (int i = 0; i < node->depth; i++)
		{
			std::cout << "  ";
		}
		std::cout << "Recursing into move: " << newNode->depth << ". " << move << "      " << newState.GetBoard() << std::endl;

		newNode->value = EvaluateRecursive(newState, newNode);

		for (int i = 0; i < node->depth; i++)
		{
			std::cout << "  ";
		}
		std::cout << "Move " << newNode->depth << ". " << move << " has value " << newNode->value << std::endl;
	}

	/* Calculate the value of the node */
	if (node->depth % 2 == 0)
	{
		/* Maximize */
		int max = INT_MIN;
		for (EvaluationTreeNode::EvaluationTreeNodeTransition& child : node->children)
		{
			if (child.node->value > max)
			{
				max = child.node->value;
			}
		}
		node->value = max;
	}
	else
	{
		/* Minimize */
		int min = INT_MAX;
		for (EvaluationTreeNode::EvaluationTreeNodeTransition& child : node->children)
		{
			if (child.node->value < min)
			{
				min = child.node->value;
			}
		}
		node->value = min;
	}

	/* Store the value in the cache */
	CachedEvaluation value;
	if (node->value == 1)
	{
		value = CachedEvaluation::WhiteWins;
	}
	else if (node->value == -1)
	{
		value = CachedEvaluation::BlackWins;
	}
	else
	{
		value = CachedEvaluation::Draw;
	}
	SetCacheEntry(state, value);

	/* Also save how many nodes that saves in future */
	m_cacheStat[GetPositionIndex(state)] = node->CountRecursive() - 1;

	return node->value;
}

EvaluationTree::CachedEvaluation EvaluationTree::GetCacheEntry(const GameState& state)
{
	int positionIndex = GetPositionIndex(state);
	int cacheByte = m_positionCache[GetCacheIndex(positionIndex)];
	int bitIndex = GetIntraByteIndex(positionIndex);

	return static_cast<CachedEvaluation>((cacheByte >> (bitIndex * 2)) & 0b11);
}

void EvaluationTree::SetCacheEntry(const GameState& state, CachedEvaluation value)
{
	int positionIndex = GetPositionIndex(state);
	int cacheIndex = GetCacheIndex(positionIndex);
	int bitIndex = GetIntraByteIndex(positionIndex);

	unsigned char& cacheByte = m_positionCache[cacheIndex];
	cacheByte &= ~(0b11 << (bitIndex * 2));
	cacheByte |= (static_cast<unsigned char>(value) << (bitIndex * 2));
}

int EvaluationTree::GetPositionIndex(const GameState& state)
{
	enum Elements {
		WhiteKing,
		WhiteKnight,
		WhiteRook,
		BlackRook,
		BlackKnight,
		BlackKing,
		Turn,
		Repetition
	};

	int index = 0;

	/* Calculate unique index for position: Take index of single combinatory element and combine it with the remaining combination of possiblities. Add all elements up */

	/* Possibilities for every combinatory element */
	const int possibilities[] = {6, 5, 7, 7, 5, 6, 2, 2};
	/* Actual state of element. Populate for pieces with the value for "taken" */
	int elementIdentifiers[8] = {-1, 4, 6, 6, 4, 5, -1, -1};

	/* Accelerate by iterating over the board and calculate on occasion. Pieces not found have already the correct value initialized */
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		Piece piece = state.GetBoard().GetPiece(i);

		switch (piece)
		{
		case Piece::WhiteKing:
			/* Take actual position. 6 and 7 are not reachable */
			elementIdentifiers[WhiteKing] = i;
			break;
		case Piece::WhiteKnight:
			/* Division by 2 yields index [1, 3, 5, 7] -> [0, 1, 2, 3] */
			elementIdentifiers[WhiteKnight] = i / 2;
			break;
		case Piece::WhiteRook:
			/* Shift, because left corner is not reachable (right corner too) */
			elementIdentifiers[WhiteRook] = i - 1;
			break;
		case Piece::BlackRook:
			/* Shift, because left corner is not reachable (right corner too) */
			elementIdentifiers[BlackRook] = i - 1;
			break;
		case Piece::BlackKnight:
			/* Division by 2 yields index [0, 2, 4, 6] -> [0, 1, 2, 3] */
			elementIdentifiers[BlackKnight] = i / 2;
			break;
		case Piece::BlackKing:
			/* Shift by 2 since 0 and 1 are not reachable */
			elementIdentifiers[BlackKing] = i - 2;
			break;
		default:
			break;
		}
	}

	/* Turn */
	elementIdentifiers[Turn] = state.GetNextPlayer() == Color::White ? 0 : 1;

	/* Repetition: A third repetition should never land here */
	elementIdentifiers[Repetition] = state.GetRepetitionCount() - 1;

	/* Calculate index */
	for (int i = 0; i < 8; i++)
	{
		/* Calculate remaining possibilities according to order */
		int remaining = 1;
		for (int j = i + 1; j < 8; j++)
		{
			remaining *= possibilities[j];
		}

		index += elementIdentifiers[i] * remaining;
	}

	return index;
}

int EvaluationTree::GetCacheIndex(int positionIndex)
{
	/* We have 4 positions per byte stored (each 2 bit) */
	return positionIndex / 4;
}

int EvaluationTree::GetIntraByteIndex(int positionIndex)
{
	/* We have 4 positions per byte stored (each 2 bit) */
	return positionIndex % 4;
}
