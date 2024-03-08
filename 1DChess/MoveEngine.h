#pragma once

#include "Position.h"
#include <vector>

/* Move struct */
struct Move
{
	int from;
	int to;
};

class MoveEngine
{
	MoveEngine();
	~MoveEngine();

public:
	/* Generate all legal moves for a position */
	static std::vector<Move> GenerateLegalMoves(const Position& position);
};