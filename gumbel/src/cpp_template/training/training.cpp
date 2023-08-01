#include "../game.cpp"

/* ----------------------------------------------------------------------------------------
--------------------------------------- TRAINING ------------------------------------------
------------------------------------------------------------------------------------------- */

// KataGo : apply a temperature on the policy prior to avoid sharp distribution
void root_policy_softmax(float policy[], int policy_counts, float temperature) {
  float sum = 0;
  float aux_policy[policy_counts];
  for (int i = 0; i < policy_counts; ++i) {
    aux_policy[i] = powf(policy[i], 1/temperature);
    sum += powf(policy[i], 1/temperature);
  }
  for (int i = 0; i < policy_counts; ++i) {
    policy[i] = aux_policy[i]/sum;
  }
}

// Create the policy defined in AlphaZero based on visit counts
void create_moves_policy(float policy[], int &policy_counts, int root, int nodeArray[], float temperature) {
  int move_counts = 0;
  int child = nodeArray[root + 1];
  float sum = 0;
  sum += powf((float) nodeArray[child + 3], 1/temperature);
  ++move_counts;
  // Iterate over siblings
  while (nodeArray[child]) {
    child = nodeArray[child];
    sum += powf((float) nodeArray[child + 3], 1/temperature);
    ++move_counts;
  }
  policy_counts = move_counts;
  int index = 0;
  child = nodeArray[root + 1];
  float probability = powf((float) nodeArray[child + 3], 1/temperature) / sum;
  policy[index] = probability;
  while (nodeArray[child]) {
    ++index;
    child = nodeArray[child];
    probability = powf((float) nodeArray[child + 3], 1/temperature) / sum;
    policy[index] = probability;
  }
}

// Return a move chosen based on a policy
int choose_move_policy(float policy[], int policy_counts, int nodeArray[], int root) {
  static std::random_device rd2;
  static std::mt19937 e2(rd2());

  static std::uniform_real_distribution<> dist(0, 1);

  float rand = (float) dist(e2);
  int child = nodeArray[root + 1];
  int index = 0;
  float sum_of_prob = policy[index];
  while (sum_of_prob <= rand) {
    ++index;
    sum_of_prob += policy[index];
    child = nodeArray[child];
  }
  return nodeArray[child + 2];
}

// Save policy in format of 6 moves
// Beware : policy contains clipped policy output
void save_policy(bool thisTurnColor, float policy[], int policy_counts, S_BOARD &board, float history[MAX_MOVES][OUTPUT_DIM], int turn) {
  int moveArray[OUTPUT_DIM];
  int move_counts;
  legalMoves(move_counts, moveArray, board);

  for (int j = 0; j < OUTPUT_DIM; ++j) {
    history[turn][j] = 0;
  }

  for (int i = 0; i < move_counts; ++i) {
    int translation = translate_move(moveArray[i], thisTurnColor);
    history[turn][translation] = policy[i];
  }
}

void save_move_history(bool thisTurnColor, int move, int history_of_moves[MAX_MOVES], int turn) {
  int translation = translate_move(move, thisTurnColor);
  history_of_moves[turn] = translation;
}

void save_board(bool history_of_board[MAX_MOVES][INPUT_DIM], int turn, S_BOARD& board) {
  bool NNboard[INPUT_DIM];
  translate_to_NN_inputs(NNboard, board);
  std::memcpy(history_of_board + turn, NNboard, sizeof(bool) * INPUT_DIM);
}


// Simulate a random variable following Bernoulli
int bernoulli(float p) {
  static std::random_device rd3;
  static std::mt19937 e3(rd3());

  static std::uniform_real_distribution<> dist(0, 1);

  float result = (float) dist(e3);

  if (result < p) {
    return 1;
  }
  else {
    return 0;
  }

}

int getRand(const int& A, const int& B) {
    static std::random_device randDev;
    static std::mt19937 twister(randDev());
    static std::uniform_int_distribution<int> dist;

    dist.param(std::uniform_int_distribution<int>::param_type(A, B));
    return dist(twister);
}

void randomPlay(S_BOARD &board, bool NNboard[], int depth) {
    for (int i = 0; i < depth; ++i) {
        if (checkWin(board)) {
            break;
        }
        int moveArray[OUTPUT_DIM];
        int moves_size;
        legalMoves(moves_size, moveArray, board);
        int rand = getRand(0, moves_size - 1);
        int move = moveArray[rand];
        playAMove(board, move);
        NNplayAMove(NNboard, move, board);
    }
}

// End of training ---------------------------------------------------------------------