// 1DChess.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//

#include <iostream>
#include "Board.h"
#include "Game.h"
#include "EvaluationTree.h"


/* Hack table to convert -2 to unknown eval in case we don't have one somehow */
const char* gEvalTable[] = {
    "?", "-1", "0", "1"
};

const char* evalGameState(const GameState& state, EvaluationTree& eval)
{
    /* Use cache if non-terminal game state.
       Otherwise do a quick eval */
    int value;
    if (state.IsGameOver())
    {
		if (state.IsMate())
		{
			value = state.GetWinner() == Color::White ? 1 : -1;
		}
		else if (state.IsDraw())
		{
			value = 0;
		}
      
    }
    else
        value = eval.GetGameStateEvaluation(state);

    /* Use string table with shifted value */
    return gEvalTable[value + 2];
}

int main()
{
    GameState state;
    state.FinalizeGameState();
    char input = 0;

    EvaluationTree eval;
    std::cout << "Evaluating whole game..." << std::endl;
    eval.Evaluate(state);
    std::cout << "Evaluation done!" << std::endl;

    do
    {
        std::cout << "------------------------------------" << std::endl;
        std::cout << state.GetBoard() << std::endl;
        
        std::cout << "Eval: " << evalGameState(state, eval) << std::endl;


        if (state.IsGameOver())
        {
            std::cout << "Game over!" << std::endl;
            if (state.IsMate())
            {
                std::cout << "Mate!" << std::endl;
                /* Give out winner */
                std::cout << (state.GetWinner() == Color::White ? "White" : "Black") << " wins!" << std::endl;
            }
            else if (state.IsDraw())
            {
                std::cout << "Draw!" << std::endl;
            }

            break;
        }

        /* Enumerate moves for user */
        std::vector<Move> moves = state.GetMoves();
        for (int i = 0; i < moves.size(); i++)
        {       
            GameState newGameState = state;
            newGameState.MakeMove(moves[i]);
            newGameState.FinalizeGameState();
            

            std::cout << i + 1 << ": " << moves[i];

            std::cout << " (" << evalGameState(newGameState, eval) << ")";

            std::cout << std::endl;
		}

		/* Ask for user input */
		std::cout << "Please enter the number of the move you want to make: ";
		std::cin >> input;
		
        if (input == 'q')
        {
			break;
		}

        int number = input - '1';
        if (number >= 0 && number < moves.size())
        {
			state.MakeMove(moves[number]);
            state.FinalizeGameState();
		}
        else
        {
			std::cout << "Invalid move!" << std::endl;
		}

	} while (input != 'q');

	return 0;
}

