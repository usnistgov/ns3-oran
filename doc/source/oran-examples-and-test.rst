.. include:: replace.txt
.. highlight:: cpp

O-RAN Examples and Tests
------------------------

This Section presents the examples distributed with the release, and gives a quick overview of the scenario in each one.



Random Walk Example
*******************

The first and simplest example is the Random Walk Example, distributed in the example file ``oran-random-walk-example.cc``. This example presents a very simple topology in which ns-3 nodes move randomly and periodically reports its position to the Near-RT RIC. This scenario shows how the RIC can be configured to collect and gather information from the simulation, without any logic processing or Command issuing taking place.

This example provides two methods for optionally printing the position of a node to the terminal when its direction is altered (method ``CourseChange``), and the database queries used to store and access data in the RIC along with their result (method ``QueryRcSink``). These two methods can be enabled or disabled with the command line switch ``--verbose``.

The example uses the ``OranHelper`` to reduce the amount of code needed to configure and deploy working models of the full O-RAN architecture. This helper is instantiated on line 107, and starting at line 116 the RIC is being configured: First a data repository that uses the SQLite backend, and then an instance of the 'No Operation' Logic Module to be used as the default LM (line 118), and an instance of the 'No Operation' Conflict Mitigation Module (line 119). This configuration ensures that no actual processing will take place, and no Commands will be issued.

The Node E2 Terminator is configured starting at line 124, with a Location Reporter using a periodic Report Trigger attached to it (line 128). These models are instantiated and deployed on line 131.

Finally, lines 140 and 141 activate and start the operation of the RIC and the Node E2 Terminator.



LTE to LTE Distance Handover Example
************************************

The LTE to LTE Distance Handover Example, distributed in the example file ``oran-lte-2-lte-distance-handover-example.cc``, is a more complex scenario that shows how to configure and deploy each of the models manually, without using the ``OranHelper``. This scenario consists of 2 LTE eNBs and 1 LTE UE that moves back and forth between both eNBs. The UE is initially attached to the closest eNB, but as it moves closer to the other eNB, the Logic Module in the RIC will issue a handover Command and the UE will be attached to the other eNB.

This example provides two methods for optionally printing a message to the terminal when a handover has been successfully completed (method ``NotifyHandoverEndOkEnb``), and the database queries used to store and access data in the RIC along with their result (method ``QueryRcSink``). These two methods can be enabled or disabled with the command line switch ``--verbose``. Additionally, the method ``ReverseVelocity`` is invoked periodically to reverse the direction in which the UE is moving.

The configuration of the O-RAN models begins at line 178. Firstly, the components of the RIC are instantiated and configured:

- Data Repository, instantiated on line 182 and configured on lines 188 through 192.
- Default Logic Module, instantiated as ``OranLmLte2LteDistanceHandover`` on line 183 and configured on lines 194 to 196.
- Conflict Mitigation Module, instantiated as an ``OranCmmNoop`` on line 184 and configured on lines 198 and 199.
- E2 Terminator, instantiated on line 186 and configured on lines 201 through 203.


After all these modules have been configured, the Near-RT RIC is configured with references to them (shown on lines 205 through 213), and the activation and start of operations is scheduled for RIC to happen one second into the simulation, on line 200.

The next step is to configure the Terminators and Reporters for the nodes. The loop on lines 217 to 240 configures the LTE UE nodes in the simulation with an LTE UE E2 Node Terminator (instantiated on line 221, and configured on lines 229 to 232), a Location Reporter with a periodic trigger (instantiated on line 219, configured on lines 223 and 224), and a Cell Attachment Reporter with a handover event trigger (instantiated on line 220, configured on lines 226 and 227). The Reporters are added to the Terminator on lines 234 and 235, and the Terminator is attached to the node on line 237. Finally, the Terminator is scheduled to start working on line 239.

Similarly the loop on lines 242 to 260 configures the LTE eNB nodes almost the same, with the only differences being that the Terminator instance used is the LTE eNB E2 Node Terminator, and that only the Location Reporter is configured for these nodes.



LTE to LTE Handover With Helper Example
***************************************

