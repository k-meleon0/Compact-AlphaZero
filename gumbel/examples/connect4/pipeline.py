import subprocess

from numpy import average, number

import dataset
import load_model
from model_trainer import ModelTrainer
import configs
import edit_template
import json


""" This version uses neural network always in the point of vue of the first player. (Only the first player plays.) """

class TrainingManager():

    """ Class that controls the training process. """

    def __init__(self, number_of_games = configs.NUMBER_OF_GAMES):
        self.current_generation = 0
        self.read_log()
        self.number_of_games = number_of_games
        self.modeltrainer = ModelTrainer(lr=configs.LEARNING_RATE)
        self.modeltrainer.load_model(self.current_generation)
        

    def read_log(self):
        """ Get the current generation in log. """
        with open("training_log.json", "r") as training_log:
            data = json.load(training_log)
        self.current_generation = data["Current_generation"]


    def write_log(self):
        """ Write the current generation in log. """
        data = {"Current_generation": self.current_generation}
        with open("training_log.json", "w") as training_log:
            json.dump(data, training_log)

    
    def update_generation(self):
        """ Increase generation by one. """
        self.current_generation += 1
        self.write_log()


    def execute_cpp(self, debug=False):
        """ Execute main.cpp for given number of games. """
        if not debug:
            subprocess.call(['clang++', "-O3", "-std=c++17", "main.cpp", '-o', "main"])
        else:
            subprocess.call(['clang++', "-O3", "-std=c++17", "-fsanitize=address", "-fno-omit-frame-pointer", "-Wall", "-g", "main.cpp", "-o", "main"]) 
        tmp=subprocess.call(["./main", f"{self.number_of_games}"])



    def train_model_on_data(self):
        """ Train model after training. """
        # Sliding windown of 20 files
        def window_size(generation, alpha=0.75, c = 250000, beta=0.4, average_samples=20, number_of_games=configs.NUMBER_OF_GAMES):
            return int(c*(1 + beta*(pow((generation + 1)*average_samples*number_of_games/c, alpha) - 1)/alpha) / (average_samples * number_of_games))
        window = window_size(self.current_generation)
        print("Window size : ", window)
        begin_id = max(0, self.current_generation - window)
        end_id = self.current_generation + 1

        self.modeltrainer.create_dataset(begin_id, end_id)
        self.modeltrainer.train_model()


    def one_iteration(self):
        """ Do one complete iteration. """
        print()
        print(f"-----Beginning generation {self.current_generation}-----")
        load_model.edit_file(self.modeltrainer.model, "main.cpp", "template.cpp")
        print("\nBeginning self play.")
        self.execute_cpp()
        print("\nSelf-play finished.")

        dataset.data_to_np_array(self.current_generation)
        print("Dataset generated.")

        print("\nBeginning model training.")
        self.train_model_on_data()
        print("Model training finished.")


        self.update_generation()
        
        load_model.save_model(self.modeltrainer.model, self.current_generation)

        print(f"-----Generation {self.current_generation - 1} done.-----")



if __name__ == "__main__":
    # create the template
    editor = edit_template.Editor()
    editor.edit_training()

    # start training
    training_manager = TrainingManager()
    number_of_generations = configs.NUMBER_OF_GENERATIONS
    for _ in range(number_of_generations):
        training_manager.one_iteration()

