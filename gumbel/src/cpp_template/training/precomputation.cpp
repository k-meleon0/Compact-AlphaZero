#include "nodearray.cpp"

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
  // TODO
  InitNodes(nodeArray);
}

// End of precomputation ------------------------------------------------------------------