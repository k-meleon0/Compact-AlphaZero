/* Template C++ code for AlphaZero */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <random>
#include <chrono>
#include <fstream>
#include <cstring>
#include <math.h>

/* Check the training.txt generated for nan values */
/* ----------------------------------------------------------------------------------------
------------------------------------------ MACRO ------------------------------------------
------------------------------------------------------------------------------------------- */

// MACRO relative to the game
#define MAX_MOVES 99

// MACRO relative to training
#define NUMBER_OF_SIMULATIONS 200
#define NUMBER_OF_TOP_MOVES 16

#define C_VISITS 50
#define C_SCALE 1

// MACRO relative to precomputation
#define POP(b) PopBit(b)
#define CNT(b) CountBits(b)
#define CNT2(b) CountBits2(b)
#define CLRBIT(bb,sq) ((bb) &= ClearMask[(sq)])
#define SETBIT(bb,sq) ((bb) |= SetMask[(sq)])
#define SETBIT2(bb,sq)  ((bb) |= SetMask2[(sq)])
#define U64 std::uint64_t
#define U32 std::uint32_t

// MACRO relative to neural network
#define INPUT_DIM 98
#define OUTPUT_DIM 14

// Sign relative to color
int color[2] = {1, -1};

// End of macro ---------------------------------------------------------------------------

/* ----------------------------------------------------------------------------------------
----------------------------------------- GUMBEL ------------------------------------------
------------------------------------------------------------------------------------------- */

// Sample k variables from the Gumbel distribution
void sample_gumbel(float gumbel_samples[], int number_of_actions) {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    static std::extreme_value_distribution<> Gumbel_0 {0, 1};

    for (int i = 0; i < number_of_actions; ++i) {
        gumbel_samples[i] = Gumbel_0(gen);
    }
}
/* ----------------------------------------------------------------------------------------
--------------------------------- NEURAL NETWORK ------------------------------------------
------------------------------------------------------------------------------------------- */

/* INPUT SPACE OF THE NEURAL NETWORK

Result : float32

*/

struct DoubleHeadedNN_s
{
    // Weights of the neural network
    float weights0[INPUT_DIM][256];
    float weights1[256][64];
    float policyWeights0[64][32];
    float policyWeights1[32][OUTPUT_DIM];
    float valueWeights0[64][32];
    float valueWeights1[32][1];

    // Bias of the neural network
    float bias0[256];
    float bias1[64];
    float policyBias0[32];
    float policyBias1[OUTPUT_DIM];
    float valueBias0[32];
    float valueBias1[1];

    // Utility vectors
    float vector0[256];
    float vector1[64];
    float vector2[32];
    float vector3[1];

    float vector5[64];
    float policyOutput[OUTPUT_DIM];
    float log_policyOutput[OUTPUT_DIM];
    int policy_size;
    float valueOutput;
};

typedef struct DoubleHeadedNN_s DoubleHeadedNN;

/*
    ACTIVATION FUNCTIONS
*/

void ReLU(float Outputs[], int input_size) {
    int i;
    for (i = 0; i < input_size; ++i) {
        if (Outputs[i] < 0) {
            Outputs[i] = 0;
        }
    }
}


// Adapted from Stack Overflow
void softmax(float input[], int input_len) {

    // Find the max value to ensure numerical stability
    float m = -INFINITY;
    for (size_t i = 0; i < input_len; i++) {
        if (input[i] > m) {
        m = input[i];
        }
    }

    float sum = 0.0;
    for (size_t i = 0; i < input_len; i++) {
        sum += expf(input[i] - m);
    }

    float offset = m + logf(sum);
    for (size_t i = 0; i < input_len; i++) {
        input[i] = expf(input[i] - offset);
    }
}

// Translate the move from board representation to policy space [0; OUTPUT_DIM]
int translate_move(int move, bool thisTurnColor);

