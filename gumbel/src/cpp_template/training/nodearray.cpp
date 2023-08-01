#include "../neural_network.cpp"
#include "../game.cpp"
#include <cstring>


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