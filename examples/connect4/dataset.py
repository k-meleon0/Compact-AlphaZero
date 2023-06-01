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
  """ The dataset contains three attributes : board, evaluation, and moves"""

  def __init__(self, board, evaluation, moves):
    self.board = board
    self.evaluation = evaluation
    self.moves = moves
  
  def __getitem__(self, index):
    return (self.board[index], self.evaluation[index], self.moves[index])

  def __len__(self):
    return len(self.evaluation)


# Selective sampling
# Uniformly sample one position per game


def generate_dataset(begin_file_id: int, end_file_id: int, input_dim=configs.INPUT_DIM, output_dim=configs.OUTPUT_DIM, number_of_games_per_file=configs.NUMBER_OF_GAMES):
    """ Sample a dataset spanning over the range from begin_file_id to end_file_id. 
    One position is sampled from each game. """
    evaluation = []
    policy = []

    paths = [f"DATA/training{k}.txt" for k in range(begin_file_id, end_file_id)]


    np_board = np.zeros((number_of_games_per_file*(end_file_id - begin_file_id), input_dim))

    temp_lines = []

    total = 0

    for path in paths:
        with open(path, 'r') as text:
            nan = 0
            number_of_turns = 0
            for line in text:
                if len(line) == 1 and number_of_turns != 0:
                    # empty lines
                    number_of_turns //= 2
                    rand = randint(number_of_turns)
                    line = temp_lines[2 * rand]
                    for l in range(input_dim):
                        if line[l] == ';':
                            print(line)
                        np_board[total, l] = int(line[l])
                    evaluation.append(float(line[input_dim:]))
                    line = temp_lines[2 * rand + 1]
                    probabilities = line.split(sep=";")
                    probabilities.pop()
                    for k in range(output_dim):
                        if float(probabilities[k]) != float(probabilities[k]):
                            # check for nan values
                            nan += 1
                            print("NAN")
                            break
                    probabilities = [float(x) for x in probabilities]
                    sum_ = sum(probabilities)
                    probabilities = [x/sum_ for x in probabilities]
                    policy.append(probabilities)
                    total += 1
                    number_of_turns = 0
                    temp_lines = []
                    continue
                elif len(line) == 1:
                    continue
                else:
                    number_of_turns += 1
                    temp_lines.append(line)
    np_board = np_board[:total, :]
    evaluation = np.array(evaluation)
    policy = np.array(policy)
    evaluation = np.array(evaluation)
    policy = np.array(policy)
    board_tensor = torch.from_numpy(np_board).float()
    evaluation_tensor = torch.from_numpy(evaluation).float().unsqueeze(1)
    policy_tensor = torch.from_numpy(policy).float()

    return CustomDataset(board_tensor, evaluation_tensor, policy_tensor)


def rawcount(filename: str):
    """ Count the number of lines in a file. """
    f = open(filename, 'rb')
    lines = 0
    buf_size = 1024 * 1024
    read_f = f.raw.read

    buf = read_f(buf_size)
    while buf:
        lines += buf.count(b'\n')
        buf = read_f(buf_size)

    f.close()

    return lines

def copy_to_file(id_new_file: int):
    """ Copy training.txt to the file DATA/training{id_new_file}.txt. """
    shutil.copyfile("training.txt", f"DATA/training{id_new_file}.txt")
