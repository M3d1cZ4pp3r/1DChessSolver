#pragma once

#include <iostream>

constexpr int BOARD_SIZE = 8;

/* Enum for pieces */
enum class PieceType
{
	None,
	Rook,
	Knight,
	King,
	NumPieces
};

/* Enum for colors */
enum class Color
{
	White,
	Black,
	NumColors
};

/* Piece with color */
enum class Piece
{
	None,
	WhiteRook,
	WhiteKnight,
	WhiteKing,
	BlackRook,
	BlackKnight,
	BlackKing
};



/* Board state for 1D-chess */
class Board
{
public:
	Board();
	Board(const Board& position);
	Board& operator=(const Board& position);
	~Board();

	/* Comparison for threefold repetition */
	bool operator==(const Board& position) const;

	/* Get piece at position */
	Piece GetPiece(int position) const;

	/* Set piece at position */
	void SetPiece(int position, Piece piece);

	/* Get color */
	Color GetColor(int position) const;

	/* Get piece type */
	PieceType GetPieceType(int position) const;

	/* Is free */
	bool IsFree(int position) const;

	/* Is own piece */
	bool IsOwnPiece(int position, Color ownColor) const;

	/* Is enemy piece */
	bool IsEnemyPiece(int position, Color ownColor) const;

	/* Is position on board */
	bool IsOnBoard(int position) const;

	/* Set to starting position */
	void SetStartingPosition();

	/* Static generator for starting board */
	static Board GetStartingPosition();

	/* Stream operator to human readably print */
	friend std::ostream& operator<<(std::ostream& os, const Board& position);

private:
	Piece m_board[BOARD_SIZE];

};