// In AlphaZero, the illegal moves are not taken into account in the calculation of softmax
// Therefore this function modifies input and input_len, so that they contain the probabilities and the size of moveArray in order at the same indexes
void clipping(float input[], int &input_len, int moveArray[], int moves_size, bool thisTurnColor) {

  // A pointer to the last unseen move
  int ptr_move = 0;

  float copy_of_input[INPUT_DIM];

  // Modify input to retain only unclipped moves, in order
  for (ptr_move = 0; ptr_move < moves_size; ++ptr_move) {
    int translation = translate_move(moveArray[ptr_move], thisTurnColor);
    copy_of_input[ptr_move] = input[translation];
  }

  std::memcpy(input, copy_of_input, sizeof(float) * moves_size);

  input_len = moves_size;
}

// In AlphaZero, the illegal moves are not taken into account in the calculation of softmax
// Therefore this function modifies input and input_len, so that they contain the probabilities and the size of moveArray in order at the same indexes
void clippedSoftmax(float input[], int &input_len, int moveArray[], int moves_size, bool thisTurnColor) {

  // A pointer to the last unseen move
  int ptr_move = 0;

  float copy_of_input[INPUT_DIM];

  // Modify input to retain only unclipped moves, in order
  for (ptr_move = 0; ptr_move < moves_size; ++ptr_move) {
    int translation = translate_move(moveArray[ptr_move], thisTurnColor);
    copy_of_input[ptr_move] = input[translation];
  }

  std::memcpy(input, copy_of_input, sizeof(float) * moves_size);

  input_len = moves_size;

  softmax(input, input_len);
}

template <typename TwoD>
void multiply(TwoD& Weights, bool Inputs[], float Outputs[], int input_size, int output_size) {
    
    int i;
    int j;
    int index_array[input_size];
    int size = 0;
    for (i = 0; i < input_size; ++i) {
      if (Inputs[i]) {
        index_array[size] = i;
        ++size;
      }
    }
    for (j = 0; j < output_size; ++j) {
        for (i = 0; i < size; ++i) {
          Outputs[j] += Weights[index_array[i]][j];
        }
    }
}

template <typename TwoD>
void multiply2(TwoD& Weights, float Inputs[], float Outputs[], int input_size, int output_size) {

    int i;
    int j;
    for (j = 0; j < output_size; ++j) {
        for (i = 0; i < input_size; ++i) {
            Outputs[j] += Weights[i][j] * Inputs[i];
        }
    }
}

void add(float Outputs[], float Vector2[], int vector_size) {
    std::memcpy(Outputs, Vector2, vector_size*sizeof(float));
}

void evaluate(float Outputs[], float (*function)(float), int vector_size)    {

    int i;
    for (i = 0; i < vector_size; ++i)   {
        Outputs[i] = function(Outputs[i]);
    }

}

void first_two_layers(DoubleHeadedNN &model, bool Inputs[]) {
    add(model.vector0, model.bias0, 256);
    multiply(model.weights0, Inputs, model.vector0, INPUT_DIM, 256);
    ReLU(model.vector0, 256);

    add(model.vector1, model.bias1, 64);
    multiply2(model.weights1, model.vector0, model.vector1, 256, 64);
    ReLU(model.vector1, 64);   
}

// Execute after first_two_layers
void value_output(DoubleHeadedNN &model) {
    add(model.vector2, model.valueBias0, 32);
    multiply2(model.valueWeights0, model.vector1, model.vector2, 64, 32);
    ReLU(model.vector2, 32);

    add(model.vector3, model.valueBias1, 1);
    multiply2(model.valueWeights1, model.vector2, model.vector3, 32, 1);

    model.valueOutput = tanhf(model.vector3[0]);
}

void clipped_log_policy_output(DoubleHeadedNN &model, int moveArray[], int moves_size, bool thisTurnColor) {
    add(model.vector5, model.policyBias0, 32);
    multiply2(model.policyWeights0, model.vector1, model.vector5, 64, 32);
    ReLU(model.vector5, 32);

    add(model.log_policyOutput, model.policyBias1, OUTPUT_DIM);
    multiply2(model.policyWeights1, model.vector5, model.log_policyOutput, 32, OUTPUT_DIM);
    model.policy_size = moves_size;

    clipping(model.log_policyOutput, model.policy_size, moveArray, moves_size, thisTurnColor);
}

