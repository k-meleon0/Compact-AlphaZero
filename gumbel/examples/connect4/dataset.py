"""
Sample a dataset from recent games.
"""
from torch.utils.data import Dataset
from numpy.random import randint
import numpy as np
import torch
import shutil
import configs


class CustomDataset(Dataset):
    """The dataset contains three attributes : board, evaluation, and moves"""

    def __init__(self, board, evaluation, moves):
        self.board = board
        self.evaluation = evaluation
        self.moves = moves

    def __getitem__(self, index):
        return (self.board[index], self.evaluation[index], self.moves[index])

    def __len__(self):
        return len(self.evaluation)


def data_to_np_array(
    file_id,
    path="training.txt",
    input_dim=configs.INPUT_DIM,
    output_dim=configs.OUTPUT_DIM,
):
    """Transform data to np array stored in .npz format."""
    evaluation = []
    moves = []
    board = []

    number_of_turns = 0

    with open(path, "r") as text:
        for line in text:
            if len(line) != 1:
                # non empty line
                if number_of_turns % 2 == 0:
                    # copy board
                    one_board = []
                    for l in range(input_dim):
                        one_board.append(int(line[l]))
                    board.append(one_board)

                    # copy evaluation
                    evaluation.append(int(line[input_dim:]))
                else:
                    # copy moves
                    moves.append(int(line.split(";")[0]))
                
                number_of_turns += 1
    
    evaluation = np.array(evaluation, dtype=int)
    moves = np.array(moves, dtype=int)
    board = np.array(board, dtype=bool)

    np.savez_compressed(f"DATA/training{file_id}", board=board, evaluation=evaluation, moves=moves)

def generate_dataset_from_np(
    begin_file_id: int,
    end_file_id: int,
    number_of_samples: int =configs.NUMBER_OF_SAMPLES_PER_EPOCH,
):
    """ Generate a dataset that contains all positions from previous games. """
    boards = []
    evaluations = []
    moves = []
    for i in range(begin_file_id, end_file_id):
        with np.load(f"DATA/training{i}.npz") as npzfile:
            boards.append(npzfile["board"])
            evaluations.append(npzfile["evaluation"])
            moves.append(npzfile["moves"])
    np_board = np.concatenate(boards)
    np_evaluation = np.concatenate(evaluations)
    np_moves = np.concatenate(moves)

    n = np_evaluation.shape[0]

    if n > number_of_samples:
        # sample the positions
        chosen_indices = np.random.choice(n, number_of_samples, replace=False)
        np_board = np_board[chosen_indices]
        np_evaluation = np_evaluation[chosen_indices]
        np_moves = np_moves[chosen_indices]

    board_tensor = torch.from_numpy(np_board).float()
    evaluation_tensor = torch.from_numpy(np_evaluation).float().unsqueeze(1)
    policy_tensor = torch.from_numpy(np_moves).long()

    return CustomDataset(board_tensor, evaluation_tensor, policy_tensor)


def rawcount(filename: str):
    """Count the number of lines in a file."""
    f = open(filename, "rb")
    lines = 0
    buf_size = 1024 * 1024
    read_f = f.raw.read

    buf = read_f(buf_size)
    while buf:
        lines += buf.count(b"\n")
        buf = read_f(buf_size)

    f.close()

    return lines

