/* ----------------------------------------------------------------------------------------
------------------------------------------- GAME ------------------------------------------
------------------------------------------------------------------------------------------- */

struct S_BOARD {

  //board[0] = white board; board[1] = black board; board[2] = both
  //Connect 4 plays on a 6*7 board, where 6 is the width
  /*
  6,13,...
  5,12,...
  4,11,...
  3,10,...
  2,9,...
  1,8,15,...
  0,7,14,...
  */
  //the row with 6 + 7k is left at 0 for useful purposes
  U64 board[3];

  //first 7 bits indicate : 0 if column isn't full, 1 if column is full
  U32 playableColumns;

  //0 for first player, 1 for second
  bool thisTurnColor;

};

// Return the initial position
S_BOARD initialise_board() {
  S_BOARD new_board = {{0, 0, 0}, 0, 0};
  return new_board;
}

// Return the initial position as neural network input
void initialise_NNboard(bool boolean_board[]) {
  for (int i = 0; i < INPUT_DIM; ++i) {
    boolean_board[i] = 0;
  }
  for (int i = 84; i < 84 + 7; ++i) {
    boolean_board[i] = true;
  }
}

// Modify the board
void playAMove(S_BOARD &board, int move) {
  U64 copy = ~((board.board[2] >> move*7) & 0b1111111);
  int height = POP(&copy);

  SETBIT(board.board[board.thisTurnColor], height + 7*move);
  SETBIT(board.board[2], height + 7*move);

  if (height == 5)  {
    SETBIT2(board.playableColumns, move);
  }

  board.thisTurnColor ^= 1;

}

// Store legal moves in moveArray, in order
void legalMoves(int &moves_size, int moveArray[], S_BOARD &board) {
  moves_size = moveList[board.playableColumns].size();
  for (int i = 0; i < moves_size; ++i) {
    moveArray[i] = moveList[board.playableColumns][i];
  }
}

// return 0 if nothing, 1 if WHITE win, -1 if BLACK win, 2 if drawn
int checkWin(S_BOARD &board)  {
 //one player cannot win before it's his turn!
  bool colorChecked = board.thisTurnColor ^ 1;

  //four in a row in the same column
  if (board.board[colorChecked] & (board.board[colorChecked] >> 1) & (board.board[colorChecked] >> 2) & (board.board[colorChecked] >> 3)) {
    return color[colorChecked];
  }
  //four in a row in the same row
  if (board.board[colorChecked] & (board.board[colorChecked] >> 7) & (board.board[colorChecked] >> 14) & (board.board[colorChecked] >> 21)) {
    return color[colorChecked];
  }
  //four in a row in diagonal from bottom left to top right
  if (board.board[colorChecked] & (board.board[colorChecked] >> 8) & (board.board[colorChecked] >> 16) & (board.board[colorChecked] >> 24)) {
    return color[colorChecked];
  }
  //four in a row in diagonal from top left to bottom right
  if (board.board[colorChecked] & (board.board[colorChecked] >> 6) & (board.board[colorChecked] >> 12) & (board.board[colorChecked] >> 18)) {
    return color[colorChecked];
  }

  //no more moves to be played
  if (board.playableColumns == 0b1111111) {
    return 2;
  }

  return 0;
}

//use this function after playAMove
// Play A Move on the boolean array used by the neural network
void NNplayAMove(bool Inputs[], int move, S_BOARD &board) {
  for (int i = 0; i < 7; ++i) {
    for (int j = 0; j < 6; ++j) {
      if (board.board[0] & SetMask[7*i + j]) {
        Inputs[6*i + j] = 1;
      }
      else {
        Inputs[6*i + j] = 0;
      }
      if (board.board[1] & SetMask[7*i + j]) {
        Inputs[6*i + j + 42] = 1;
      }
      else {
        Inputs[6*i + j + 42] = 0;
      }
    }
  }
  U64 mask = 0b1111111;
  U64 moves = (~board.playableColumns) & mask;
  if (!board.thisTurnColor) {
    for (int i = 0; i < 7; ++i) {
      if (moves & SetMask[i]) {
        Inputs[84 + i] = 1;
      }
      else {
        Inputs[84 + i] = 0;
      }
    }
    for (int i = 0; i < 7; ++i) {
      Inputs[84 + 7 + i] = 0;
    }
  }
  else {
    for (int i = 0; i < 7; ++i) {
      if (moves & SetMask[i]) {
        Inputs[84 + 7 + i] = 1;
      }
      else {
        Inputs[84 + 7 + i] = 0;
      }
    }
    for (int i = 0; i < 7; ++i) {
      Inputs[84 + i] = 0;
    }
  }
}

// Translate the move from board representation to policy space [0; OUTPUT_DIM]
int translate_move(int move, bool thisTurnColor) {
    if (!thisTurnColor) {
        return move;
    }
    else {
        return move + 7;
    }
}

void translate_to_NN_inputs(bool Inputs[], S_BOARD& board) {
    for (int i = 0; i < 7; ++i) {
        for (int j = 0; j < 6; ++j) {
      if (board.board[0] & SetMask[7 * i + j]) {
        Inputs[6 * i + j] = 1;
      } else {
        Inputs[6 * i + j] = 0;
      }
      if (board.board[1] & SetMask[7 * i + j]) {
        Inputs[6 * i + j + 42] = 1;
      } else {
        Inputs[6 * i + j + 42] = 0;
      }
        }
    }
    U64 mask = 0b1111111;
    U64 moves = (~board.playableColumns) & mask;
    if (!board.thisTurnColor) {
        for (int i = 0; i < 7; ++i) {
      if (moves & SetMask[i]) {
        Inputs[84 + i] = 1;
      } else {
        Inputs[84 + i] = 0;
      }
        }
        for (int i = 0; i < 7; ++i) {
      Inputs[84 + 7 + i] = 0;
        }
    } else {
        for (int i = 0; i < 7; ++i) {
      if (moves & SetMask[i]) {
        Inputs[84 + 7 + i] = 1;
      } else {
        Inputs[84 + 7 + i] = 0;
      }
        }
        for (int i = 0; i < 7; ++i) {
      Inputs[84 + i] = 0;
        }
    }
}


// End of game -------------------------------------------------------------------------