void policy_output(DoubleHeadedNN &model) {
    add(model.vector5, model.policyBias0, 32);
    multiply2(model.policyWeights0, model.vector1, model.vector5, 64, 32);
    ReLU(model.vector5, 32);

    add(model.policyOutput, model.policyBias1, OUTPUT_DIM);
    multiply2(model.policyWeights1, model.vector5, model.policyOutput, 32, OUTPUT_DIM);

    softmax(model.policyOutput, OUTPUT_DIM);
}

void clipped_policy_output(DoubleHeadedNN &model, int moveArray[], int moves_size, bool thisTurnColor) {
    add(model.vector5, model.policyBias0, 32);
    multiply2(model.policyWeights0, model.vector1, model.vector5, 64, 32);
    ReLU(model.vector5, 32);

    add(model.policyOutput, model.policyBias1, OUTPUT_DIM);
    multiply2(model.policyWeights1, model.vector5, model.policyOutput, 32, OUTPUT_DIM);
    model.policy_size = moves_size;

    clippedSoftmax(model.policyOutput, model.policy_size, moveArray, moves_size, thisTurnColor);
}


/* End of neural network section ------------------------------------------------ */


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

struct node {
    int next_sibling_id;
    int first_child_id;
    int move;
    int number_of_traversals;
    float Q_value;
    float log_policy;
    float gumbel_plus_log;
    float current_score;
};


// Create children with moveList as legal moves
// newId is a global variable that stores an id not used currently by nodeArray
void createChildren(node nodeArray[], int parentId, int &newId, DoubleHeadedNN &model, int moveList[], int moveList_size, float log_policy[])  {

    int i;
    int j;
    //first_child
    nodeArray[parentId].first_child_id = newId;

    //siblings
    for (j = 0; j < moveList_size; ++j)  {
        //next_sibling
        nodeArray[newId].next_sibling_id = newId + 1;
        //first_child
        nodeArray[newId].first_child_id = 0;
        //move
        nodeArray[newId].move = moveList[j];
        //number of traversals
        nodeArray[newId].number_of_traversals = 0;
        //Q-value
        // This value is initialized at zero in the AlphaZero algorithm; for more information, see "first play urgency" in Leela Chess Zero
        nodeArray[newId].Q_value = 0; 
        //log_policy
        nodeArray[newId].log_policy = log_policy[j];
        //gumbel plus log
        nodeArray[newId].gumbel_plus_log = 0;
        //current score
        nodeArray[newId].current_score = 0;

        newId += 1;
    }
    nodeArray[newId - 1].next_sibling_id = 0;
}

// End of game tree -----------------------------------------------------------------------

/* ----------------------------------------------------------------------------------------
--------------------------------------- PRECOMPUTATION ------------------------------------
------------------------------------------------------------------------------------------- */

const char LogTable256[256] =
{
#define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
    -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
    LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
    LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)
};



unsigned int ln2(unsigned int v)  {
  // 32-bit word to find the log of
  unsigned int t, tt; // temporaries

  if ((tt = v >> 16))
  {
    return (t = tt >> 8) ? 24 + LogTable256[t] : 16 + LogTable256[tt];
  }
  else
  {
    return (t = v >> 8) ? 8 + LogTable256[t] : LogTable256[v];
  }
}

const int BitTable[64] = {
  63, 30, 3, 32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34, 61, 29, 2,
  51, 21, 43, 45, 10, 18, 47, 1, 54, 9, 57, 0, 35, 62, 31, 40, 4, 49, 5, 52,
  26, 60, 6, 23, 44, 46, 27, 56, 16, 7, 39, 48, 24, 59, 14, 12, 55, 38, 28,
  58, 20, 37, 17, 36, 8
};

int PopBit(U64 *bb) {
  U64 b = *bb ^ (*bb - 1);
  unsigned int fold = (unsigned) ((b & 0xffffffff) ^ (b >> 32));
  *bb &= (*bb - 1);
  return BitTable[(fold * 0x783a9b23) >> 26];
}

