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

    int* nodeArray = new int[10000];

    AllInit(nodeArray);


    std::ofstream myFile("training.txt");

    int number_of_games = 0;


    for (number_of_games = 0; number_of_games < total_number_of_games; ++number_of_games) {

      InitNodes(nodeArray);

      int newId = 6;

      S_BOARD newBoard = initialise_board();

      bool Inputs[INPUT_DIM];

      initialise_NNboard(Inputs);


      int root = 0;
      int i = 0;
      int j = 0;
      int move;
      float policy[OUTPUT_DIM];
      int policy_counts;
      float temperature = 0.8;

      bool history_of_board[MAX_MOVES][INPUT_DIM];

      int turn = 0;

      // KataGo Playout cap randomization
      int number_of_recorded_turn = 0;
      float probability_of_record = 0.25;

      float history_of_policy[MAX_MOVES][OUTPUT_DIM];

      float history_of_Q_value[MAX_MOVES];

      bool history_of_color[MAX_MOVES];


      int moveArray[OUTPUT_DIM];

      int moves_size;

      int result_game = checkWin(newBoard);


      while(!result_game)  { 
        
        temperature = 1 - (0.3/180) * turn; 
        

        InitNodes(nodeArray);

        newId = 6;

        root = 0;

        if (bernoulli(probability_of_record)) {
    
          // slow searches
          save_board(history_of_board, number_of_recorded_turn, Inputs);

          history_of_color[number_of_recorded_turn] = newBoard.thisTurnColor;

          treeTraversal_with_Dirichlet(nodeArray, root, newBoard, model, C_PUCT, newId, Inputs);

          for (i = 0; i < 600; ++i) {
            treeTraversal(nodeArray, root, newBoard, model, C_PUCT, newId, Inputs);
          }


          save_Q_value(nodeArray, root, history_of_Q_value, number_of_recorded_turn);

          create_moves_policy(policy, policy_counts, root, nodeArray, 1);
          save_policy(newBoard.thisTurnColor, policy, policy_counts, newBoard, history_of_policy, number_of_recorded_turn);
          create_moves_policy(policy, policy_counts, root, nodeArray, temperature);
          move = choose_move_policy(policy, policy_counts, nodeArray, root);

          
          
          ++number_of_recorded_turn;
          ++turn;
        }
        else {
          // fast searches, with no randomization and noise
          for (i = 0; i < 100; ++i) {
            treeTraversal(nodeArray, root, newBoard, model, C_PUCT, newId, Inputs);
          } 
          move = nodeArray[chooseNode(nodeArray, root) + 2];
          ++turn;
        }
 

        playAMove(newBoard, move);
        NNplayAMove(Inputs, move, newBoard);

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


      int it;
      std::vector <int> line;
      int jt;
      int kt;
      float target;
      float confidence_factor;
      for (it = 0; it < number_of_recorded_turn; ++it)  {
        for (jt = 0; jt < INPUT_DIM; ++jt)  {
          myFile << history_of_board[it][jt];
        }
        //myFile << history_of_Q_value[it];
        confidence_factor = 1; // Beware of type casting !
        //int corresponding_result = result * color[history_of_color[it]];
        int corresponding_result = result;
        target = (confidence_factor * (float) corresponding_result) + (1 - confidence_factor) * history_of_Q_value[it];
        myFile << target;
        myFile << "\n";

        for (kt = 0; kt < OUTPUT_DIM; ++kt) {
          myFile << history_of_policy[it][kt];
          myFile << ";";
        }
        myFile << "\n";
      }
      myFile << "\n\n";

      std::cout << "\r     \r" << (number_of_games + 1) << std::flush;
    }

  myFile.close();


  return 0;
}