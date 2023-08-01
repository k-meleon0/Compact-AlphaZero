/* ----------------------------------------------------------------------------------------
--------------------------------------- PRECOMPUTATION ------------------------------------
------------------------------------------------------------------------------------------- */

// TODO

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

int getRand(const int& A, const int& B) {
    static std::random_device randDev;
    static std::mt19937 twister(randDev());
    static std::uniform_int_distribution<int> dist;

    dist.param(std::uniform_int_distribution<int>::param_type(A, B));
    return dist(twister);
}

// Initialise the nodeArray
void InitNodes(int nodeArray[]) {
    int i = 0;
    for (i = 0; i < 7; ++i) {
        nodeArray[i] = 0;
    }
}

// Initialise precomputed tables :
void AllInit(int nodeArray[]) {
  // TODO
  InitNodes(nodeArray);
}

// End of precomputation ------------------------------------------------------------------