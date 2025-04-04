# NIST-developed software is provided by NIST as a public service. You may
# use, copy and distribute copies of the software in any medium, provided that
# you keep intact this entire notice. You may improve, modify and create
# derivative works of the software or any portion of the software, and you may
# copy and distribute such modifications or works. Modified works should carry
# a notice stating that you changed the software and should note the date and
# nature of any such change. Please explicitly acknowledge the National
# Institute of Standards and Technology as the source of the software.
#
# NIST-developed software is expressly provided "AS IS." NIST MAKES NO
# WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF
# LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST
# NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE
# UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST
# DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE
# SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE
# CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
#
# You are solely responsible for determining the appropriateness of using and
# distributing the software and you assume all risks associated with its use,
# including but not limited to the risks and costs of program errors,
# compliance with applicable laws, damage to or loss of data, programs or
# equipment, and the unavailability or interruption of operation. This
# software is not intended to be used in any situation where a failure could
# cause risk of injury or damage to property. The software developed by NIST
# employees is not subject to copyright protection within the United States.

import pandas as pd
import numpy as np
from sklearn.preprocessing import LabelEncoder
from sklearn.model_selection import train_test_split
import time

import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import Dataset, DataLoader, WeightedRandomSampler


# DataSet class that will be used to feed data to the neural network
# dynamically, according to the training speed and available memory
# We just need to implement the methods shown here and perform any
# processing if needed
class ClassifierDataset(Dataset):

    def __init__(self, X_data, Y_data):
        self.X_data = X_data
        self.Y_data = Y_data

    def __getitem__(self, index):
        return self.X_data[index], self.Y_data[index]

    def __len__ (self):
        return len(self.X_data)


# Class that implements the neural network model
# This is a simple 4-layer neural network that inherits
# from MulticlassClassification
class MulticlassClassification(nn.Module):
    def __init__(self, num_feature, num_class):
        super(MulticlassClassification, self).__init__()

        self.layer_1 = nn.Linear(num_feature, 512)
        self.layer_2 = nn.Linear(512, 128)
        self.layer_3 = nn.Linear(128, 64)
        self.layer_out = nn.Linear(64, num_class)

        self.relu = nn.ReLU()

    # The forward method defines the topology, the connections
    # between the layers and components defined in the __init__
    # method
    def forward(self, x):
        x = self.layer_1(x)
        x = self.relu(x)

        x = self.layer_2(x)
        x = self.relu(x)

        x = self.layer_3(x)
        x = self.relu(x)

        x = self.layer_out(x)

        return x

# Method that defines the accuracy calculation for the training iterations
# In this case, we compare the predicted classes with the true classes,
# count the correct classifications, and get the ratio of correct over total
def multi_acc(Y_pred, Y_test):
    Y_pred_softmax = torch.log_softmax(Y_pred, dim = 1)
    _, Y_pred_tags = torch.max(Y_pred_softmax, dim = 1)

    correct_pred = (Y_pred_tags == Y_test).float()
    acc = correct_pred.sum() / len(correct_pred)

    acc = torch.round(acc) * 100

    return acc

# Method that gets the class distribution in the training dataset.
# This is used to set the class weights for random sampling during training
def get_class_distribution(obj):
    count_dict = {
        "A": 0,
        "B": 0,
        "C": 0,
        "D": 0,
    }

    for i in obj:
        if i == 0:
            count_dict["A"] += 1
        elif i == 1:
            count_dict["B"] += 1
        elif i == 2:
            count_dict["C"] += 1
        elif i == 3:
            count_dict["D"] += 1
        else:
            print("Check classes.")

    return count_dict


# Read the training and evaluation dataset. In this case we are using a single file
# that will be split in 3 sets
df = pd.read_csv ("training.data", delim_whitespace=True, header=None)
# Columns 1 to 12 are inputs
X = df.iloc [:,0:-1]
# Column 13 is the true class
Y = df.iloc [:,-1]

# Split the dataset: 20 % of the entries will be used for testing
X_trainval, X_test, Y_trainval, Y_test = train_test_split (X, Y, test_size = 0.2, stratify = Y, random_state = 10)
# Split the other 80 % between training and validation
X_train, X_val, Y_train, Y_val = train_test_split (X_trainval, Y_trainval, random_state = 20)

# Format conversion to NumPy arrays
X_train, Y_train = np.array (X_train), np.array (Y_train)
X_val, Y_val = np.array (X_val), np.array (Y_val)
X_test, Y_test = np.array (X_test), np.array (Y_test)

# Convert the raw input into PyTorch datasets.
# These datasets will be managed by DataLoaders that will throttle the speed at which the input file is read
# depending on parallelization capabilities, training speed, and memory available
train_dataset = ClassifierDataset(torch.from_numpy(X_train).float(), torch.from_numpy(Y_train).long())
val_dataset = ClassifierDataset(torch.from_numpy(X_val).float(), torch.from_numpy(Y_val).long())
test_dataset = ClassifierDataset(torch.from_numpy(X_test).float(), torch.from_numpy(Y_test).long())

# Create a list of classes in the training dataset
target_list = []
for _, t in train_dataset:
    target_list.append(t)