int CountBits(U64 b) {
  int r;
  for(r = 0; b; r++, b &= b - 1);
  return r;
}

int CountBits2(U32 b) {
  int r;
  for(r = 0; b; r++, b &= b - 1);
  return r;
}

U64 SetMask[64];
U32 SetMask2[32];
U64 ClearMask[64];
std::vector <int> moveList[128];



void InitBitMasks() {
  int index = 0;


  for(index = 0; index < 64; ++index) {
    SetMask[index] = 0;
    ClearMask[index] = 0;
  }
  U64 shiftMe = 1;
  for (index = 0; index < 64; ++index) {
    SetMask[index] |= (shiftMe << index);
    ClearMask[index] = ~SetMask[index];
  }
  for(index = 0; index < 32; ++index) {
    SetMask2[index] = 0;
  }
  U32 shiftM = 1;
  for(index = 0; index < 32; ++index) {
    SetMask2[index] |= (shiftM << index);
  }

}

void InitMoveList() {
  int moveCode;
  std::vector<int> moveVector;
  int column;
  for (moveCode = 0; moveCode < 128; ++moveCode)  {
    for (column = 0; column < 7; ++ column) {
      if (!(moveCode >> column & 1))  {
        moveVector.push_back(column);
      }
    }
    moveList[moveCode] = moveVector;
    moveVector.clear();
  }
}

// Initialise the nodeArray
void InitNodes(node nodeArray[]) {
    nodeArray[0].next_sibling_id = 0;
    nodeArray[0].first_child_id = 0;
    nodeArray[0].move = 0;
    nodeArray[0].number_of_traversals = 0;
    nodeArray[0].Q_value = 0;
    nodeArray[0].log_policy = 0;
    nodeArray[0].gumbel_plus_log = 0;
    nodeArray[0].current_score = 0;
}

// Initialise precomputed tables 
void AllInit(node nodeArray[]) {
  InitBitMasks();
  InitMoveList();
  InitNodes(nodeArray);
}

// End of precomputation ------------------------------------------------------------------
/* ----------------------------------------------------------------------------------------
------------------------------------------- GAME ------------------------------------------
------------------------------------------------------------------------------------------- */

struct S_BOARD {

  //board[0] = white board; board[1] = black board; board[2] = both
  //Connect 4 plays on a 6*7 board, where 6 is the width
  /*
  6,13,...
  5,12,...
  4,11,...
  3,10,...
  2,9,...
  1,8,15,...
  0,7,14,...
  */
  //the row with 6 + 7k is left at 0 for useful purposes
  U64 board[3];

  //first 7 bits indicate : 0 if column isn't full, 1 if column is full
  U32 playableColumns;

  //0 for first player, 1 for second
  bool thisTurnColor;

};

// Return the initial position
S_BOARD initialise_board() {
  S_BOARD new_board = {{0, 0, 0}, 0, 0};
  return new_board;
}

// Return the initial position as neural network input
void initialise_NNboard(bool boolean_board[]) {
  for (int i = 0; i < INPUT_DIM; ++i) {
    boolean_board[i] = 0;
  }
  for (int i = 84; i < 84 + 7; ++i) {
    boolean_board[i] = true;
  }
}

// Modify the board
void playAMove(S_BOARD &board, int move) {
  U64 copy = ~((board.board[2] >> move*7) & 0b1111111);
  int height = POP(&copy);

  SETBIT(board.board[board.thisTurnColor], height + 7*move);
  SETBIT(board.board[2], height + 7*move);

  if (height == 5)  {
    SETBIT2(board.playableColumns, move);
  }

  board.thisTurnColor ^= 1;

}

// Store legal moves in moveArray, in order
void legalMoves(int &moves_size, int moveArray[], S_BOARD &board) {
  moves_size = moveList[board.playableColumns].size();
  for (int i = 0; i < moves_size; ++i) {
    moveArray[i] = moveList[board.playableColumns][i];
  }
}

