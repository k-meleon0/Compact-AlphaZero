import subprocess
import load_model
import eval_edit_template


def execute_cpp():
    """ Execute main.cpp for given number of games. """
    subprocess.call(['clang++', "-O3", "-std=c++17", "test.cpp", '-o', "test"])
    tmp=subprocess.call(["./test"])

if __name__ == "__main__":
    editor = eval_edit_template.Editor()
    generation = input("Enter the generation to test (-1 for pure MCTS): ")
    if generation == "-1":
        editor.edit_player_mcts()
        print("Playing pure MCTS")
    else:
        editor.edit_player()
        model = load_model.load_in_python(generation)
        load_model.edit_file(model, "test.cpp", "template_play.cpp")
        print(f"Playing AlphaZero gen{generation}")
    execute_cpp()
