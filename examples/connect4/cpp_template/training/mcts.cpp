/* ----------------------------------------------------------------------------------------
------------------------------------------- MCTS ------------------------------------------
------------------------------------------------------------------------------------------- */


// Defined later.
void root_policy_softmax(float policy[], int policy_counts, float temperature);


// BackPropagate the evaluation of a leaf node
// path_indexes stores the indexes of the ancestors of the leaf node up to pathLength (and including the leaf node)
void backPropagation(int nodeArray[], int path_indexes[], int pathLength, float scoreUpdate) {
    int i = 0;
    for (i = 0; i < pathLength; ++i) {
        // Point to the index
        int current_index = path_indexes[i];
        // The number of visits increased
        ++nodeArray[current_index + 3];
        // Get the number of visits in float
        float visit_counts = (float)nodeArray[current_index + 3];
        float Q_value = decode_float(nodeArray, current_index + 4);
        // Updating the Q_value
        Q_value += (scoreUpdate - Q_value) / (visit_counts);
        store_float(nodeArray, current_index + 4, Q_value);
    }
}

// Do one iteration in the MCTS algorithm
void treeTraversal(int nodeArray[], int root, S_BOARD board, DoubleHeadedNN &model, float c_puct, int &newId, bool Inputs[]) {
    // The depth of current node
    int loopNumber = 0;

    int maxChild;
    float maxPUCT = INFINITY;

    int currentChild;
    float currentPUCT;

    float Q_value;
    float policy;

    int parent_visits;
    int child_visits;

    int path_indexes[MAX_MOVES];

    path_indexes[loopNumber] = root;

    // Copy Inputs so that this function doesn't modify the board
    bool NNBoard[INPUT_DIM];
    std::memcpy(NNBoard, Inputs, INPUT_DIM*sizeof(bool));


    //while root has a child
    while (nodeArray[root + 1]) {
        parent_visits = nodeArray[root + 3];
        ++loopNumber;

        //select child
        currentChild = nodeArray[root + 1];
        maxChild = nodeArray[root + 1];
        Q_value = decode_float(nodeArray, maxChild + 4);
        policy = decode_float(nodeArray, maxChild + 5);
        child_visits = nodeArray[maxChild + 3];
        maxPUCT = color[board.thisTurnColor] * Q_value + c_puct * policy * sqrtf((float)parent_visits)/(1 + child_visits);


        //iterate through siblings
        while (nodeArray[currentChild]) {
            currentChild = nodeArray[currentChild];
            Q_value = decode_float(nodeArray, currentChild + 4);
            policy = decode_float(nodeArray, currentChild + 5);
            child_visits = nodeArray[currentChild + 3];
            currentPUCT = color[board.thisTurnColor] * Q_value + c_puct * policy * sqrtf((float)parent_visits)/(1 + child_visits);
            if (currentPUCT > maxPUCT) {
                maxChild = currentChild;
                maxPUCT = currentPUCT;
            }
        }
        root = maxChild;
        // Add chosen node to the path
        path_indexes[loopNumber] = root;
        playAMove(board, nodeArray[root + 2]);
        NNplayAMove(NNBoard, nodeArray[root + 2], board);
    }
    //expansion if game is not finished
    int moveArray[OUTPUT_DIM];
    int moves_size;
    legalMoves(moves_size, moveArray, board);

    // check if the game has ended
    int result = checkWin(board);

    if (!result) {
        first_two_layers(model, NNBoard);
        value_output(model);
        float evaluation = model.valueOutput;
        
        clipped_policy_output(model, moveArray, moves_size, board.thisTurnColor);

        createChildren(nodeArray, root, board, newId, model, moveArray, moves_size, model.policyOutput);

        backPropagation(nodeArray, path_indexes, loopNumber + 1, evaluation);
        return;
    }
    else {
        float evaluation;
        switch (result) {
          case 1:
            evaluation = 1;
            break;
          case -1:
            evaluation = -1;
            break;
          case 2:
            evaluation = 0;
            break;
        }

        backPropagation(nodeArray, path_indexes, loopNumber + 1, evaluation);
        return;
    }
}

