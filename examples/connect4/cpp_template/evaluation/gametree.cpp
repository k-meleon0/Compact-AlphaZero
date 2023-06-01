/* ----------------------------------------------------------------------------------------
-------------------------------------- GAME TREE ------------------------------------------
------------------------------------------------------------------------------------------- */



/* Every node in the nodeArray is represented by 6 int :
next_sibling id
first_child id
move
number of traversals
Q-value
policy
*/
// Store a float in nodeArray[index]

void store_float(int nodeArray[], int index, float value_to_store) {
    std::memcpy(nodeArray + index, &value_to_store, sizeof(float));
}

// Get the float in nodeArray[index]
float decode_float(int nodeArray[], int index) {
    float to_return;
    std::memcpy(&to_return, nodeArray + index, sizeof(float));
    return to_return;
}

// Given a position (coded by board), create the children nodes
// newId is a global variable that stores an id not used currently by nodeArray
void createChildren(int nodeArray[], int parentId, S_BOARD &board, int &newId, DoubleHeadedNN &model, int moveList[], int moveList_size, float policy[])  {

    int i;
    int j;
    //first_child
    nodeArray[parentId + 1] = newId;

    //siblings
    for (j = 0; j < moveList_size; ++j)  {
        //next_sibling
        nodeArray[newId] = newId + 6;
        //first_child
        nodeArray[newId + 1] = 0;
        //move
        nodeArray[newId + 2] = moveList[j];
        //number of traversals
        nodeArray[newId + 3] = 0;
        //Q-value
        // This value is initialized at zero in the AlphaZero algorithm; for more information, see "first play urgency" in Leela Chess Zero
        nodeArray[newId + 4] = 0;
        //policy
        store_float(nodeArray, newId + 5, policy[j]);

        newId += 6;
    }
    nodeArray[newId - 6] = 0;
}

void createChildren_mcts(int nodeArray[], int parentId, S_BOARD &board, int &newId, int moveList[], int moveList_size)  {

  int i;
  int j;
  //first_child
  nodeArray[parentId + 1] = newId;

    for (j = 0; j < moveList_size; ++j)  {
        //next_sibling
        nodeArray[newId] = newId + 6;
        //first_child
        nodeArray[newId + 1] = 0;
        //parent
        nodeArray[newId + 2] = parentId;
        //move
        nodeArray[newId + 3] = moveList[j];
        //number of traversals
        nodeArray[newId + 4] = 0;
        //score
        nodeArray[newId + 5] = 0;

        newId += 6;
    }
    nodeArray[newId - 6] = 0;
}

// End of game tree -----------------------------------------------------------------------