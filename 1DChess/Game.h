#pragma once
#include "Board.h"
#include <array>
#include <vector>


/* Move struct */
struct Move
{
	int from;
	int to;

	/* Which piece moves */
	PieceType piece;

	/* Stream output */
	friend std::ostream& operator<<(std::ostream& os, const Move& move)
	{
		/* Translation from PieceType to character */
		static const char* pieceNames[] = { "?", "R", "N", "K" };

		os << pieceNames[(int)move.piece] << move.to + 1;
		return os;
	}

	/* Comparison */
	bool operator==(const Move& other) const
	{
		return from == other.from && to == other.to && piece == other.piece;
	}
};

/* class which contains the rules and can calculate moves */
class GameState
{
public:

	GameState() : m_board(Board::GetStartingPosition()), m_nextPlayer(Color::White), m_bOwnInCheck(false), m_bEnemyInCheck(false), m_gameResult(GameResult::NotFinished), m_kingCount(0), m_notKingCount(0), m_repetitionCount(1)
	{
		CalculateValidity();
	}

	/* Calculate basic information about the state, mainly move candidates and checks and validity */
	void CalculateValidity();

	/* Finalize state calculation, including moves and mate (Mate not possible without all valid moves).
	* We need to make this public and have it called manually, so we don't get in a full evaluation when checking for mate */
	void CalculateStateFull();

	/* Get all possible moves */
	const std::vector<Move>& GetMoves() const;

	/* Make a move */
	void MakeMove(const Move& move);

	/* Make a move without checking for validity */
	void MakeMoveUnchecked(const Move& move);

	/* Check if the game is over */
	bool IsGameOver() const;

	/* Check if the game is a draw */
	bool IsDraw() const;

	/* Check if the game is won */
	bool IsMate() const;

	/* Get the winner */
	Color GetWinner() const;

	/* If the state is valid, i.e. no inverse check is present from an illegal move and both kings there */
	bool IsValidState() const { return (!m_bEnemyInCheck && m_kingCount == 2); }

	/* Returns the number of occurences of this position (1 = first time) */
	int GetRepetitionCount() const { return m_repetitionCount; }



	/* Getters */
	Board GetBoard() const { return m_board; }
	Color GetNextPlayer() const { return m_nextPlayer; }
	std::vector<Board> GetHistory() const { return m_history; }

private:

	/* Enum for game result */
	enum class GameResult
	{
		NotFinished,
		Draw,
		WhiteWon,
		BlackWon
	};

	/* Calculate enemy attacked fields */
	void CalculateAttackedFields();

	/* Calculate checks */
	void CalculateChecks();

	/* Calculate moves */
	void CalculateMoves();

	/* Calculate terminal states like mate */
	void CalculateGameResult();

	/* Calculates target fields for a piece on a given position */
	void CalculateTargetFields(int position, std::vector<int>& targetFields);

	Board m_board;
	Color m_nextPlayer;

	/* History of states, to check for 3 move rule */
	/* Currently this does not respect whose move it is */
	std::vector<Board> m_history;

	/* internal helper states */
	bool m_bOwnInCheck;
	bool m_bEnemyInCheck;

	/* Number of pieces cached - we just differentiate between king and not king */
	int m_kingCount;
	int m_notKingCount;

	/* 1 = first time etc. */
	int m_repetitionCount;

	GameResult m_gameResult;

	/* Move candidates */
	std::vector<Move> m_moveCandidates;
	std::vector<Move> m_moves;

	/* attacked fields, needed for check calculation */
	std::array<bool, BOARD_SIZE> m_ownAttackedFields;
	std::array<bool, BOARD_SIZE> m_enemyAttackedFields;
};