The LTE to LTE Distance Handover Helper Example, distributed in the example file ``oran-lte-2-lte-distance-handover-helper-example.cc``, is functionally the same scenario as the previous example. However, in this scenario the ``OranHelper`` is used to configure and deploy the models.

The Helper is instantiated on line 181. After that, the Helper is configured with several attributes that will be passed to the models as they are instantiated (lines 183 to 189), the model names and parameters of the RIC modules that we want to use (lines 197 to 201), and the RIC is instantiated (line 203).

The instantiation and configuration of the models for the LTE UE nodes is achieved with lines 206 to 217, and the LTE eNB nodes are setup on lines 220 to 228.

Finally, the activation and start of all the models is scheduled on lines 237 to 239.

This example is used as the base for others, that will show additional functionality with the same topology.



LTE to LTE Handover With LM Processing Delay Example
****************************************************

The LTE to LTE Distance Handover With LM Processing Delay Example, distributed in the example file ``oran-lte-2-lte-distance-handover-lm-processing-delay-example.cc``, is functionally the same scenario as the one in the previous example. However, in this scenario the LM is configured with a processing delay.

The processing delay is defined with a Random Variable. By default the scenario uses a Normal Random Variable with a mean of 5 ms and a variance of 0.031 ms, but this can be overriden using the ``processing-delay-rv`` command line parameter.

The relevant lines for configuring the processing delay for LMs are:

- Lines 208 and 209, where LMs are created with the ``ProcesingDelayRv`` attribute set to some non-default value.
- Line 197, where the maximum time to wait for LMs while processing is configured  (this value will be passed to the Near-RT RIC).
- Line 198, where the Near-RT RIC policy for handling LMs that do not complete the calculations in time is defined.



LTE to LTE Handover With LM Query Trigger Example
*************************************************

The LTE to LTE Distance Handover With LM Query Trigger Example, distributed in the example file ``oran-lte-2-lte-distance-handover-lm-query-trigger-example.cc``, is functionally the same scenario as the one in the previous example. However, in this scenario the Near-RT RIC is configured with a custom Query Trigger (provided in the scenario) that may initiate the LM querying process as soon as Reports with certain characteristics are received by the Near-RT RIC.

