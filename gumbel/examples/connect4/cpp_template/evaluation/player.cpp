int main(int argc, char* argv[])
{
    // Defining the neural network

    DoubleHeadedNN model = {    
// INSERT MODEL WEIGHTS 
    };

    int* nodeArray = new int[100000000];

    // Player one plays first

    AllInit(nodeArray);

    int moveArray[OUTPUT_DIM];

    int moves_size;

    int result_game;

    int first_to_move;

    int first_game_result = 0;
    int second_game_result = 0;

    std::cout << "First to move, type 1; second to move, type 2 : " << std::endl;
    std::cin >> first_to_move;

    InitNodes(nodeArray);


    int newId = 6;


    S_BOARD newBoard = initialise_board();


    bool Inputs[INPUT_DIM];

    initialise_NNboard(Inputs);



    int root = 0;
    int move;
        
    int i;

    if (first_to_move == 2) {

        for (i = 0; i < AZ_SEARCHES; ++i) {
            treeTraversal(nodeArray, root, newBoard, model, C_PUCT, newId, Inputs);
        }

        // if (ANALYSIS) PrintResults(nodeArray, root, 1);

        root = chooseNode(nodeArray, root);
        move = nodeArray[root + 2];


        playAMove(newBoard, move);
        NNplayAMove(Inputs, move, newBoard);


        legalMoves(moves_size, moveArray, newBoard);

        result_game = checkWin(newBoard);


        PrintBoard(newBoard);

        while(!result_game)  {
                    
            int found_move = -1;
            while (found_move == -1) {
                std::cout << "Play a move : ";
                std::cin >> move;
                found_move = findOpponentNode(nodeArray, root, move);
            }

            root = found_move; 

            playAMove(newBoard, nodeArray[root + 2]);
            NNplayAMove(Inputs, nodeArray[root + 2], newBoard);

            result_game = checkWin(newBoard);

            PrintBoard(newBoard);


            if (result_game)  {
                break;
            }


            for (i = 0; i < AZ_SEARCHES; ++i) {
                treeTraversal(nodeArray, root, newBoard, model, C_PUCT, newId, Inputs);
            }

            if (ANALYSIS) PrintResults(nodeArray, root, 1);

            root = chooseNode(nodeArray, root);
            move = nodeArray[root + 2];

            

            playAMove(newBoard, move);
            NNplayAMove(Inputs, move, newBoard);

            
            result_game = checkWin(newBoard);


            PrintBoard(newBoard);

            if (result_game)  {
                break;
            }

        }

        switch (result_game)
        {
            case 1:
                std::cout << "Player 1 (X) wins!" << std::endl;
                first_game_result = 1;
                break;
            case -1:
                std::cout << "Player 2 (O) wins!" << std::endl;
                first_game_result = -1;
                break;
            case 2:
                std::cout << "It's a draw!" << std::endl;
                first_game_result = 0;
                break;
        }
    }

    if (first_to_move == 1) {
        
        // Player two plays first

        PrintBoard(newBoard);

        result_game = checkWin(newBoard);
    

        while(!result_game)  {
            
            treeTraversal(nodeArray, root, newBoard, model, C_PUCT, newId, Inputs);

            int found_move = -1;
            while (found_move == -1) {
                std::cout << "Play a move : ";
                std::cin >> move;
                found_move = findOpponentNode(nodeArray, root, move);
            }

            root = found_move; 

            playAMove(newBoard, nodeArray[root + 2]);
            NNplayAMove(Inputs, nodeArray[root + 2], newBoard);

            result_game = checkWin(newBoard);

            PrintBoard(newBoard);

            if (result_game) {
                break;
            }

            for (i = 0; i < AZ_SEARCHES; ++i) {
                treeTraversal(nodeArray, root, newBoard, model, C_PUCT, newId, Inputs);
            }

            if (ANALYSIS) PrintResults(nodeArray, root, 1);

            root = chooseNode(nodeArray, root);
            move = nodeArray[root + 2];

            playAMove(newBoard, move);
            NNplayAMove(Inputs, move, newBoard);

            PrintBoard(newBoard);
            

            result_game = checkWin(newBoard);


            if (result_game)  {
                break;
            }

        }

        switch (result_game)
        {
            case 1:
                std::cout << "Player 1 (X) wins!" << std::endl;
                second_game_result = 1;
                break;
            case -1:
                std::cout << "Player 2 (O) wins!" << std::endl;
                second_game_result = -1;
                break;
            case 2:
                std::cout << "It's a draw!" << std::endl;
                second_game_result = 0;
                break;
        }
    }

    switch(first_game_result) {
        case 1:
        std::cout << "AlphaZero wins!" << std::endl;
        break;
        case -1:
        std::cout << "You wins!" << std::endl;
        break;
        case 0:
        break;
    }

    switch(second_game_result) {
        case 1:
        std::cout << "You wins!" << std::endl;
        break;
        case -1:
        std::cout << "AlphaZero wins!" << std::endl;
        break;
        case 0:
        break;
    }
    
  return 0;
}