#include "Game.h"


bool GameState::IsGameOver() const
{
	return IsDraw() || IsMate();
}

bool GameState::IsDraw() const
{
	return m_gameResult == GameResult::Draw;
}

bool GameState::IsMate() const
{
	return m_gameResult == GameResult::WhiteWon || m_gameResult == GameResult::BlackWon;
}

Color GameState::GetWinner() const
{
	return m_gameResult == GameResult::WhiteWon ? Color::White : Color::Black;
}


void GameState::CalculateMoves()
{
	m_moves.clear();
	/* A move candidate qualifies, when the resulting game state is valid */
	/* Iterate over all move candidates and "simulate" the position by advancing a gamestate copy */
	/* If the resulting game state is valid, the move candidate is valid */
	for (const Move& move : m_moveCandidates)
	{
		/* Create a copy of the current game state */
		GameState copy = *this;
		/* Make the move */
		copy.MakeMoveUnchecked(move);
		/* If the resulting game state is valid */
		if (copy.IsValidState())
		{
			/* Add the move to the list of valid moves */
			m_moves.push_back(move);
		}
	}
}

void GameState::CalculateAttackedFields()
{
	/* Clear the attacked fields */
	m_ownAttackedFields = {};
	m_enemyAttackedFields = {};
	m_moveCandidates.clear();
	m_kingCount = 0;
	m_notKingCount = 0;

	/* Iterate over all fields */
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		/* If the field is empty */
		if (m_board.IsFree(i))
		{
			/* Skip it */
			continue;
		}

		/* Count the cache for piece counts */
		if (m_board.GetPieceType(i) == PieceType::King)
		{
			m_kingCount++;
		}
		else
		{
			m_notKingCount++;
		}

		/* If the field has our piece */
		if (m_board.IsOwnPiece(i, m_nextPlayer))
		{
			/* Calculate the target fields */
			std::vector<int> targetFields;
			CalculateTargetFields(i, targetFields);

			/* Iterate over the target fields */
			for (int j = 0; j < targetFields.size(); j++)
			{
				/* Mark the field as attacked */
				m_ownAttackedFields[targetFields[j]] = true;

				/* Cache this as move candidate also */
				m_moveCandidates.push_back({ i, targetFields[j], m_board.GetPieceType(i) });
			}
		}
		else
		{
			/* Calculate the target fields */
			std::vector<int> targetFields;
			CalculateTargetFields(i, targetFields);

			/* Iterate over the target fields */
			for (int j = 0; j < targetFields.size(); j++)
			{
				/* Mark the field as attacked */
				m_enemyAttackedFields[targetFields[j]] = true;
			}
		}
	}
}

void GameState::CalculateChecks()
{
	/* Clear the checks */
	m_bOwnInCheck = false;
	m_bEnemyInCheck = false;

	Color color = m_nextPlayer;

	/* Iterate over all fields */
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		/* If the field is empty */
		if (m_board.IsFree(i))
		{
			/* Skip it */
			continue;
		}

		if (m_board.IsOwnPiece(i, color) && m_board.GetPieceType(i) == PieceType::King)
		{
			/* Check if attacked */
			if (m_enemyAttackedFields[i])
			{
				m_bOwnInCheck = true;
			}
		}
		else if (!m_board.IsOwnPiece(i, color) && m_board.GetPieceType(i) == PieceType::King)
		{
			/* Check if attacked */
			if (m_ownAttackedFields[i])
			{
				m_bEnemyInCheck = true;
			}
		}
	}
}

void GameState::CalculateGameResult()
{
	/* First: Check for mate
	* For that we check if the player is in chess and there are 0 moves possible. We can directly see a draw reason, if there are 0 moves and no check */
	m_gameResult = GameResult::NotFinished;

	/* Check for 0 moves - the most terminal condition */
	if (m_moves.size() == 0)
	{
		/* Check for mate */
		if (m_bOwnInCheck)
		{
			/* Checkmate */
			m_gameResult = m_nextPlayer == Color::White ? GameResult::BlackWon : GameResult::WhiteWon;
		}
		else
		{
			/* Stalemate */
			m_gameResult = GameResult::Draw;
		}
	}
	else if (m_notKingCount == 0)
	{
		/* Insufficient material */
		m_gameResult = GameResult::Draw;
	}
	else
	{
		/* We have that board right now, so start with 1 */
		int count = 1;
		/* Check for threefold repetition */
		for (const Board& board : m_history)
		{
			if (board == m_board)
			{
				count++;
			}
		}
		if (count >= 3)
		{
			m_gameResult = GameResult::Draw;
		}

		m_repetitionCount = count;
	}

}