In this scenario the function that will be used to analyze the Reports received by the Near-RT RIC and decide whether to start the LM Querying process is defined on lines 93 to 127 (function ``CustomLmQueryTriggerCallback``. This function:

- Receives a Report.
- Runs its logic, which in this case is:
   - Check if the Report is a Location Report. If so,
   - Check if the neighbor LTE eNB is closer. If so,
   - Prepare to return ``true``.
- Return ``true`` if the LM querying process should be started right away.

Once the function is defined, the only other step required for deploying the custom Query Trigger is to add it to the Helper before instantiating the Near-RT RIC, as done on lines 234 and 235. When adding this Query Trigger to the Helper we specify a name, the Query Trigger class that will be used (``OranQueryTriggerCustom`` in this case), and the function to use for evaluation, specified as a Callback.



Keep-Alive Example
******************

The Keep-Alive Example, distributed in the example file ``oran-keep-alive-example.cc``, showcases how the Keep-Alive mechanism works. This example uses a single node moving in a straight line and periodically reporting its position to the Near-RT RIC. However, not all of these Reports will be accepted by the Near-RT RIC, because the configuration of the timing for sending Registration messages in the node means that the node will frequently be marked as 'inactive' by the Near-RT RIC.

The configuration parameters relevant in this example are:

- In the Near-RT RIC:
   - The maximum time since a previous registration before marking an E2 Node as 'Deregistered'. Configured on line 83 to be 1 second.
   - The Random Variable for the inteval between periodic checks of E2 Nodes to see if any of them should be marked as 'Deregistered'. Configured on line 84 to be a constant interval of 0.5 seconds.
- In the E2 Node Terminator:
   - The Random Variable for the inteval between sending periodic Registration messages to the Near-RT RIC. Configured on line 101 to be a uniform interval between 2 and 3 seconds.

Note that the values used for the Random Variables and timings have been chosen to showcase the workings of the Keep-Alive mechanism and the effects of E2 Nodes being deregisted from the Near-RT RIC. These values are not recommended for use in actual scenarios.

Running this scenario with WARNING logs enabled for the E2 Node Terminator like this:

``NS_LOG="OranE2NodeTerminator=prefix_time|warn" ./waf --run oran-keep-alive-example``

will show how many of the Registration messages were sent too late.



Data Repository Example
***********************

The Data Repository Example, distributed in the example file ``oran-data-repository-example.cc``, showcases how the Data Repository API can be used to store and retrieve information. This example performs all these operations from the scenario for simplicity, but the same methods can be used by any model in the simulation that can access the RIC. This will be important when developing custom Logic Modules, and can also be used in scenarios for debugging, testing and validation.



LTE to LTE ML Handover Example
************************************

The LTE to LTE ML Handover Example, distributed in the example file ``oran-lte-2-lte-ml-handover-example.cc``, is a scenario that showcases how pretrained ONNX and PyTorch ML Models can be used to initiate handovers based on location and packet loss data. It consists of four UEs and two eNBs, where UE 1 and UE 4 are configured to move only within coverage of eNB 1 or eNB 2, respectively, while UE 2 and UE 3 move around in an area where the coverage of eNB 1 and eNB 2 overlap. As the simulation progresses with UEs moving and receiving data, the distances of all four UEs as well as the recorded packet loss for each UE are fed to an ML model that returns a desired configuration that indicates which eNB UE 2 and UE 3 should be attached to to minimize the overall packet loss. The models that we provide are for demonstrate purposes only and have not been thoroughly developed. It should also be noted that "saved_trained_model_pytorch.onnx" is the same trained model as "saved_trained_model_pytorch.pt" only it has been exported to the ONNX format.

Note that in order to run this example using the flag, ``--use-onnx-lm``, the ONNX libraires must be found during the configuration of ns-3, and it is assumed that the ML model file ``saved_trained_model_pytorch.onnx`` has been copied from the example directory to the working directory. In order to run this example using the flag, ``--use-torch-lm`` the PyTorch libraires must be found during the configuration of ns-3, and it is assumed that the ML model file ``saved_trained_model_pytorch.pt`` has been copied from the example directory to the working directory.

The configuration of the O-RAN models begins at line 305. The ``OranLmLte2LteOnnxHandover`` LM is instantiated and configured on lines 320 to 324, while the ``OranLmLte2LteTorchHandover`` LM is instantiated and configured on lines 325 to 329. These LMs use a pretrained ML model that takes UE distance and application loss as an input and then outputs a desired configuration that the LM can then use to determine if any handovers need to take place.

There is a script included in the examples folder called, ``oran-lte-2-lte-ml-handover-example-generate-training-data.sh`` that can be used to generate data using this same example to create and train an ML model. This script essentially runs a simulation for each possible combination of UE-to-cell configuration, and then parses the data to determine which confiugration provides the lowest average packet loss. Using this information, the script then provides traning data that can be used by the PyTorch classifier defined in ``oran-lte-2-lte-ml-handover-example-classifier.py``, which takes the outputs from the script as input. This input essentially tells the ML model which configuration will provide the lowest average packet loss for the next one second, given the inputs we described earlier. After runing the script to generate the training data and feeding that training data to the python classifier, a new ``saved_trained_model_pytorch.pt`` should exist that can now be used by the ``OranLmLte2LteTorchHandover`` LM.


Tests
*****

The Test Suite provided with the models includes a single test (``OranTestCaseMobility1``). This test creates a single node which starts to move two seconds into the simulation and stops moving 12 seconds into the simulation. During all this time the node moves with a constant velocity of (2, 2, 0). The scenario uses the ``OranHelper`` to deploy a RIC with 'No Operation' LMs and CMM, so no attempts to modify the topology is made by the RIC, and a Location Reporter is attached to the node.

After 14 seconds of simulation the Data Storage in the RIC is queried to retrieve the first and last positions reported by the node, and they are compared with the pre-computed values to verify their correctness.


