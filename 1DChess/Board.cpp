#include "Board.h"

Board::Board()
{
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		m_board[i] = Piece::None;
	}
}

Board::Board(const Board& position)
{
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		m_board[i] = position.m_board[i];
	}
}

Board& Board::operator=(const Board& position)
{
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		m_board[i] = position.m_board[i];
	}
	return *this;
}

Board::~Board()
{
}

bool Board::operator==(const Board& position) const
{
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		if (m_board[i] != position.m_board[i])
		{
			return false;
		}
	}
	return true;
}

Piece Board::GetPiece(int position) const
{
	/* Range check */
	if (!IsOnBoard(position))
	{
		return Piece::None;
	}

	return m_board[position];
}

void Board::SetPiece(int position, Piece piece)
{
	/* Range check */
	if (!IsOnBoard(position))
	{
		return;
	}

	m_board[position] = piece;
}

void Board::SetStartingPosition()
{
	m_board[0] = Piece::WhiteKing;
	m_board[1] = Piece::WhiteKnight;
	m_board[2] = Piece::WhiteRook;
	m_board[3] = Piece::None;
	m_board[4] = Piece::None;
	m_board[5] = Piece::BlackRook;
	m_board[6] = Piece::BlackKnight;
	m_board[7] = Piece::BlackKing;
}

Board Board::GetStartingPosition()
{
	Board position;
	position.SetStartingPosition();
	return position;
}

std::ostream& operator<<(std::ostream& os, const Board& position)
{
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		switch (position.m_board[i])
		{
		case Piece::None:
			os << ".";
			break;
		case Piece::WhiteRook:
			os << "R";
			break;
		case Piece::WhiteKnight:
			os << "N";
			break;
		case Piece::WhiteKing:
			os << "K";
			break;
		case Piece::BlackRook:
			os << "r";
			break;
		case Piece::BlackKnight:
			os << "n";
			break;
		case Piece::BlackKing:
			os << "k";
			break;
		}
	}
	return os;
}

Color Board::GetColor(int position) const
{
	/* Range check */
	if (!IsOnBoard(position))
	{
		return Color::White;
	}

	switch (m_board[position])
	{
	case Piece::WhiteRook:
	case Piece::WhiteKnight:
	case Piece::WhiteKing:
		return Color::White;
	case Piece::BlackRook:
	case Piece::BlackKnight:
	case Piece::BlackKing:
		return Color::Black;
	default:
		return Color::White;
	}
}

PieceType Board::GetPieceType(int position) const
{
	/* Range check */
	if (!IsOnBoard(position))
	{
		return PieceType::None;
	}

	switch (m_board[position])
	{
	case Piece::WhiteRook:
	case Piece::BlackRook:
		return PieceType::Rook;
	case Piece::WhiteKnight:
	case Piece::BlackKnight:
		return PieceType::Knight;
	case Piece::WhiteKing:
	case Piece::BlackKing:
		return PieceType::King;
	default:
		return PieceType::None;
	}
}

bool Board::IsOnBoard(int position) const
{
	return position >= 0 && position < BOARD_SIZE;
}

bool Board::IsFree(int position) const
{
	/* Range check */
	if (!IsOnBoard(position))
	{
		return true;
	}

	return m_board[position] == Piece::None;
}

bool Board::IsOwnPiece(int position, Color color) const
{
	/* Range check */
	if (!IsOnBoard(position))
	{
		return false;
	}

	if (IsFree(position))
	{
		return false;
	}

	return GetColor(position) == color;
}

bool Board::IsEnemyPiece(int position, Color color) const
{
	/* Range check */
	if (!IsOnBoard(position))
	{
		return false;
	}

	if (IsFree(position))
	{
		return false;
	}

	return GetColor(position) != color;
}