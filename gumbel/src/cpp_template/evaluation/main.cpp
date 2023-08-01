int main(int argc, char* argv[])
{
    // Defining the neural network

    DoubleHeadedNN model = {    
// INSERT MODEL WEIGHTS 
    };

    int* nodeArray = new int[100000000];

    int* nodeArray2 = new int[100000000];


    // Player one plays first
    AllInit(nodeArray);

    int AZ_wins_white = 0;
    int AZ_wins_black = 0;
    int AZ_draws_white = 0;
    int AZ_draws_black = 0;
    int MCTS_wins_white = 0;
    int MCTS_wins_black = 0;

    int moveArray[OUTPUT_DIM];

    int moves_size;

    int result_game;

    for (int times = 0; times < NUMBER_OF_GAMES; ++times) {

        int i;

        InitNodes(nodeArray);

        InitNodes(nodeArray2);

        int newId = 6;

        int newId2 = 6;

        S_BOARD newBoard = initialise_board();

        S_BOARD newBoard2 = initialise_board();

        bool Inputs[INPUT_DIM];

        initialise_NNboard(Inputs);



        int root = 0;
        int root2 = 0;
        int move;

        for (i = 0; i < AZ_SEARCHES; ++i) {
            treeTraversal(nodeArray, root, newBoard, model, C_PUCT, newId, Inputs);
        }

        if (ANALYSIS) PrintResults(nodeArray, root, 1);

        root = chooseNode(nodeArray, root);
        move = nodeArray[root + 2];


        playAMove(newBoard, move);
        NNplayAMove(Inputs, move, newBoard);


        legalMoves(moves_size, moveArray, newBoard);

        result_game = checkWin(newBoard);


        if (ANALYSIS) PrintBoard(newBoard);

        while(!result_game)  {
        
            treeTraversal_mcts(nodeArray2, root2, newBoard2, newId2);

            root2 = findOpponentNode_mcts(nodeArray2, root2, move);

            playAMove(newBoard2, nodeArray2[root2 + 3]);

            for (i = 0; i < MCTS_SEARCHES; ++i) {
                treeTraversal_mcts(nodeArray2, root2, newBoard2, newId2);
            }

            root2 = chooseNode_mcts(nodeArray2, root2);
            move = nodeArray2[root2 + 3];

            playAMove(newBoard2, move);


            
            root = findOpponentNode(nodeArray, root, move);

            

            playAMove(newBoard, nodeArray[root + 2]);
            NNplayAMove(Inputs, nodeArray[root + 2], newBoard);

            result_game = checkWin(newBoard);

            if (ANALYSIS) PrintBoard(newBoard2);


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


            if (ANALYSIS) PrintBoard(newBoard);

            if (result_game)  {
                break;
            }

        }
    
    int first_game_result;
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



    // Player two plays first

    InitNodes(nodeArray);
    InitNodes(nodeArray2);

    root = 0;

    root2 = 0;

    newId = 6;

    newId2 = 6;

    newBoard = initialise_board();

    newBoard2 = initialise_board();

    initialise_NNboard(Inputs);


    for (i = 0; i < MCTS_SEARCHES; ++i) {
        treeTraversal_mcts(nodeArray2, root2, newBoard2, newId2);
    }


    root2 = chooseNode_mcts(nodeArray2, root2);
    move = nodeArray2[root2 + 3];


    

    playAMove(newBoard2, move);

    result_game = checkWin(newBoard2);


    if (ANALYSIS) PrintBoard(newBoard2);

    

    while(!result_game)  {
        
        treeTraversal(nodeArray, root, newBoard, model, C_PUCT, newId, Inputs);

        root = findOpponentNode(nodeArray, root, move);

        playAMove(newBoard, nodeArray[root + 2]);
        NNplayAMove(Inputs, nodeArray[root + 2], newBoard);

        for (i = 0; i < AZ_SEARCHES; ++i) {
            treeTraversal(nodeArray, root, newBoard, model, C_PUCT, newId, Inputs);
        }

        if (ANALYSIS) PrintResults(nodeArray, root, 1);

        root = chooseNode(nodeArray, root);
        move = nodeArray[root + 2];

        playAMove(newBoard, move);
        NNplayAMove(Inputs, move, newBoard);

        if (ANALYSIS) PrintBoard(newBoard);
        
        root2 = findOpponentNode_mcts(nodeArray2, root2, move);

        

        playAMove(newBoard2, nodeArray2[root2 + 3]);

        result_game = checkWin(newBoard2);


        if (checkWin(newBoard2))  {
            break;
        }


        for (i = 0; i < MCTS_SEARCHES; ++i) {
            treeTraversal_mcts(nodeArray2, root2, newBoard2, newId2);
        }


        root2 = chooseNode_mcts(nodeArray2, root2);
        move = nodeArray2[root2 + 3];

        

        playAMove(newBoard2, move);

        result_game = checkWin(newBoard2);


        if (ANALYSIS) PrintBoard(newBoard2);

        if (checkWin(newBoard2))  {
            break;
        }

        
    }

    int second_game_result;
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

    switch(first_game_result) {
        case 1:
        std::cout << "AlphaZero wins game 1" << std::endl;
        ++AZ_wins_white;
        break;
        case -1:
        std::cout << "Pure MCTS wins game 1" << std::endl;
        ++MCTS_wins_black;
        break;
        case 0:
        std::cout << "Game 1 ends in a draw" << std::endl;
        ++AZ_draws_white;
        break;
    }

    switch(second_game_result) {
        case 1:
        std::cout << "Pure MCTS wins game 2" << std::endl;
        ++MCTS_wins_white;
        break;
        case -1:
        std::cout << "AlphaZero wins game 2" << std::endl;
        ++AZ_wins_black;
        break;
        case 0:
        std::cout << "Game 2 ends in a draw" << std::endl;
        ++AZ_draws_black;
        break;
    }
    }
    std::cout << "AZ wins as white : " << AZ_wins_white << std::endl;
    std::cout << "AZ losses as white : " << MCTS_wins_black << std::endl;
    std::cout << "AZ draws as white : " << AZ_draws_white << std::endl;
    std::cout << "AZ wins as black : " << AZ_wins_black << std::endl;
    std::cout << "AZ losses as black : " << MCTS_wins_white << std::endl;
    std::cout << "AZ draws as black : " << AZ_draws_black << std::endl;


  return 0;
}