void GameState::CalculateTargetFields(int position, std::vector<int>& targetFields)
{
	/* Early exit if the position is empty */
	if (m_board.GetPiece(position) == Piece::None)
	{
		return;
	}

	/* Get the piece type */
	PieceType type = m_board.GetPieceType(position);
	/* Get the color */
	Color color = m_board.GetColor(position);

	/* If the piece is a rook */
	if (type == PieceType::Rook)
	{
		/* Rook movement is one-dimensional. So we just need to walk to paths and stop if any piece is encountered */
		/* First path */
		for (int j = position + 1; j < BOARD_SIZE; j++)
		{
			/* If the position is empty or has an enemy piece */
			if (!m_board.IsOwnPiece(j, color))
			{
				/* Add the move */
				targetFields.push_back(j);
			}
			/* If the position has a piece */
			if (!m_board.IsFree(j))
			{
				/* Stop the path */
				break;
			}
		}
		/* Second path */
		for (int j = position - 1; j >= 0; j--)
		{
			/* If the position is empty or has an enemy piece */
			if (!m_board.IsOwnPiece(j, color))
			{
				/* Add the move */
				targetFields.push_back(j);
			}
			/* If the position has a piece */
			if (!m_board.IsFree(j))
			{
				/* Stop the path */
				break;
			}
		}
	}
	/* If the piece is a knight */
	else if (type == PieceType::Knight)
	{
		/* Knight moves 2 fields without getting blocked. So just check adjacent 2 possible fields. */
		/* Right field */
		if (m_board.IsOnBoard(position + 2))
		{
			/* Move is possible if not our piece */
			if (!m_board.IsOwnPiece(position + 2, color))
			{
				/* Add the move */
				targetFields.push_back(position + 2);
			}
		}
		/* Left field */
		if (m_board.IsOnBoard(position - 2))
		{
			/* Move is possible if not our piece */
			if (!m_board.IsOwnPiece(position - 2, color))
			{
				/* Add the move */
				targetFields.push_back(position - 2);
			}
		}

	}
	/* If the piece is a king */
	else if (type == PieceType::King)
	{
		/* King moves 1 field without getting blocked. So just check adjacent 1 possible fields. */
		/* Right field */
		if (m_board.IsOnBoard(position + 1))
		{
			/* Move is possible if not our piece */
			if (!m_board.IsOwnPiece(position + 1, color))
			{
				/* Add the move */
				targetFields.push_back(position + 1);
			}
		}
		/* Left field */
		if (m_board.IsOnBoard(position - 1))
		{
			/* Move is possible if not our piece */
			if (!m_board.IsOwnPiece(position - 1, color))
			{
				/* Add the move */
				targetFields.push_back(position - 1);
			}
		}
	}
}

void GameState::CalculateValidity()
{
	/* We need those two to compute validity. Computing the moves would lead to recursion */
	CalculateAttackedFields();
	CalculateChecks();
}

void GameState::CalculateStateFull()
{
	/* Compute remaining stuff, requiring computation of next level of game states */
	CalculateMoves();
	CalculateGameResult();
}

const std::vector<Move>& GameState::GetMoves() const
{
	return m_moves;
}

void GameState::MakeMove(const Move& move)
{
	/* Early exit if the game is over */
	if (IsGameOver())
	{
		return;
	}

	/* Exit if the move is not possible */
	if (std::find(m_moves.begin(), m_moves.end(), move) == m_moves.end())
	{
		return;
	}

	MakeMoveUnchecked(move);
}

void GameState::MakeMoveUnchecked(const Move& move)
{
	/* Save in history */
	m_history.push_back(m_board);

	/* Make the move: Remove source piece and insert it at target */
	m_board.SetPiece(move.to, m_board.GetPiece(move.from));
	m_board.SetPiece(move.from, Piece::None);

	/* Change the player */
	m_nextPlayer = m_nextPlayer == Color::White ? Color::Black : Color::White;

	/* Basic state calculation */
	CalculateValidity();
}
