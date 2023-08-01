#include "nodearray.cpp"
#include "macro.cpp"
#include "precomputation.cpp"
#include "gumbel.cpp"
#include "../neural_network.cpp"

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
