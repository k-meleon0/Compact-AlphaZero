/* ----------------------------------------------------------------------------------------
------------------------------------------ MACRO ------------------------------------------
------------------------------------------------------------------------------------------- */

// MACRO relative to the game
#define MAX_MOVES 201 // Maximum of moves in the game
#define C_PUCT 4 // Value of C_PUCT

// MACRO relative to training
#define EPSILON 0.25 // Quantity of noise applied at the root node
#define ALPHA 1 // Parameter of Dirichlet noise
#define ROOT_TEMPERATURE 1.2 // Temperature applied at the policy node

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