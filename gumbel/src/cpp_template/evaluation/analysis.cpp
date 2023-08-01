/* ----------------------------------------------------------------------------------------
--------------------------------------- ANALYSIS ------------------------------------------
------------------------------------------------------------------------------------------- */

//first player is O, second is X, none is _
void PrintBoard(S_BOARD &board) {
  // TODO
}

//print the evaluation of a move given by the ratio of its score over the number of visits
//positive for WHITE, negative for BLACK
void PrintResults(int nodeArray[], int root, bool color) {

  std::cout << "This is an estimation of the moves' relative strength : \n" << std::endl;

  int node = nodeArray[root + 1];
  int move;
  float score;
  float policy;

  move = nodeArray[node + 2];

  float f_value;

  // Get the Q-value of the node
  f_value = decode_float(nodeArray, node + 4);
  score = f_value;
 
  // Get the initial policy of the node
  f_value = decode_float(nodeArray, node + 5);
  policy = f_value;

  std::cout << move << " : " << score << "  " << nodeArray[node + 3] << "  " << policy << std::endl;

  while (nodeArray[node]) {
    node = nodeArray[node];
    move = nodeArray[node + 2];
    f_value = decode_float(nodeArray, node + 4); 
    score = f_value;

    f_value = decode_float(nodeArray, node + 5);
    policy = f_value;

    std::cout << move << " : " << score << "  " << nodeArray[node + 3] << "  " << policy << std::endl;
  }

  int bestNode;
  
  if (!color)  {
    bestNode = chooseNode(nodeArray, root);
    }
  else  {
    bestNode = chooseNode(nodeArray, root);
  }

  
  std::cout << "This is an estimation of the opponent's moves' relative strength : \n" << std::endl;

  node = nodeArray[bestNode + 1];

  move = nodeArray[node + 2];
  f_value = decode_float(nodeArray, node + 4);
  score = f_value;
  f_value = decode_float(nodeArray, node + 5);
  policy = f_value;
  std::cout << move << " : " << score << "  " << nodeArray[node + 3] << "  " << policy << std::endl;

  while (nodeArray[node]) {
    node = nodeArray[node];
    move = nodeArray[node + 2];
    f_value = decode_float(nodeArray, node + 4);
    score = f_value;
    f_value = decode_float(nodeArray, node + 5);
    policy = f_value;
    std::cout << move << " : " << score << "  " << nodeArray[node + 3] << "  " << policy << std::endl;
  }

}

// End of analysis ------------------------------------------------------------------------