// return 0 if nothing, 1 if WHITE win, -1 if BLACK win, 2 if drawn
int checkWin(S_BOARD &board)  {
 //one player cannot win before it's his turn!
  bool colorChecked = board.thisTurnColor ^ 1;

  //four in a row in the same column
  if (board.board[colorChecked] & (board.board[colorChecked] >> 1) & (board.board[colorChecked] >> 2) & (board.board[colorChecked] >> 3)) {
    return color[colorChecked];
  }
  //four in a row in the same row
  if (board.board[colorChecked] & (board.board[colorChecked] >> 7) & (board.board[colorChecked] >> 14) & (board.board[colorChecked] >> 21)) {
    return color[colorChecked];
  }
  //four in a row in diagonal from bottom left to top right
  if (board.board[colorChecked] & (board.board[colorChecked] >> 8) & (board.board[colorChecked] >> 16) & (board.board[colorChecked] >> 24)) {
    return color[colorChecked];
  }
  //four in a row in diagonal from top left to bottom right
  if (board.board[colorChecked] & (board.board[colorChecked] >> 6) & (board.board[colorChecked] >> 12) & (board.board[colorChecked] >> 18)) {
    return color[colorChecked];
  }

  //no more moves to be played
  if (board.playableColumns == 0b1111111) {
    return 2;
  }

  return 0;
}

//use this function after playAMove
// Play A Move on the boolean array used by the neural network
void NNplayAMove(bool Inputs[], int move, S_BOARD &board) {
  for (int i = 0; i < 7; ++i) {
    for (int j = 0; j < 6; ++j) {
      if (board.board[0] & SetMask[7*i + j]) {
        Inputs[6*i + j] = 1;
      }
      else {
        Inputs[6*i + j] = 0;
      }
      if (board.board[1] & SetMask[7*i + j]) {
        Inputs[6*i + j + 42] = 1;
      }
      else {
        Inputs[6*i + j + 42] = 0;
      }
    }
  }
  U64 mask = 0b1111111;
  U64 moves = (~board.playableColumns) & mask;
  if (!board.thisTurnColor) {
    for (int i = 0; i < 7; ++i) {
      if (moves & SetMask[i]) {
        Inputs[84 + i] = 1;
      }
      else {
        Inputs[84 + i] = 0;
      }
    }
    for (int i = 0; i < 7; ++i) {
      Inputs[84 + 7 + i] = 0;
    }
  }
  else {
    for (int i = 0; i < 7; ++i) {
      if (moves & SetMask[i]) {
        Inputs[84 + 7 + i] = 1;
      }
      else {
        Inputs[84 + 7 + i] = 0;
      }
    }
    for (int i = 0; i < 7; ++i) {
      Inputs[84 + i] = 0;
    }
  }
}

// Translate the move from board representation to policy space [0; OUTPUT_DIM]
int translate_move(int move, bool thisTurnColor) {
    if (!thisTurnColor) {
        return move;
    }
    else {
        return move + 7;
    }
}

void translate_to_NN_inputs(bool Inputs[], S_BOARD& board) {
    for (int i = 0; i < 7; ++i) {
        for (int j = 0; j < 6; ++j) {
      if (board.board[0] & SetMask[7 * i + j]) {
        Inputs[6 * i + j] = 1;
      } else {
        Inputs[6 * i + j] = 0;
      }
      if (board.board[1] & SetMask[7 * i + j]) {
        Inputs[6 * i + j + 42] = 1;
      } else {
        Inputs[6 * i + j + 42] = 0;
      }
        }
    }
    U64 mask = 0b1111111;
    U64 moves = (~board.playableColumns) & mask;
    if (!board.thisTurnColor) {
        for (int i = 0; i < 7; ++i) {
      if (moves & SetMask[i]) {
        Inputs[84 + i] = 1;
      } else {
        Inputs[84 + i] = 0;
      }
        }
        for (int i = 0; i < 7; ++i) {
      Inputs[84 + 7 + i] = 0;
        }
    } else {
        for (int i = 0; i < 7; ++i) {
      if (moves & SetMask[i]) {
        Inputs[84 + 7 + i] = 1;
      } else {
        Inputs[84 + 7 + i] = 0;
      }
        }
        for (int i = 0; i < 7; ++i) {
      Inputs[84 + i] = 0;
        }
    }
}


