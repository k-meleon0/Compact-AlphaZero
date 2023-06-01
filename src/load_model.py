"""
Load model in main.cpp file or in Python.
"""

import numpy as np
import torch
from os import sep
import configs



class DoubleHeadedNN(torch.nn.Module):
  """ Define the double headed model used in AlphaZero"""

  def __init__(self, input_dim=configs.INPUT_DIM, output_dim=configs.OUTPUT_DIM):
    super(DoubleHeadedNN, self).__init__()

    self.commonLinear1 = torch.nn.Linear(input_dim, 64)
    self.commonLinear2 = torch.nn.Linear(64, 64)
    self.policyHead1 = torch.nn.Linear(64, 32)
    self.policyHead2 = torch.nn.Linear(32, output_dim)
    self.valueHead1 = torch.nn.Linear(64, 32)
    self.valueHead2 = torch.nn.Linear(32, 1)
    self.relu = torch.nn.ReLU()
    self.tanh = torch.nn.Tanh()

  def policyForward(self, x):
    x = self.commonLinear1(x)
    x = self.relu(x)
    x = self.commonLinear2(x)
    x = self.relu(x)
    x = self.policyHead1(x)
    x = self.relu(x)
    x = self.policyHead2(x)
    # results are in logits
    return x

  def valueForward(self, x):
    x = self.commonLinear1(x)
    x = self.relu(x)
    x = self.commonLinear2(x)
    x = self.relu(x)
    x = self.valueHead1(x)
    x = self.relu(x)
    x = self.valueHead2(x)
    x = self.tanh(x)
    # result is in [-1, 1]
    return x

def load_in_python(generation):
    """ Return the DoubleHeadedNN model store as gen{generation}. """
    # Loading the model
    model = DoubleHeadedNN()
    PATH = 'models' + sep + f'gen{generation}'
    model.load_state_dict(torch.load(PATH))
    return model

def save_model(model, generation):
    """ Save the model as gen{generation}. """
    # Saving the model
    PATH = 'models' + sep + f'gen{generation}'
    torch.save(model.state_dict(), PATH)

# Export model to C code
def WeightsC(array, name_in_string):
    """ Return a string that represents the weights of a given layer in cpp. """
    input, output = np.shape(array)
    weights_string = ""
    weights_string += f"{name_in_string} = "
    weights_string += "{"

    for i in range(input - 1):
        weights_string += "{"
        for j in range(output - 1):
            weights_string += str(array[i][j])
            weights_string += ","
        #last j
        weights_string += str(array[i][-1])
        weights_string += "},"
    #last i
    weights_string += "{"
    for j in range(output - 1):
        weights_string += str(array[-1][j])
        weights_string += ","
    #last i and last j
    weights_string += str(array[-1][-1])
    weights_string += "}"

    weights_string += "},\n"
    return weights_string

def BiasC(array, name_in_string):
    """ Return a string that represents the bias of a given layer. """
    output = np.shape(array)[0]
    bias_string = ""
    bias_string += f"{name_in_string} = "
    bias_string += "{"

    for i in range(output - 1):
        bias_string += str(array[i])
        bias_string += ","
    bias_string += str(array[-1])

    bias_string += "},\n"
    return bias_string

def edit_file(model, path, template_path):
    """ Change the main.cpp file so that it contains the model. """
    new_file = open(path, "w")
    template = open(template_path, "r")
    with open(template_path, "r") as template:
        for line in template:
            if "// INSERT MODEL WEIGHTS" in line:
                for i in range(2):
                    for name, layer in model.named_modules():
                        if name == "commonLinear1":
                            weight_attribute = ".weights0"
                            bias_attribute = ".bias0"
                        elif name == "commonLinear2":
                            weight_attribute = ".weights1"
                            bias_attribute = ".bias1"
                        elif name == "valueHead1":
                            weight_attribute = ".valueWeights0"
                            bias_attribute = ".valueBias0"
                        elif name == "valueHead2":
                            weight_attribute = ".valueWeights1"
                            bias_attribute = ".valueBias1"
                        elif name == "policyHead1":
                            weight_attribute = ".policyWeights0"
                            bias_attribute = ".policyBias0"
                        elif name == "policyHead2":
                            weight_attribute = ".policyWeights1"
                            bias_attribute = ".policyBias1"
                        else:
                            continue
                        if i == 0:
                            new_file.write(WeightsC(torch.transpose(layer.state_dict()['weight'], 0, 1).numpy(), weight_attribute))
                        else:
                            new_file.write(BiasC(layer.state_dict()['bias'].numpy(), bias_attribute))
            else:
                new_file.write(line)

    new_file.close()

# Create gen0
if __name__ == "__main__":
    model = DoubleHeadedNN()
    save_model(model, 0)
