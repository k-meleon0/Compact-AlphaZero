"""
Combine every template in cpp to a single file.
"""

from os import sep

import configs

import os.path
module_directory = os.path.dirname(os.path.abspath(__file__))

class Editor:

    def __init__(self):
        pass

    def edit_training(self):
        """ Edit the template for the training. """

        template = open("template.cpp", "w")

        # from directory training
        prefix = "training" + sep

        list_of_templates = ["include.cpp", prefix + "macro.cpp", prefix + "gumbel.cpp", "neural_network.cpp", prefix + "nodeArray.cpp", prefix + "precomputation.cpp",
                             "game.cpp", prefix + "bandit.cpp", prefix + "training.cpp", prefix + "main.cpp"]

        dict_of_macros = {"MAX_MOVES": configs.MAX_MOVES,
                          "NUMBER_OF_SIMULATIONS": configs.NUMBER_OF_SIMULATIONS,
                          "NUMBER_OF_TOP_MOVES": configs.NUMBER_OF_TOP_MOVES,
                          "C_VISITS": configs.C_VISITS,
                          "C_SCALE": configs.C_SCALE,
                          "INPUT_DIM": configs.INPUT_DIM,
                          "OUTPUT_DIM": configs.OUTPUT_DIM,}

        for path in list_of_templates:
            path = "cpp_template" + sep + path
            absolute_path = os.path.join(module_directory, path)
            with open(absolute_path, "r") as file:
                if "macro.cpp" in path:
                    for line in file:
                        for key in dict_of_macros:
                            if "#define " + key in line:
                                value = dict_of_macros[key]
                                template.write("#define " + key + " " + str(value) + "\n")
                                break
                        else: 
                            template.write(line)
                elif not "include.cpp" in path:
                    for line in file:
                        if "#include" in line:
                            continue
                        else:
                            template.write(line)
                else:
                    for line in file:
                        template.write(line)
            template.write("\n")
        
        template.close()

    def edit_eval_mcts(self):
        """ Edit the template for the MCTS evaluation. """

        template = open("template.cpp", "w")

        # from directory evaluation
        prefix = "evaluation" + sep

        list_of_templates = ["include.cpp", prefix + "macro.cpp", "neural_network.cpp", prefix + "precomputation.cpp",
                             "game.cpp", prefix + "gametree.cpp", prefix + "mcts.cpp", prefix + "analysis", prefix + "main.cpp"]
 
        for path in list_of_templates:
            absolute_path = os.path.join(module_directory, path)
            with open(absolute_path, "r") as file:
                if path == "macro.cpp":
                    for line in file:
                        template.write(line)
                else:
                    for line in file:
                        template.write(line)
        template.close()