# And use the list to randomly select the order of the
# classes for the output
target_list = torch.tensor(target_list)
target_list = target_list[torch.randperm(len(target_list))]

# Compute the class weights for the random sampling
class_count = [i for i in get_class_distribution(Y_train).values()]
class_weights = 1./torch.tensor(class_count, dtype=torch.float)

# Create a weighted random sampler for out training data
class_weights_all = class_weights[target_list]
weighted_sampler = WeightedRandomSampler(
    weights=class_weights_all,
    num_samples=len(class_weights_all),
    replacement=True
)

# Set training parameters
EPOCHS = 3
BATCH_SIZE = 10
LEARNING_RATE = 0.0007
NUM_FEATURES = len(X.columns)
NUM_CLASSES = 4

# Initialize the data loaders for each dataset
train_loader = DataLoader(dataset=train_dataset, batch_size=BATCH_SIZE, sampler=weighted_sampler)
val_loader = DataLoader(dataset=val_dataset, batch_size=1)
test_loader = DataLoader(dataset=test_dataset, batch_size=1)

# Set the context for the model: if hardware acceleration is available, use it
# Otherwise, use the CPU
device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
# Instantiate the Neural Network
model = MulticlassClassification (num_feature = NUM_FEATURES, num_class = NUM_CLASSES)
model.to (device)

# Define the Loss metric and the optimizer
criterion = nn.CrossEntropyLoss(weight=class_weights.to(device))
optimizer = optim.Adam(model.parameters(), lr=LEARNING_RATE)


# Create dictionaries to keep track of the training statistics
accuracy_stats = {
    'train': [],
    "val": []
}
loss_stats = {
    'train': [],
    "val": []
}

print ("Begin Training")

# Train the model
for e in  (range (1, EPOCHS + 1)):
    start_time = time.time ()
    train_epoch_loss = 0
    train_epoch_acc = 0

    # Set the model in training mode
    model.train ()
    # For as long as the training data loader has data ....
    for X_train_batch, Y_train_batch in train_loader:
        # Load the data in the proper device ...
        X_train_batch, Y_train_batch = X_train_batch.to(device), Y_train_batch.to(device)
        # Reset the optimizer gradients to 0...
        optimizer.zero_grad()

        # Train the model
        Y_train_pred = model(X_train_batch)

        # Compute loss and accuracy for the batch
        train_loss = criterion(Y_train_pred, Y_train_batch)
        train_acc = multi_acc(Y_train_pred, Y_train_batch)

        # Perform backpropagation
        train_loss.backward()
        # Update optimizer gradients
        optimizer.step()

        # Update training statistics
        train_epoch_loss += train_loss.item()
        train_epoch_acc += train_acc.item()

    # In each epoch, after training we do validation. We set the environment
    # in a state in which the gradients will not be updated
    with torch.no_grad():

        val_epoch_loss = 0
        val_epoch_acc = 0

        # Set the model in no-training mode
        model.eval()
        # Load the data from the validation loader
        for X_val_batch, Y_val_batch in val_loader:
            X_val_batch, Y_val_batch = X_val_batch.to(device), Y_val_batch.to(device)

            # Run the inputs through the model
            Y_val_pred = model(X_val_batch)

            # Compute loss and accuracy
            val_loss = criterion(Y_val_pred, Y_val_batch)
            val_acc = multi_acc(Y_val_pred, Y_val_batch)

            # Accumulate statistics
            val_epoch_loss += val_loss.item()
            val_epoch_acc += val_acc.item()

    # Print the training and validation statistics for this epoch
    loss_stats['train'].append(train_epoch_loss/len(train_loader))
    loss_stats['val'].append(val_epoch_loss/len(val_loader))
    accuracy_stats['train'].append(train_epoch_acc/len(train_loader))
    accuracy_stats['val'].append(val_epoch_acc/len(val_loader))
    total_time = time.time () - start_time

    print(f'Epoch {e+0:03}: | Train Loss: {train_epoch_loss/len(train_loader):.5f} | Val Loss: {val_epoch_loss/len(val_loader):.5f} | Train Acc: {train_epoch_acc/len(train_loader):.3f}| Val Acc: {val_epoch_acc/len(val_loader):.3f}| Time: {total_time:.5f}')


# After the training is complete, we perform the testing
# We will also save the trained model. As the gradients use lazy
# evaluation, we need to run at least one input through the model
# after setting the environment and the model in no-training mode
# before we can save the model
print ("Begin Testing")
test_loss = 0
test_acc = 0
save_pending = True
# Set the environment in no-training mode
with torch.no_grad():
    # Set the model in no-training mode
    model.eval()
    for X_batch, Y_batch in test_loader:
        X_batch, Y_batch = X_batch.to(device), Y_batch.to(device)
        # Run the input through the trained model
        Y_test_pred = model(X_batch)
        # Compute accuracy
        test_acc += multi_acc(Y_test_pred, Y_batch)
        # If this is the first input, we can now save the model
        if save_pending:
            traced_script_module = torch.jit.trace(model, X_batch)
            traced_script_module.save ("saved_trained_classification_pytorch.pt")
            save_pending = False

    # Print the testing statistics
    print(f'Test Acc: {test_acc/len(test_loader):.5f}')

