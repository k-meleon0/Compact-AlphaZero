/* ----------------------------------------------------------------------------------------
------------------------------------------- GAME ------------------------------------------
------------------------------------------------------------------------------------------- */

struct S_BOARD {

    // 0 for the first player, 1 for the second player
    bool thisTurnColor;

    // TODO
};

// Return the initial position
S_BOARD initialise_board() {
    // TODO
}

// Return the initial position as neural network input
void initialise_NNboard(bool boolean_board[]) {
    // TODO
}

// Modify the board
void playAMove(S_BOARD &board, int move) {
    // TODO
}

// Store legal moves in moveArray, in order
void legalMoves(int &moves_size, int moveArray[], S_BOARD &board) {
    // TODO 
}

// return 0 if nothing, 1 if WHITE wins, -1 if BLACK wins, 2 if drawn
int checkWin(S_BOARD &board)  {
    // TODO
}

//use this function after playAMove
// Play a move on the array used as input by the neural network
void NNplayAMove(bool Inputs[], int move, S_BOARD &board) {
    // TODO
}

// Translate the move from board representation to policy space [0; OUTPUT_DIM]
int translate_move(int move, bool thisTurnColor) {
    // TODO
}

// Translate the board to NN inputs
void translate_to_NN_inputs(bool Inputs[], S_BOARD &board) {
    // TODO
}

// End of game -------------------------------------------------------------------------