// Do one iteration in the MCTS algorithm, but with Dirichlet noise at the last visited node
// Only used at the root in AlphaZero, during training
void treeTraversal_with_Dirichlet(int nodeArray[], int root, S_BOARD board, DoubleHeadedNN &model, float c_puct, int &newId, bool Inputs[]) {
    // The depth of current node
    int loopNumber = 0;

    int maxChild;
    float maxPUCT = INFINITY;

    int currentChild;
    float currentPUCT;

    float Q_value;
    float policy;

    int parent_visits;
    int child_visits;

    int path_indexes[MAX_MOVES];

    path_indexes[loopNumber] = root;

    // Copy Inputs so that this function doesn't modify the board
    bool NNBoard[INPUT_DIM];
    std::memcpy(NNBoard, Inputs, INPUT_DIM*sizeof(bool));


    //while root has a child
    while (nodeArray[root + 1]) {
        parent_visits = nodeArray[root + 3];
        ++loopNumber;

        //select child
        currentChild = nodeArray[root + 1];
        maxChild = nodeArray[root + 1];
        Q_value = decode_float(nodeArray, maxChild + 4);
        policy = decode_float(nodeArray, maxChild + 5);
        child_visits = nodeArray[maxChild + 3];
        maxPUCT = color[board.thisTurnColor] * Q_value + c_puct * policy * sqrtf((float)parent_visits)/(1 + child_visits);


        //iterate through siblings
        while (nodeArray[currentChild]) {
            currentChild = nodeArray[currentChild];
            Q_value = decode_float(nodeArray, currentChild + 4);
            policy = decode_float(nodeArray, currentChild + 5);
            child_visits = nodeArray[currentChild + 3];
            currentPUCT = color[board.thisTurnColor] * Q_value + c_puct * policy * sqrtf((float)parent_visits)/(1 + child_visits);
            if (currentPUCT > maxPUCT) {
                maxChild = currentChild;
                maxPUCT = currentPUCT;
            }
        }
        root = maxChild;
        // Add chosen node to the path
        path_indexes[loopNumber] = root;
        playAMove(board, nodeArray[root + 2]);
        NNplayAMove(NNBoard, nodeArray[root + 2], board);
    }
    //expansion if game is not finished
    int moveArray[OUTPUT_DIM];
    int moves_size;
    legalMoves(moves_size, moveArray, board);

    // check if the game ended
    int result = checkWin(board); 
    if (!result) {
        first_two_layers(model, NNBoard);
        value_output(model);
        float evaluation = model.valueOutput;
        
        clipped_policy_output(model, moveArray, moves_size, board.thisTurnColor);

        root_policy_softmax(model.policyOutput, moves_size, ROOT_TEMPERATURE);

        add_dirichlet_noise(model.policyOutput, moves_size, EPSILON, ALPHA);

        
        createChildren(nodeArray, root, board, newId, model, moveArray, moves_size, model.policyOutput);

        backPropagation(nodeArray, path_indexes, loopNumber + 1, evaluation);
        return;
    }
    else {
        float evaluation;
        switch (result) {
          case 1:
            evaluation = 1;
            break;
          case -1:
            evaluation = -1;
            break;
          case 2:
            evaluation = 0;
            break;
        }

        backPropagation(nodeArray, path_indexes, loopNumber + 1, evaluation);
        return;
    }
}


float score(int nodeArray[], int node)  {
  float f_value;
  std::memcpy(&f_value, &nodeArray[node + 4], sizeof(float));
  return f_value;
}

// In AlphaZero, the most visited child is chosen
int number_of_visits(int nodeArray[], int node) {
  return nodeArray[node + 3];
}


//return the node id with max visits
int chooseNode(int nodeArray[], int root)  {
  int maxNode;
  int maxEval;

  maxNode = nodeArray[root + 1];
  maxEval = number_of_visits(nodeArray, maxNode);
  root = maxNode;

  while (nodeArray[root]) {
    root = nodeArray[root];
    if (maxEval < number_of_visits(nodeArray, root))  {
      maxEval = number_of_visits(nodeArray, root);
      maxNode = root;
    }
  }
  return maxNode;
}


//returns the node id of opponent's move
int findOpponentNode(int nodeArray[], int root, int move)  {
  int node = nodeArray[root + 1];
  if (nodeArray[node + 2] == move)  {
    return node;
  }
  while (nodeArray[node]) {
    node = nodeArray[node];
    if (nodeArray[node + 2] == move)  {
      return node;
    }
  }
  std::cout << "The move is invalid." << std::endl;
  return -1;
}

// End of MCTS ----------------------------------------------------------------------