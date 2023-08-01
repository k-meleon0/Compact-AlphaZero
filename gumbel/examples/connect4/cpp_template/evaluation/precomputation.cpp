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
  InitBitMasks();
  InitMoveList();
  InitNodes(nodeArray);
}

// End of precomputation ------------------------------------------------------------------