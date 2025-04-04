.. include:: replace.txt
.. highlight:: cpp

O-RAN Model Description
-----------------------

The ``oran`` module for ns-3 implements the classes required to model a network architecture based on the `O-RAN`_ specifications. These models include a RAN Intelligent Controller (RIC) that is functionally equivalent to O-RAN's Near-Real Time (Near-RT) RIC, and reporting modules that attach to simulation nodes and serve as communication endpoints with the RIC in a similar fashion as the E2 Terminators in O-RAN.

These models have been designed to provide the infrastructure and access to data so that developers and researchers can focus on implementing their solutions, and minimize the time and effort spent on handling interactions between models. With this in mind, all the components that contain logic that may be modified by end users have been modeled hierarchically (so that parent classes can take care of common actions and methods, and leave child models to focus on the logic itself), and at least one example is provided, to serve as reference for new models.

The RIC model uses a data repository to store all the information exchanged between the RIC and the modules, as well as to serve as a logging endpoint. This release provides an `SQLite`_ storage backend for the data repository. The database file is accessible after the simulation and can be accessed by any SQLite-compatible tool and interface.

Modeling of the reporting and communication models for the simulation nodes has been implemented using existing traces and methods, which means there is no need to modify the models provided by the ns-3 distribution to make use of the full capabilities of this module.



Features
********

This release of the ``oran`` module contains the following features:

- Near-RT RIC model, including:

  - Data access API independent of the data repository backend.
  - SQLite database repository implementation for Reports, Commands, and logging.
  - Support for Logic Modules that serve as O-RAN's ``xApps``.
  - Separation of Logic Modules into ``default`` (only one, mandatory) and ``additional`` (zero to many, optional).
  - Support for addition and removal of Logic Modules during the simulation.
  - Conflict Mitigation API.
  - Logic Module and Conflict Mitigation logic logging to the data repository.
  - Periodic or report-triggered invocation of the Logic Module's algorithms.
  - Simulated processing delay for each Logic Module.
  - Integration with `ONNX Runtime`_ and `PyTorch`_ to support Machine Learning (ML)

- Periodic reporting of metrics from the simulation nodes to the Near-RT RIC.
- Periodic or event-triggered generation of reports.
- Reporting capabilities for node location (any simulation node) and LTE cell attachment (LTE UE nodes).
- Generation and execution of LTE-to-LTE handover Commands.
- Activation and deactivation for individual components and RIC.
- Simulated transmission delay between the E2 Nodes and the Near-RT RIC.
- E2 Node Keep-Alive mechanism.


.. _O-RAN: https://www.o-ran.org/
.. _Sqlite: https://www.sqlite.org
.. _ONNX Runtime: https://onnxruntime.ai/
.. _PyTorch: https://pytorch.org/
