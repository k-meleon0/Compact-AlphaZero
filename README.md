# Compact-AlphaZero

A small but efficient implementation of AlphaZero in Python and C++.

## General description

This is a hobby project I developed in order to better understand the central ideas behind Deep Reinforcement Learning. This repository contains a general implementation of AlphaZero, an algorithm where the model learns by playing against itself from scratch in order to achieve higher levels of play in board games.

In this implementation, the model is able to attain great levels of play in small games in a short amount of time, using a single thread on CPU.

## Requirements

- A recent version of PyTorch
- clang++

## Usage

### Coding the rules

In order to train an agent, you first need to hard code the rules of the game you want it to train on. To do so, go to `src/cpp_template/game.cpp` and complete every TODO. Note that the input to the neural network must be an array of booleans for performance reasons. For further speed improvements, you may go to `src/cpp_template/training/precomputation.cpp` and precompute some important calculations.

### Hyperparameters settings

Next, go to `src/configs.py` in order to specify the hyperparameters for training. `INPUT_DIM` designate the input dimension of the double-headed neural network and `OUTPUT_DIM` the output dimension of the policy head. `MAX_MOVES` should be assigned to an upper bound of the amount of turns the game can last.

### Training

Go to `src` and launch `python3 load_model.py` in order to create the gen0 neural network. Then enter `python3 pipeline.py` to start the training. You may stop at any point during the self play session and resume later.

### Evaluation

This implementation doesn't evaluate the model against older versions. Instead, you may evaluate the performance of a given generation against the classic MCTS (Monte Carlo Tree Search) algorithm.

Go to `evaluation` and modify `eval_configs.py` to adjust the hyperparameters. Note that if you set `ANALYSIS` to `"true"` you will see what the model thinks of the position. Start the evaluation with `python3 test.py` and enter the generation you want to assess.

### Playing against the model

It is preferable to first implement printing a representation of the board. Complete the `PrintBoard` function in `src/cpp_template/evaluation/analysis.cpp`.

Go to `evaluation` and modify `eval_configs.py` (the same file for evaluating the model) to adjust the hyperparameters. Set `ANALYSIS` to `"false"` to have a fair match. Input `python3 play.py` to start playing.

## Performance

This implementation was tested on several games. You may see the example of Connect Four inside the `examples` folder.

On Connect Four, the model achieved a great level of play after one and a half of training on a laptop, which consisted of 40 generations (40,000 games). It was able to equalize against the Connect Four solver as the first player. You may replicate the results in the `connect4` folder where the hyperparameters have been adjusted.

On Oware Abapa (not shown in examples), the model arrived 18th on the leaderboard of CodinGame after four hours of training which consisted of around 120 generations.

## Project status

The project is no longer being worked on as I an now studying the methods on games with imperfect information.




