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