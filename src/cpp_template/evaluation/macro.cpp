/* ----------------------------------------------------------------------------------------
------------------------------------------ MACRO ------------------------------------------
------------------------------------------------------------------------------------------- */

// MACRO relative to the evaluation
#define NUMBER_OF_GAMES 10

#define MCTS_SEARCHES 20000
#define AZ_SEARCHES 30

#define ANALYSIS false

// MACRO relative to the game
#define MAX_MOVES 201 // Maximum of moves in the game
#define C_PUCT 1.1 // Value of C_PUCT

// MACRO relative to pure MCTS
#define C_VALUE 0.8


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
#define INPUT_DIM 336
#define OUTPUT_DIM 6

// Sign relative to color
int color[2] = {1, -1};

// End of macro ---------------------------------------------------------------------------