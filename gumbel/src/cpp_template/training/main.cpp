#include "bandit.cpp"

int main(int argc, char* argv[])
{
    int total_number_of_games;
    if (argc != 2) {
      total_number_of_games = 2400;
    }
    else {
      total_number_of_games = atoi(argv[1]);
    }
    
    // Defining the neural network

    DoubleHeadedNN model = {
// INSERT MODEL WEIGHTS
    };

    node* nodeArray = new node[10000];

    AllInit(nodeArray);


    std::ofstream myFile("training.txt");

    int number_of_games = 0;


    for (number_of_games = 0; number_of_games < total_number_of_games; ++number_of_games) {

      InitNodes(nodeArray);

      int newId = 1;

      S_BOARD newBoard = initialise_board(); 


      int root = 0;
      int i = 0;
      int j = 0;
      int move;
      float policy[OUTPUT_DIM];
      int policy_counts;

      bool history_of_board[MAX_MOVES][INPUT_DIM];

      int turn = 0;


      int history_of_translated_moves[MAX_MOVES];

      bool history_of_color[MAX_MOVES];


      int moveArray[OUTPUT_DIM];

      int moves_size;

      int result_game = checkWin(newBoard);

      int number_of_recorded_turn = 0;

      while (!result_game) {
          InitNodes(nodeArray);

          newId = 1;

          root = 0;

          // slow searches
          save_board(history_of_board, number_of_recorded_turn, newBoard);

          history_of_color[number_of_recorded_turn] = newBoard.thisTurnColor;

          move = choose_with_Gumbel(nodeArray, newId, newBoard, model);

          save_move_history(newBoard.thisTurnColor, move, history_of_translated_moves, number_of_recorded_turn);

          ++number_of_recorded_turn;
          ++turn;

          playAMove(newBoard, move);

          result_game = checkWin(newBoard);
      }

      int result;
      switch (result_game)
      {
        case 1:
            //std::cout << "Player 1 (X) wins!" << std::endl;
            result = 1;
            break;
        case -1:
            //std::cout << "Player 2 (O) wins!" << std::endl;
            result = -1;
            break;
        case 2:
            //std::cout << "It's a draw!" << std::endl;
            result = 0;
            break;
      }


      for (int it = 0; it < number_of_recorded_turn; ++it)  {
        for (int jt = 0; jt < INPUT_DIM; ++jt) {
            myFile << history_of_board[it][jt];
        }
        // int corresponding_result = result * color[history_of_color[it]];
        int corresponding_result = result;
        int target = corresponding_result;
        myFile << target;
        myFile << "\n";

        myFile << history_of_translated_moves[it];
        myFile << ";";
        myFile << "\n";
      }
      myFile << "\n\n";

      std::cout << "\r     \r" << (number_of_games + 1) << std::flush;
    }

  myFile.close();


  return 0;
}