"""
Combine every template in cpp to a single file.
"""

from os import sep
from os import pardir

import eval_configs

import os.path
module_directory = os.path.dirname(os.path.abspath(__file__))

class Editor:

    def __init__(self):
        pass

    def edit_eval_mcts(self):
        """ Edit the template for the MCTS evaluation. """

        template = open("template_eval.cpp", "w")

        # from directory evaluation
        prefix = "evaluation" + sep

        list_of_templates = ["include.cpp", prefix + "macro.cpp", "neural_network.cpp", prefix + "precomputation.cpp",
                             "game.cpp", prefix + "gametree.cpp", prefix + "mcts.cpp", prefix + "analysis.cpp", prefix + "main.cpp"]
 
        dict_of_macros = {"MAX_MOVES": eval_configs.MAX_MOVES,
                          "C_PUCT": eval_configs.C_PUCT,
                          "NUMBER_OF_GAMES": eval_configs.NUMBER_OF_GAMES,
                          "MCTS_SEARCHES": eval_configs.MCTS_SEARCHES,
                          "AZ_SEARCHES": eval_configs.AZ_SEARCHES,
                          "ANALYSIS": eval_configs.ANALYSIS,
                          "C_VALUE": eval_configs.C_VALUE,
                          "INPUT_DIM": eval_configs.INPUT_DIM,
                          "OUTPUT_DIM": eval_configs.OUTPUT_DIM,}

        for path in list_of_templates:
            path = "cpp_template" + sep + path
            path = os.path.join(pardir, path)
            absolute_path = os.path.join(module_directory, path)
            absolute_path = os.path.abspath(absolute_path)
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
                else:
                    for line in file:
                        template.write(line)
            template.write("\n")

        template.close()

    def edit_player(self):
        """ Edit the template for playing against AlphaZero. """
        template = open("template_play.cpp", "w")

        # from directory evaluation
        prefix = "evaluation" + sep

        list_of_templates = ["include.cpp", prefix + "macro.cpp", "neural_network.cpp", prefix + "precomputation.cpp",
                             "game.cpp", prefix + "gametree.cpp", prefix + "mcts.cpp", prefix + "analysis.cpp", prefix + "player.cpp"]
 
        dict_of_macros = {"MAX_MOVES": eval_configs.MAX_MOVES,
                          "C_PUCT": eval_configs.C_PUCT,
                          "NUMBER_OF_GAMES": eval_configs.NUMBER_OF_GAMES,
                          "MCTS_SEARCHES": eval_configs.MCTS_SEARCHES,
                          "AZ_SEARCHES": eval_configs.AZ_SEARCHES,
                          "ANALYSIS": eval_configs.ANALYSIS,
                          "C_VALUE": eval_configs.C_VALUE,
                          "INPUT_DIM": eval_configs.INPUT_DIM,
                          "OUTPUT_DIM": eval_configs.OUTPUT_DIM,}

        for path in list_of_templates:
            path = "cpp_template" + sep + path
            path = os.path.join(pardir, path)
            absolute_path = os.path.join(module_directory, path)
            absolute_path = os.path.abspath(absolute_path)
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
                else:
                    for line in file:
                        template.write(line)
            template.write("\n")

        template.close()

    def edit_player_mcts(self):
        """ Edit the file for playing against pure MCTS. """
        template = open("test.cpp", "w")

        # from directory evaluation
        prefix = "evaluation" + sep

        list_of_templates = ["include.cpp", prefix + "macro.cpp", "neural_network.cpp", prefix + "precomputation.cpp",
                             "game.cpp", prefix + "gametree.cpp", prefix + "mcts.cpp", prefix + "analysis.cpp", prefix + "player_mcts.cpp"]
 
        dict_of_macros = {"MAX_MOVES": eval_configs.MAX_MOVES,
                          "C_PUCT": eval_configs.C_PUCT,
                          "NUMBER_OF_GAMES": eval_configs.NUMBER_OF_GAMES,
                          "MCTS_SEARCHES": eval_configs.MCTS_SEARCHES,
                          "AZ_SEARCHES": eval_configs.AZ_SEARCHES,
                          "ANALYSIS": eval_configs.ANALYSIS,
                          "C_VALUE": eval_configs.C_VALUE,
                          "INPUT_DIM": eval_configs.INPUT_DIM,
                          "OUTPUT_DIM": eval_configs.OUTPUT_DIM,}

        for path in list_of_templates:
            path = "cpp_template" + sep + path
            path = os.path.join(pardir, path)
            absolute_path = os.path.join(module_directory, path)
            absolute_path = os.path.abspath(absolute_path)
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
                else:
                    for line in file:
                        template.write(line)
            template.write("\n")

        template.close()