// End of game -------------------------------------------------------------------------

/* ----------------------------------------------------------------------------------------
----------------------------------------- BANDIT ------------------------------------------
------------------------------------------------------------------------------------------- */

float sigma(float average_q, int max_visits)
{
    return (C_VISITS + max_visits) * C_SCALE * average_q;
}

float sigma_2(float Q_completion) {
    return Q_completion * 2;
}

void completion_Q_values(node nodeArray[], float new_policy[], int chosenNode, bool thisTurnColor)
{
    // new_policy[OUTPUT_DIM]

    // Create a improved policy on the children of the chosen node
    float root_estimation = nodeArray[chosenNode].Q_value;

    int num_moves = 0;

    int child = nodeArray[chosenNode].first_child_id;
    float completed_Q_value;

    // Iterate over children
    while (nodeArray[child].next_sibling_id)
    {
        if (nodeArray[child].number_of_traversals)
        {
            completed_Q_value = nodeArray[child].Q_value;
        }
        else
        {
            completed_Q_value = root_estimation;
        }
        completed_Q_value *= color[thisTurnColor];
        new_policy[num_moves] = nodeArray[child].log_policy;
        new_policy[num_moves] += sigma_2(completed_Q_value);

        child = nodeArray[child].next_sibling_id;
        ++num_moves;
    }
    // Last child
    if (nodeArray[child].number_of_traversals)
    {
        completed_Q_value = nodeArray[child].Q_value;
    }
    else
    {
        completed_Q_value = root_estimation;
    }
    completed_Q_value *= color[thisTurnColor];
    new_policy[num_moves] = nodeArray[child].log_policy;
    new_policy[num_moves] += sigma_2(completed_Q_value);

    ++num_moves;

    // softmax
    softmax(new_policy, num_moves);
}

// BackPropagate the evaluation of a leaf node
// path_indexes stores the indexes of the ancestors of the leaf node up to pathLength (and including the leaf node)
void backPropagation(node nodeArray[], int path_indexes[], int pathLength, float scoreUpdate) {
    int i = 0;
    for (i = 0; i < pathLength; ++i) {
        // Point to the index
        int current_index = path_indexes[i];
        // The number of visits increased
        ++nodeArray[current_index].number_of_traversals;
        // Get the number of visits in float
        float visit_counts = (float)nodeArray[current_index].number_of_traversals;
        float Q_value = nodeArray[current_index].Q_value;
        // Updating the Q_value
        nodeArray[current_index].Q_value += (scoreUpdate - Q_value) / (visit_counts);
    }
}

//first player is O, second is X, none is _
void PrintBoard(S_BOARD &board) {
  int row;
  int column;
  //number that are congruent modulo 7 are in the same row
  for (row = 5; row >= 0; --row)  {
    for (column = 0; column < 7; ++column)  {
      if (board.board[0] & SetMask[row + 7*column]) {
        std::cout << "X";
      }
      else if (board.board[1] & SetMask[row + 7*column])  {
        std::cout << "O";
      }
      else {
        std::cout << "_";
      }
      std::cout << " ";
    }
    std::cout << "\n";
  }
  std::cout << "\n";
}

