/* ----------------------------------------------------------------------------------------
------------------------------------------ MACRO ------------------------------------------
------------------------------------------------------------------------------------------- */

// MACRO relative to the game
#define MAX_MOVES 201 // Maximum of moves in the game

// MACRO relative to training
#define NUMBER_OF_SIMULATIONS 200
#define NUMBER_OF_TOP_MOVES 32

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
#define INPUT_DIM 336
#define OUTPUT_DIM 6

// Sign relative to color
int color[2] = {1, -1};

// End of macro ---------------------------------------------------------------------------