
""" AlphaZero hyperparameters. """
NUMBER_OF_GENERATIONS = 20
NUMBER_OF_GAMES = 1000 # per generation

C_PUCT = 4

EPSILON = 0.25
ALPHA = 1
ROOT_TEMPERATURE = 1.1

""" Machine learning hyperparameters. """
LEARNING_RATE = 0.05
MOMENTUM = 0.9
EPOCHS = 1
WEIGHT_DECAY = 3e-5
BATCH_SIZE = 256

""" Parameters relative to the game. """
INPUT_DIM = 98 # TODO
OUTPUT_DIM = 14 # TODO
MAX_MOVES = 99 # TODO