void one_simulation(node nodeArray[], int chosenNode, S_BOARD board, DoubleHeadedNN &model, int &newId)
{

    int path_indexes[MAX_MOVES];

    path_indexes[0] = chosenNode;
    int loopNumber = 0;

    int current_node = chosenNode;
    playAMove(board, nodeArray[current_node].move);


    // While current_node is not a leaf
    while (nodeArray[current_node].first_child_id) {
    ++loopNumber;
    // calculate the improved policy
    float new_policy[OUTPUT_DIM];
    completion_Q_values(nodeArray, new_policy, current_node, board.thisTurnColor);

    // Find the next node
    int total_visits_at_root = nodeArray[current_node].number_of_traversals;
    int child_node = nodeArray[current_node].first_child_id;
    int argmax_node = child_node;
    float max_value = new_policy[0] - (((float)nodeArray[child_node].number_of_traversals) / (1 + total_visits_at_root));
    int i = 0;
    ++i;

    while (nodeArray[child_node].next_sibling_id) {
      child_node = nodeArray[child_node].next_sibling_id;
      float new_value = new_policy[i] - (((float)nodeArray[child_node].number_of_traversals) / (1 + total_visits_at_root));
      if (max_value < new_value) {
        max_value = new_value;
        argmax_node = child_node;
      }
      ++i;
    }

    // the next node is argmax_node
    current_node = argmax_node;
    path_indexes[loopNumber] = current_node;
    playAMove(board, nodeArray[current_node].move);
    }

    // Current_node is a leaf

    //expansion if game is not finished
    int moveArray[OUTPUT_DIM];
    int moves_size;
    legalMoves(moves_size, moveArray, board);

    // Check if game ended
    int result = checkWin(board);
    float reward;

    if (result) {
        // game ended
        switch (result) {
            case 1:
                reward = 1;
                break;
            case -1:
                reward = -1;
                break;
            case 2:
                reward = 0;
                break;
        }
    } else {
        // Translate to NN inputs
        bool Inputs[INPUT_DIM];
        translate_to_NN_inputs(Inputs, board);

        first_two_layers(model, Inputs);
        // Estimate new Q value
        value_output(model);
        reward = model.valueOutput;

        // Expand
        clipped_log_policy_output(model, moveArray, moves_size, board.thisTurnColor);
        createChildren(nodeArray, current_node, newId, model, moveArray, moves_size, model.log_policyOutput);
    }
    backPropagation(nodeArray, path_indexes, loopNumber + 1, reward);
}

void insertion_sort_reverse(node nodeArray[], int array_of_nodes[], int size_of_array)
{
    // Insertion sort in decreasing order on current score
    for (int k = 1; k < size_of_array; ++k)
    {
        int temp = array_of_nodes[k];
        int j = k - 1;
        while (j >= 0 && nodeArray[temp].current_score >= nodeArray[array_of_nodes[j]].current_score)
        {
            array_of_nodes[j + 1] = array_of_nodes[j];
            j = j - 1;
        }
        array_of_nodes[j + 1] = temp;
    }
}

void naive_top_k_score(node nodeArray[], int array_of_nodes[], int size_of_array, int k)
{
    // Only when k < size_of_array

    // Sort the first k elements using insertion sort
    insertion_sort_reverse(nodeArray, array_of_nodes, k);

    // The minimum of the top k elements
    int current_min = nodeArray[array_of_nodes[k - 1]].current_score;

    // Insert the new value at the correct position
    for (int i = k; i < size_of_array; ++i)
    {
        int temp = array_of_nodes[i];
        // Only difference with insertion sort : the insertion start at k instead of i
        int j = k - 1;
        while (j >= 0 && nodeArray[temp].current_score > nodeArray[array_of_nodes[j]].current_score)
        {
            array_of_nodes[j + 1] = array_of_nodes[j];
            j = j - 1;
        }
        array_of_nodes[j + 1] = temp;
    }
}

void insertion_sort_reverse_gumbel_log(node nodeArray[], int array_of_nodes[], int size_of_array)
{
    // Insertion sort in decreasing order on gumbel plus log
    for (int k = 1; k < size_of_array; ++k)
    {
        int temp = array_of_nodes[k];
        int j = k - 1;
        while (j >= 0 && nodeArray[temp].gumbel_plus_log >= nodeArray[array_of_nodes[j]].gumbel_plus_log)
        {
            array_of_nodes[j + 1] = array_of_nodes[j];
            j = j - 1;
        }
        array_of_nodes[j + 1] = temp;
    }
}

