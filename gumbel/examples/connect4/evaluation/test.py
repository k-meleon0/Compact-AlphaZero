import subprocess
import load_model
import eval_edit_template


def execute_cpp():
    """ Execute main.cpp for given number of games. """
    subprocess.call(['clang++', "-O3", "-std=c++17", "eval.cpp", '-o', "eval"])
    tmp=subprocess.call(["./eval"])

if __name__ == "__main__":
    editor = eval_edit_template.Editor()
    editor.edit_eval_mcts()
    generation = input("Enter the generation to test : ")
    model = load_model.load_in_python(generation)
    load_model.edit_file(model, "eval.cpp", "template_eval.cpp")
    execute_cpp()
