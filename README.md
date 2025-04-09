# ORAN ns-3 Module
A module that can be used in [ns-3](https://www.nsnam.org/) to simulate and
model behavior based on the [O-RAN](https://www.o-ran.org) specifications.
A conference paper that describes the model and its use in a case study can
be found here: [https://doi.org/10.1145/3592149.3592157](https://doi.org/10.1145/3592149.3592157)

# Table of Contents
* [Project Overview](#project-overview)
  * [Contact](#contact)
* [Model Description](#model-description)
* [Features](#features)
* [Minimum Requirements](#minimum-requirements)
  * [Optional Dependencies](#optional-dependencies)
* [Installation](#installation)
  * [Clone (Recommended)](#clone-recommended)
  * [Download ZIP](#download-zip)
  * [Connecting the Module Quickly](#connecting-the-module-quickly)
    * [Cmake](#cmake-quick-connect)
    * [Code](#code-quick-connect)
  * [Connecting the Module Safely](#connecting-the-module-safely)
    * [Cmake](#cmake-safe-connect)
    * [Code](#code-safe-connect)
* [Updating](#updating)
  * [Clone](#clone)
  * [ZIP](#zip)
* [ML Support](#ml-support)
  * [ONNX](#onnx)
  * [PyTorch](#pytorch)
* [Documentation](#documentation)
* [Running the Examples](#running-the-examples)
  * [Random Walk Example](#random-walk-example)
  * [LTE to LTE Distance Handover Example](#lte-to-lte-distance-handover-example)
  * [LTE to LTE Distance Handover With Helper Example](#lte-to-lte-distance-handover-with-helper-example)
  * [LTE to LTE Distance Handover With LM Processing Delay Example](#lte-to-lte-distance-handover-with-lm-processing-delay-example)
  * [LTE to LTE Distance Handover With LM Query Trigger Example](#lte-to-lte-distance-handover-with-lm-query-trigger-example)
  * [Keep-Alive Example](#keep-alive-example)
  * [Data Repository Example](#data-repository-example)
  * [Multiple Network Devices Example](#multiple-network-devices-example)
  * [LTE to LTE ML Handover Example](#lte-to-lte-ml-handover-example)
  * [LTE to LTE RSRP Handover LM Example](#lte-to-lte-rsrp-handover-lm-example)

# Project Overview
This project has been developed by the National Institute of Standards and Technology (NIST)
Communications Technology Lab (CTL) Wireless Networks Division (WND).

This project includes open source, third party dependencies. For details of the licenses of these
dependencies see [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md)

Certain equipment, instruments, software, or materials, commercial or non-commercial, are used in this project. Such
usage is not intended to imply recommendation or endorsement of any product or service by NIST, nor is it intended to
imply that the software, materials, or equipment identified are necessarily the best available for the purpose.

This project is considered feature complete, and will be maintained on a 'best effort' basis.

## Contact
To report a bug, please open a [GitHub Issue](https://github.com/usnistgov/ns3-oran/issues/new).
The point of contact for this project is Evan Black ([evan.black@nist.gov](mailto:evan.black@nist.gov))


# Model Description
The `oran` module for `ns-3` implements the classes required to model a
network architecture based on the O-RAN specifications. These models include
a RAN Intelligent Controller (RIC) that is functionally equivalent to
O-RAN's Near-Real Time (Near-RT) RIC, and reporting modules that attach to
simulation nodes and serve as communication endpoints with the RIC in a
similar fashion as the E2 Terminators in O-RAN.

These models have been designed to provide the infrastructure and access to
data so that developers and researchers can focus on implementing their
solutions, and minimize the time and effort spent on handling interactions
between models. With this in mind, all the components that contain logic that
may be modified by end users have been modeled hierarchically (so that parent
classes can take care of common actions and methods and leave child models to
focus on the logic itself), and at least one example is provided, to serve as
reference for new models.

The RIC model uses a data repository to store all the information exchanged
between the RIC and the modules, as well as to serve as a logging endpoint.
This release provides an [SQLite](https://www.sqlite.org) storage backend for
the data repository. The database file is accessible after the simulation and
can be accessed by any SQLite-compatible tool and interface.

Modeling of the reporting and communication models for the simulation nodes
has been implemented using existing traces and methods, which means there is
no need to modify the models provided by the `ns-3` distribution to make use
of the full capabilities of this module.

# Features
This release of the `oran` module contains the following features:
- Near-RT RIC model, including:
  - Data access API independent of the data repository backend
  - SQLite database repository implementation for Reports, Commands, and
    logging
  - Support for Logic Modules that serve as O-RAN's `xApps`
  - Separation of Logic Modules into `default` (only one, mandatory) and
    `additional` (zero to many, optional)
  - Support for addition and removal of Logic Modules during the simulation
  - Conflict Mitigation API
  - Logic Module and Conflict Mitigation logic logging to the data repository
  - Periodic invocation of the Logic Module's algorithms
- Periodic reporting of metrics from the simulation nodes to the Near-RT RIC
- Reporting capabilities for node location (any simulation node) and LTE cell
  attachment (LTE UE nodes)
- Generation and execution of LTE-to-LTE handover Commands
- Activation and deactivation for individual components and RIC
- Periodic node registration and de-registration with the RIC
- Integration with [ONNX Runtime](https://onnxruntime.ai/) and
  [PyTorch](https://pytorch.org/) to support Machine Learning (ML)

# Minimum Requirements
* ns-3.42
* SQLite 3.7.17

## Optional Dependencies
* ONNX Runtime 1.14.1
* PyTorch 2.2.2

# Installation
## Clone (Recommended)
Clone the project into a directory called `oran` in the `contrib` directory
of a supported version of `ns-3`.

1) `cd` into the `contrib` directory of `ns-3`

```shell
cd contrib/
```

2) Clone the project from one of the below URLs

```shell
# Pick one of the below
# HTTPS (Choose this one if you're uncertain)
git clone https://github.com/usnistgov/ns3-oran.git oran

# SSH
git clone git@github.com:usnistgov/ns3-oran.git oran
```

3) (Re)configure and (Re)build `ns-3`

```shell
# --enable-examples is optional, see `Running the Examples`
# for how to run them
./ns3 configure --enable-examples
./ns3
```

## Download ZIP
If, for whatever reason, `git` is not available, download the
project and unzip it into the `contrib` directory of `ns-3`.

Note: Updates will have to be performed manually using this method.

1) Download the ZIP of the project from the url below

```shell
wget https://github.com/usnistgov/ns3-oran/archive/refs/heads/master.zip
```

2) Unzip the file into the `ns-3` `contrib/` directory

```shell
unzip ns3-oran-master.zip
```

3) Rename the resulting directory to `oran`, as `ns-3` will not accept a
module named differently than its directory

```shell
mv ns3-oran-master oran
```

## Connecting the Module Quickly
If you are linking your module/program to the `oran` module add the following
to your `CMakeLists.txt`(CMake).

### CMake (Quick Connect)
For CMake, add `oran`'s target `${liboran}` to your `libraries_to_link` list.

```cmake
# Example
build_lib_example(
  NAME your-example-name
  SOURCE_FILES example.cc
  LIBRARIES_TO_LINK
    ${liboran}
    # ...
)

# Module
build_lib(
  LIBNAME your-module-name
  SOURCE_FILES
    # ...
  HEADER_FILES
    # ...
  LIBRARIES_TO_LINK
    ${liboran}
    # ...
)
```

## Connecting the Module Safely
You may wish for your module to not have a hard dependency on the `oran`
module. The following steps will link the `oran` module, but still allow you
to build and run your module without the `oran` module present.

### CMake (Safe Connect)
Check for `oran` in the `ns3-all-enabled-modules` list to confirm that the
module is present.

Note: If the module that links to the `oran` module is in the `src/`
directory, then you will need to add the `ENABLE_ORAN` C++ define yourself when
you check for the presence of the module.

```cmake
# Create a list of your required modules to link
# 'core' and 'mobility' are just examples here
  set(libraries_to_link "${libcore};${libmobility}")

# Check if the `oran` module is in the enabled modules list
if("oran" IN_LIST ns3-all-enabled-modules)
  # If it is there, then it is safe to add it to the library list
  list(APPEND libraries_to_link ${liboran})

  # N.B. if the module you are linking to is in the `src/` directory
  # of ns-3, then (at least for now), you must also add the C++ define
  # yourself, like this.
  #
  # There is no harm in repeated definitions of the same value, so there is no
  # need to guard this statement
  add_definitions(-DENABLE_ORAN)
endif()

# Use the `libraries_to_link` list as your dependency list

# Module
build_lib(
  LIBNAME your-module
  SOURCE_FILES
    # ...
  HEADER_FILES
    # ...
  LIBRARIES_TO_LINK
    ${libraries_to_link}
)

# Example
build_lib_example(
  NAME your-scenario
  SOURCE_FILES scenario.cc
  LIBRARIES_TO_LINK
    ${libraries_to_link}
)
```

### Code (Safe Connect)
In addition to the variable in the build environment, the module also defines
a C++ macro also named `ENABLE_ORAN`. This macro may be used in C++ code to
check for the presence of the `oran` module.

Note: If you are using CMake and the module is in the `src/` directory, you
may have to add this definition yourself (`scratch/` and module examples are
fine). See
[the CMake build system section for more information](#build-system).

```cpp
// Guard the include with the macro
#ifdef ENABLE_ORAN
#include <ns3/oran-module.h>
#endif

// ...

int main ()
{
  // ...

  // Guard any oran references in code with the macro as well
#ifdef ENABLE_ORAN
  auto oranHelper = CreateObject<OranHelper> ();
  // ...
#endif
}
```

# Updating
## Clone
To update the cloned module, move to the module's root directory and perform
a `git pull`.

```shell
# From the ns-3 root
cd contrib/oran
git pull
```

## ZIP
To update a ZIP installation, remove the old module and replace it with the
updated one.

```shell
# From the ns-3 root
cd contrib
rm -Rf oran

# Use this command, or download manually
wget https://github.com/usnistgov/ns3-oran/archive/refs/heads/master.zip \
  -O ns3-oran-master.zip
unzip ns3-oran-master.zip

# Make sure the directory in the ns-3 contrib/ directory is named `oran`
mv ns3-oran-master oran
```
# ML Support
ML is not required to use this module, however, we provide a means of
integration for both ONNX and PyTorch so that it is possible to use ML to
make inferences when running simulations. Therefore, users who wish to
simulate O-RAN based solutions that leverage ML may do so using this module.
It does however require the extra step of making at least one of these
libraries accessible to our module so that we can link and compile against it.
This also means that while these tools may be installed and accessible system
wide, this is not a requirement as it is possible to have a locally compiled
version of the library and source files in a user's working space. Please note
that when using these tools with our module, the expectation is that the user
will have already trained and created an ML model outside of the O-RAN
simulations, but once this is done, that model may be used via the ONNX or
PyTorch C++ API from the simulation. We provide a class for each tool to
demonstrate the use of both ONNX and PyTorch, respectively:
- OranLmLte2LteOnnxHandover
- OranLmLte2LteTorchHandover

OranLmLte2LteOnnxHandover is defined by the files:
- 'model/oran-lm-lte-2-lte-onnx-handover.h'
- 'model/oran-lm-lte-2-lte-onnx-handover.cc'

OranLmLte2LteTorchHandover is defined by the files:
- 'model/oran-lm-lte-2-lte-torch-handover.h'
- 'model/oran-lm-lte-2-lte-torch-handover.cc'

There is also the
[LTE to LTE ML Handover Example](#lte-to-lte-ml-handover-example)
that we will discuss later that demonstrates the use
of these two classes using existing ML models included with this module.

## ONNX
At the time that this documentation was created, not very many linux
distributions provide ONNX packages. However, the use of ONNX for this module
is attractive since models that are created using other tools, such as
PyTorch, can be exported to ONNX. Therfore, integration with ONNX was desired
with the hopes that it can be used to provide the flexibility needed to
support more than one type of ML model without having to integrate each
individually. To make use of ONNX with this module, one simply needs to
download the ONNX libraries that are distributed on the ONNX webiste
([https://onnxruntime.ai/](https://onnxruntime.ai/)), and export the location
of the extracted library to the `LIBONNXPATH` environment variable. For
example,

```shell
# Download the library files
wget "https://github.com/microsoft/onnxruntime/releases/download/v1.14.1/onnxruntime-linux-x64-1.14.1.tgz"

# Extract the library files
tar xzf onnxruntime-linux-x64-1.14.1.tgz

# Create environment variable with library location so that cmake knows where
# to find it
export LIBONNXPATH="$(pwd)/onnxruntime-linux-x64-1.14.1"

```

At this point, the user should be able to navigate to their working directory
of `ns-3` and run

```shell
./ns3 configure
```

The output of this command should include the text "find_external_library:
OnnxRuntime was found," indicating that the library and necessary source
files were discovered.

## PyTorch
PyTorch is widely available through most linux distribution package bases.
Therefore, a user should simply be able to install the desired
"python-pytorch" package with no further steps being required. However, if
the user cannot or does not wish to install the package, one simply can
download the PyTorch libraries that are distrubited on the PyTorch webiste
([https://pytorch.org/](https://pytorch.org/)), and export the location of the
extracted library to the `LIBTORCHPATH` environment variable. For example,

```shell
# Download the library files
wget "https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-2.2.2%2Bcpu.zip"

# Extract the library files
unzip libtorch-cxx11-abi-shared-with-deps-2.2.2+cpu.zip

# Create environment variable with library location so that cmake knows where
# to find it
export LIBTORCHPATH="$(pwd)/libtorch"

```
At this point, the accessibility of the library can be verified by navigating
to the working directory of `ns-3` and running the following command.

```shell
./ns3 configure
```

The output of this command should include the text "find_external_library:
Torch was found," indicating that the library and necessary source
files were discovered.

# Documentation
[Sphinx](https://www.sphinx-doc.org/en/master/) is required to build the
documentation.

To run Sphinx to build the documentation, cd into the `doc` directory in the
module and run `make [type]` for the type of documentation you wish to build.

```shell

# From the ns-3 root directory
cd contrib/oran/doc

# HTML (Several Pages)
make html

# HTML (One Page)
make singlehtml

# PDF
make latexpdf

# To list other options, just run make
make
```

The built documentation can now be found in `doc/build/[type]`.

# Running the Examples
Listed below are the commands to run the examples provided with the module.

## Random Walk Example
Example with a very simple topology in which `ns-3` nodes move randomly and
periodically report their position to the Near-RT RIC.

```shell
./ns3 run "oran-random-walk-example --verbose"
```

## LTE to LTE Distance Handover Example
A complex scenario that shows how to configure and deploy each of the models
manually, without using the `oran`'s helper. This scenario consists of 2 LTE
eNBs and 1 LTE UE that moves back and forth between both eNBs. The UE is
initially attached to the closest eNB, but as it moves closer to the other
eNB, the Logic Module in the RIC will issue a handover Command and the UE
will be handed over to the other eNB.

```shell
./ns3 run "oran-lte-2-lte-distance-handover-example"
```

## LTE to LTE Distance Handover With Helper Example
Functionally the same scenario as the
[LTE to LTE Distance Handover Example](#lte-to-lte-distance-handover-example),
however, in this scenario the helper is used to configure and deploy the
models.

```shell
./ns3 run "oran-lte-2-lte-distance-handover-helper-example"
```

## LTE to LTE Distance Handover With LM Processing Delay Example
Similar to the
[LTE to LTE Distance Handover With Helper Example](#lte-to-lte-distance-handover-with-helper-example),
however, in this scenario the Logic Module is configured with a processing
delay.

```shell
./ns3 run "oran-lte-2-lte-distance-handover-lm-processing-delay-example"
```

## LTE to LTE Distance Handover With LM Query Trigger Example
Similar to the
[LTE to LTE Distance Handover With Helper Example](#lte-to-lte-distance-handover-with-helper-example)
example, however, in this scenario the Near-RT RIC is configured with a
custom Query Trigger (provided in the scenario) that may initiate the Logic
Module querying process as soon as Reports with certain characteristics are
received by the Near-RT RIC.

```shell
./ns3 run "oran-lte-2-lte-distance-handover-lm-query-trigger-example"
```

## Keep-Alive Example
This example showcases how the keep-alive mechanism works. A single node
moving in a straight line periodically reports its position to the Near-RT
RIC. However, not all of these Reports will be accepted by the Near-RT RIC
because the configuration of the timing for sending Registration messages in
the node means that the node will frequently be marked as ’inactive’ by the
Near-RT RIC.

```shell
NS_LOG="OranE2NodeTerminator=prefix_time|warn" ./ns3 run "oran-keep-alive-example"
```

## Data Repository Example
This example showcases how the Data Repository API can be used to store and
retrieve information. This example performs all these operations from the
scenario for simplicity, but the same methods can be used by any model in the
simulation that can access the RIC. This will be important when developing
custom Logic Modules, and can also be used in scenarios for debugging,
testing and validation.

```shell
./ns3 run "oran-data-repository-example"
```

## Multiple Network Devices Example
Similar to the
[LTE to LTE Distance Handover Wth Helper Example](#lte-to-lte-distance-handover-with-helper-example)
this example showcases how a node with several network devices can interact
with the Near-RT RIC separately.

```shell
./ns3 run "oran-multiple-net-devices-example"
```

## LTE to LTE ML Handover Example
Note that in order to run this example using the flag, `--use-onnx-lm`, the
ONNX libraries must be found during the configuration of `ns-3`, and it is
assumed that the ML model file "saved_trained_model_pytorch.onnx" has been
copied from the example directory to the working directory. In order to run
this example using the flag, `--use-torch-lm`, the PyTorch libraries must be
found during the configuration of `ns-3`, and it is assumed that the ML model
file "saved_trained_model_pytorch.pt" has been copied from the example
directory to the working directory.

This example showcases how pretrained ONNX and PyTorch ML Models can be used
to initiate handovers based on location and packet loss data. It consists of
four UEs and two eNBs, where UE 1 and UE 4 are configured to move only within
coverage of eNB 1 or eNB 2, respectively, while UE 2 and UE 3 move around in
an area where the coverage of eNB 1 and eNB 2 overlap. As the simulation
progresses with UEs moving and receiving data, the distances of all four UEs
as well as the recorded packet loss for each UE are fed to an ML model that
returns a desired configuration that indicates which eNB UE 2 and UE 3 should
be attached to minimize the overall packet loss. The models that we provide
are for demonstration purposes only and have not been thoroughly developed. It
should also be noted that "saved_trained_model_pytorch.onnx" is the same
trained model as "saved_trained_model_pytorch.pt" only it  has been exported
to the ONNX format.

```shell
./ns3 run "oran-lte-2-lte-ml-handover-example"
```

Without going into too many details, there is also a script included in the
examples folder called,
"oran-lte-2-lte-ml-handover-example-generate-training-data.sh," that can be
used as a start to generate data using this same example to train an ML model.
Furthermore, once the training data has been generated, the file
"oran-lte-2-lte-ml-handover-example-classifier.py" that is also included in
the example folder, can be used to produce a PyTorch ML model using the
training data that is generated.

## LTE to LTE RSRP Handover LM Example
In this scenario the Near-RT RIC is configured with an LM that uses RSRP
measurements that are reported by the UE to trigger handovers.

```shell
./ns3 run "oran-lte-2-lte-rsrp-handover-lm-example"
```