void naive_top_k_gumbel_log(node nodeArray[], int array_of_nodes[], int size_of_array, int k)
{
    // Only when k < size_of_array

    // Sort the first k elements using insertion sort
    insertion_sort_reverse(nodeArray, array_of_nodes, k);

    // The minimum of the top k elements
    int current_min = nodeArray[array_of_nodes[k - 1]].gumbel_plus_log;

    // Insert the new value at the correct position
    for (int i = k; i < size_of_array; ++i)
    {
        int temp = array_of_nodes[i];
        // Only difference with insertion sort : the insertion start at k instead of i
        int j = k - 1;
        while (j >= 0 && nodeArray[temp].gumbel_plus_log > nodeArray[array_of_nodes[j]].gumbel_plus_log)
        {
            array_of_nodes[j + 1] = array_of_nodes[j];
            j = j - 1;
        }
        array_of_nodes[j + 1] = temp;
    }
}

int sequential_halving(node nodeArray[], int array_of_top_nodes[], int size_of_array, int max_visits, int log_m, S_BOARD& board, DoubleHeadedNN& model, int& newId)
{
    // Max_visits is the current max number of visits for a node

    if (size_of_array <= 1) {
        return array_of_top_nodes[0];
    } else {
        // Calculate the number of visits
        int number_visits = NUMBER_OF_SIMULATIONS / (log_m * size_of_array);

        if (number_visits == 0)
            number_visits = 1;

        for (int i = 0; i < size_of_array; ++i) {
            for (int j = 0; j < number_visits; ++j) {
                one_simulation(nodeArray, array_of_top_nodes[i], board, model, newId);
            }

            // update the current_score of every move
            int current_indice = array_of_top_nodes[i];
            int color_multiplier = color[board.thisTurnColor];
            nodeArray[current_indice].current_score = nodeArray[current_indice].gumbel_plus_log + sigma(color_multiplier * nodeArray[current_indice].Q_value, max_visits);
        }

        // find top k moves
        naive_top_k_score(nodeArray, array_of_top_nodes, size_of_array, size_of_array / 2);

        // Select the (size_of_array / 2) best moves
        return sequential_halving(nodeArray, array_of_top_nodes, size_of_array / 2, max_visits + number_visits, log_m, board, model, newId);
    }
}

int gumbel_planning(node nodeArray[], int array_of_nodes[], int size_of_array, int m, S_BOARD& board, DoubleHeadedNN& model, int& newId)
{
    // Precalculate the log probabilities before calling this function

    // Sample from Gumbel distribution
    float gumbel_samples[size_of_array];

    sample_gumbel(gumbel_samples, size_of_array);

    // Update the gumbel_plus_log attributes
    for (int i = 0; i < size_of_array; ++i)
    {
        nodeArray[array_of_nodes[i]].gumbel_plus_log = nodeArray[array_of_nodes[i]].log_policy + gumbel_samples[i];
    }

    // Find the top m moves according to gumbel_plus_log
    if (m >= size_of_array)
    {
        m = size_of_array;
    }
    else
    {
        naive_top_k_gumbel_log(nodeArray, array_of_nodes, size_of_array, m);
    }

    // Use sequential halving to determine the best node from the top m
    int log_m = (int)ln2(m);
    int best_indice = sequential_halving(nodeArray, array_of_nodes, m, 0, log_m, board, model, newId);

    return best_indice;
}

int choose_with_Gumbel(node nodeArray[], int& newId, S_BOARD& board, DoubleHeadedNN& model) {
    // Return the chosen move

    // Calculate legal moves
    int moveArray[OUTPUT_DIM];
    int moves_size;
    legalMoves(moves_size, moveArray, board);

    // Translate to NN inputs
    bool Inputs[INPUT_DIM];
    translate_to_NN_inputs(Inputs, board);

    int zero = 0;
    first_two_layers(model, Inputs);
    
    int array_of_nodes[moves_size];

    for (int i = 0; i < moves_size; ++i) {
        array_of_nodes[i] = newId + i;
    }

    // Expand
    clipped_log_policy_output(model, moveArray, moves_size, board.thisTurnColor);
    createChildren(nodeArray, zero, newId, model, moveArray, moves_size, model.log_policyOutput);

    int best_indice = gumbel_planning(nodeArray, array_of_nodes, moves_size, NUMBER_OF_TOP_MOVES, board, model, newId); 

    return nodeArray[best_indice].move;
 
}


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
