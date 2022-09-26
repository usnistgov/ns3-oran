# ORAN ns-3 Module
A module that can be used in [ns-3](https://www.nsnam.org/) to simulate and 
model behavior based on the [O-RAN](https://www.o-ran.org) speciications.

# Table of Contents
* [Model Description](#model-description)
* [Features](#features)
* [Requirements](#requirements)
* [Installation](#installation)
  * [Clone (Recommended)](#clone-recommended)
  * [Download ZIP](#download-zip)
  * [Connecting the Module Quickly](#connecting-the-module-quickly)
    * [Cmake](#cmake-quick-connect)
    * [Waf](#waf-quick-connect)
    * [Code](#code-quick-connect)
  * [Connecting the Module Safely](#connecting-the-module-safely)
    * [Cmake](#cmake-safe-connect)
    * [Waf](#waf-safe-connect)
    * [Code](#code-safe-connect)
* [Updating](#updating)
  * [Clone](#clone)
  * [ZIP](#zip)
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

# Requirements
* A C++ compliant compiler
    * GCC 7.3.0 or higher
    * Clang 6.0.0 or higher
* SQLite3

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
Note: If you are using a version of `ns-3` without CMake, replace `./ns3 ` 
with `./waf`.

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
to your `CMakeLists.txt`(CMake) or `wscript`(Waf).

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

### Waf (Quick Connect)
For waf, add `oran`'s module name `oran` to the dependency list.

```python
# Program
obj = bld.create_ns3_program('program-name', ['oran', '''...'''])


# Module
module = bld.create_ns3_module('module-name', ['oran', '''...'''])
```

### Code (Quick Connect)
You may now include and use the `oran` module in your code.

```cpp
#include <ns3/oran-module.h>
//...

int main ()
{
  auto oranHelper = CreateObject<OranHelper> ();
  // ...
}
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

### Waf (Safe Connect)
If you wish for your module/program to be able to build without the `oran` 
module you may check for its existence by reading `bld.env['ENABLE_ORAN']` in 
your `wscript`.

```python
def build(bld):
  # Create a list of your required modules to link
  # 'core' and 'mobility' are just examples here
  linked_modules = ['core', 'mobility']

  # Check if 'ENABLE_ORAN' was defined during configuration
  if 'ENABLE_ORAN' in bld.env:
    # If it was defined, then the 'oran' module is present and we may link it
    linked_modules.append('oran')

  # Be sure to pass your list of `linked_modules` to `create_ns3_program`
  # or `create_ns3_module`
  obj = bld.create_ns3_program('application-name', linked_modules)
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

Note: If you are using a version of `ns-3` without CMake, replace 
`./ns3 run ` with `./waf --run`.

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
[LTE to LTE Distance Handover Wth Helper Example](#lte-to-lte-distance-handover-with-helper-example), 
however, in this scenario the Logic Module is configured with a processing 
delay.

```shell
./ns3 run "oran-lte-2-lte-distance-handover-lm-processing-delay-example"
```

## LTE to LTE Distance Handover With LM Query Trigger Example
Similar to the 
[LTE to LTE Distance Handover Wth Helper Example](#lte-to-lte-distance-handover-with-helper-example) 
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

