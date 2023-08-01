
""" AlphaZero hyperparameters. """
NUMBER_OF_GENERATIONS = 40
NUMBER_OF_GAMES = 1000 # per generation

NUMBER_OF_SIMULATIONS = 200
NUMBER_OF_TOP_MOVES = 16

C_VISITS = 50
C_SCALE = 1

""" Machine learning hyperparameters. """
LEARNING_RATE = 0.05
MOMENTUM = 0.9
EPOCHS = 1
WEIGHT_DECAY = 3e-5
BATCH_SIZE = 256

NUMBER_OF_SAMPLES_PER_EPOCH = 6000

""" Parameters relative to the game. """
INPUT_DIM = 98
OUTPUT_DIM = 14
MAX_MOVES = 99