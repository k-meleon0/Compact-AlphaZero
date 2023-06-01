import torch
import dataset
import load_model
from torch.utils.data import DataLoader
import configs



class ModelTrainer():

    """ Custom class for training a model. """

    def __init__(self, lr = configs.LEARNING_RATE, momentum = configs.MOMENTUM, n_epochs = configs.EPOCHS, weight_decay = configs.WEIGHT_DECAY, batch_size = configs.BATCH_SIZE):
        self.device = "cpu"

        self.value_loss_fn = torch.nn.MSELoss(reduction= 'mean')
        self.policy_loss_fn = torch.nn.CrossEntropyLoss()

        self.lr = lr
        self.momentum = momentum
        self.n_epochs = n_epochs
        self.weight_decay = weight_decay
        self.model = load_model.DoubleHeadedNN()
        self.optimizer = torch.optim.SGD(params = self.model.parameters(), lr=self.lr, momentum = self.momentum, weight_decay = self.weight_decay)

        self.batch_size = batch_size
        self.custom_dataset = None
        self.train_loader = None

    def load_model(self, generation: int):
        """ Load the model and change the optimizer. """
        self.model = load_model.load_in_python(generation)
        self.optimizer = torch.optim.SGD(params = self.model.parameters(), lr=self.lr, momentum = self.momentum, weight_decay = self.weight_decay)
    

    def value_policy_step(self, board_batch, evaluation_batch, policy_batch):
        """ Makes a step to reduce both value and policy loss """
        self.model.train()
        self.optimizer.zero_grad()
        evaluation_pred = self.model.valueForward(board_batch)
        moves_pred = self.model.policyForward(board_batch)
        loss = self.value_loss_fn(evaluation_pred, evaluation_batch) + self.policy_loss_fn(moves_pred, policy_batch)
        loss.backward()
        self.optimizer.step()
        return loss.item()

    def create_dataset(self, begin_id: int, end_id: int):
        """ Get a train_loader. """
        self.custom_dataset = dataset.generate_dataset(begin_id, end_id)
        self.train_loader = DataLoader(dataset=self.custom_dataset, batch_size=self.batch_size, shuffle=True)

    def train_model(self):
        """ Train the model for a given number of epochs. """

        assert self.train_loader is not None

        for _ in range(self.n_epochs):
            total_value_policy_loss_this_epoch = 0
            total_correct_labels_this_epoch = 0
            steps = 0
            # the training loop
            for board_batch, evaluation_batch, policy_batch in self.train_loader:
                board_batch = board_batch.to(self.device)
                evaluation_batch = evaluation_batch.to(self.device)
                policy_batch = policy_batch.to(self.device)

                vp_loss = self.value_policy_step(board_batch, evaluation_batch, policy_batch) 

                # Calculate accuracy
                moves_pred = self.model.policyForward(board_batch)
                _, preds = torch.max(moves_pred.data, 1)
                _, train_moves = torch.max(policy_batch.data, 1)

                acc_all = (preds == train_moves).float().mean().item()
                total_correct_labels_this_epoch += acc_all * self.batch_size    

                total_value_policy_loss_this_epoch += vp_loss * self.batch_size

                steps += 1
            print("Value-policy loss :", total_value_policy_loss_this_epoch/(steps*self.batch_size))
            print("Accuracy :", total_correct_labels_this_epoch/(steps*self.batch_size))
