.. include:: replace.txt
.. highlight:: cpp

O-RAN Usage
-----------

In this Chapter we will review how to use the provided O-RAN models in an scenario, as well as how to implement our own models. The working examples provided with the release demonstrate all the required operations and configurations needed to use the models.


Deployment in Scenario
**********************

The operations required to use the O-RAN models in a simulation, are:

- Configuration and instantiation of the Near-RT RIC and all its modules, and
- Configuration and instantiation of the Node E2 Terminators and Reporters.


Instantiating and configuring the Near-RT RIC to the point where it is ready to be used in the simulation requires the previous instantiation of all the modules that make up the RIC. First, we need to instantiate a Data Repository. This can be done by simply creating an object of the appropriate class, and configuring its attributes. The next listing shows how this is done for the SQLite-backed Data Repository, which needs to know the name of the file we will use for the database, and optionally, the trace sink for the status report of all the SQL queries run against the database (method ``QueryRcSink`` in the listing)::

  void
  QueryRcSink (std::string query, std::string args, int rc)
  {
    std::cout << Simulator::Now ().GetSeconds () <<
      " Query " << ((rc == SQLITE_OK || rc == SQLITE_DONE) ? "OK" : "ERROR") <<
      "(" << rc << "): \"" << query << "\"";

    if (! args.empty ())
      {
        std::cout << " (" << args << ")";
      }
    std::cout << std::endl;
  }
  // ....
  Ptr<OranDataRepository> dataRepository = CreateObject<OranDataRepositorySqlite> ();
  dataRepository->SetAttribute ("DatabaseFile", StringValue (dbFileName));
  dataRepository->TraceConnectWithoutContext ("QueryRc", MakeCallback (&QueryRcSink));


Once the Data Repository has been configured, the next step is to instantiate and configure the Logic Modules. We need a pointer to the Near-RT RIC to complete the configuration of these modules, so we will create the pointer to the RIC without any further configuration, and then we will focus on the LMs, as shown in the next listing. All the LMs, regardless of their type, must be provided with a  pointer to the Near-RT RIC, a Random Variable that will provide the processing time (in seconds) when the LM is queried, and optionally (but recommended), a name. This name will be used in the logging messages. Finally, LMs can be configured to be verbose, which makes the LMs log information to the Data Repository each time it is run::

  Ptr<OranNearRtRic> nearRtRic = CreateObject<OranNearRtRic> ();
  // ....
  Ptr<OranLm> myLm = CreateObject<OranLmNoop> ();
  myLm->SetName ("MyNoopLm");
  myLm->SetAttribute ("NearRtRic", PointerValue (nearRtRic));
  myLm->SetAttribute ("Verbose", BooleanValue (true));
  myLm->SetAttribute ("ProcessingDelayRv",
      StringValue ("ns3::ConstantRandomVariable[Constant=0.005]"));

The same process and attributes to instantiate and configure the LMs applies to the Conflict Mitigation Modules::

  Ptr<OranNearRtRic> nearRtRic = CreateObject<OranNearRtRic> ();
  // ....
  Ptr<OranCmm> cmm = CreateObject<OranCmmSingleCommandPerNode> ();
  cmm->SetName ("MyConflictMitigationModule");
  cmm->SetAttribute ("NearRtRic", PointerValue (nearRtRic));
  cmm->SetAttribute ("Verbose", BooleanValue (true));

The last module we need to instantiate is the E2 Terminator for the RIC. The configuration of this module is very simple, as it just needs to be provided pointers to the Near-RT RIC, the Data Repository, and the Random Variable that will simulate the transmission delay (in seconds) for the issued Commands::

  Ptr<OranNearRtRic> nearRtRic = CreateObject<OranNearRtRic> ();
  Ptr<OranDataRepository> dataRepository = CreateObject<OranDataRepositorySqlite> ();
  // ....
  Ptr<OranNearRtRicE2Terminator> nearRtRicE2Terminator = CreateObject<OranNearRtRicE2Terminator> ();
  nearRtRicE2Terminator->SetAttribute ("NearRtRic", PointerValue (nearRtRic));
  nearRtRicE2Terminator->SetAttribute ("DataRepository", PointerValue (dataRepository));
  nearRtRicE2Terminator->SetAttribute ("TransmissionDelayRv",
      StringValue ("ns3::ConstantRandomVariable[Constant=0.001]"));

All of these modules can also be activated and deactivated individually, but in most cases we will want to activate and deactivate the whole RIC at once, which can be done once we configure the object created earlier.

The next block shows the code necessary to configure the Near-RT RIC. Lines 1 to 8 serve as a reminder that we need to have all the modules instantiated as previously shown. After that, lines 10 to 16 show how to link these models to the RIC. Special attention should be given to lines 10 and 11, where we can see that the Default LM is set using an attribute (as this LM is mandatory), but the additional LMs are added to the RIC using the method ``AddLogicModule``. Lines 18 to 20 configure the maximum time for an E2 Node to refresh its registration, and the Random Variable that will define how often the Near-RT RIC will check for nodes that have not registered recently. Lines 22 and 23 configure the maximum time the Near-RT RIC will wait for the LMs to run their logic, and the policy that will define what to do with the reports generated by LMs that take longer than the maximum time (in this case, the policy is 'DROP', so they will be discarded). Line 25 adds a Query Trigger to the Near-RT RIC.

Finally, line 27 shows the scheduling of the activation and start of operations 1 second into the simulation. At this time, when the ``Start`` method is invoked in the RIC, all the modules linked to the RIC will also be activated, and the timer to periodically run the LMs will also start::

  Ptr<OranNearRtRic> nearRtRic = CreateObject<OranNearRtRic> ();
  // ...
  Ptr<OranDataRepository> dataRepository = CreateObject<OranDataRepositorySqlite> ();
  Ptr<OranLm> defaultLm = CreateObject<OranNoop> ();
  Ptr<OranLm> otherLm = CreateObject<OranLmLte2LteDistanceHandover> ();
  Ptr<OranCmm> cmm = CreateObject<OranCmmNoop> ();
  Ptr<OranNearRtRicE2Terminator> nearRtRicE2Terminator = CreateObject<OranNearRtRicE2Terminator> ();
  Ptr<OranQueryTrigger> myQueryTrigger = CreateObject<OranQueryTriggerNoop> ();
  // ...
  nearRtRic->SetAttribute ("DefaultLogicModule", PointerValue (defaultLm));
  nearRtRic->AddLogicModule (otherLm);
  nearRtRic->SetAttribute ("LmQueryInterval", TimeValue (Seconds (5)));

  nearRtRic->SetAttribute ("E2Terminator", PointerValue (nearRtRicE2Terminator));
  nearRtRic->SetAttribute ("DataRepository", PointerValue (dataRepository));
  nearRtRic->SetAttribute ("ConflictMitigationModule", PointerValue (cmm));

  nearRtRic->SetAttribute ("E2NodeInactivityThreshold", TimeValue (Seconds (2)));
  nearRtRic->SetAttribute ("E2NodeInactivityIntervalRv",
      StringValue ("ns3::ConstantRandomVariable[Constant=2]"));

  nearRtRic->SetAttribute ("LmQueryMaxWaitTime", TimeValue (Seconds (0.5)));
  nearRtRic->SetAttribute ("LmQueryLateCommandPolicy", EnumValue (OranNearRtRic::DROP));

  nearRtRic->AddQueryTrigger ("MyNoopQueryTrigger", myQueryTrigger);

  Simulator::Schedule (Seconds (1), &OranNearRtRic::Start, nearRtRic);

Once we have finished configuring the RIC, we can start deploying E2 Terminators in the simulation nodes. The next listing shows that the Node E2 Terminators themselves need to be provided a pointer to the Near-RT RIC (note that the Near-RT RIC must be instantiated before configuring the Node E2 Terminator; however, the listing does not include the code for instantiating the Near-RT RIC for clarity purposes; previous listings demonstrate how to instantiate all the required models), as well as the random variables that will be used for triggering periodic registration events,  periodic transmission of Reports to the Near-RT RIC (lines 6 to 8), and the delay for the transmission of said Reports (line 9). Once these attributes have been configured, and the Reporters in this node created, these Reporters must be added to the E2 Terminator using the ``AddReporter`` method, as shown on line 11. Additionally we need to attach the Terminator to the simulation node, so IDs can be retrieved for registration purposes, and the listing shows how to do this on line 12. Finally, we need to schedule a time for the activation of the Terminator and all the attached Reporters, as is done on line 14 of the listing::

  Ptr<Node> myWiredNode = CreateObject<Node> ();
  Ptr<OranNearRtRic> nearRtRic = CreateObject<OranNearRtRic> ();
  Ptr<OranE2NodeTerminatorWired> wiredNodeTerminator = CreateObject<OranE2NodeTerminatorWired> ();
  Ptr<OranReporterLocation> locationReporter = CreateObject<OranReporterLocation> ();
  // ....
  wiredNodeTerminator->SetAttribute ("NearRtRic", PointerValue (nearRtRic));
  wiredNodeTerminator->SetAttribute ("RegistrationIntervalRv", StringValue ("ns3::ConstantRandomVariable[Constant=5]"));
  wiredNodeTerminator->SetAttribute ("SendIntervalRv", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  wiredNodeTerminator->SetAttribute ("TransmissionDelayRv", StringValue ("ns3::ConstantRandomVariable[Constant=0.001]");

  wiredNodeTerminator->AddReporter (locationReporter);
  wiredNodeTerminator->Attach (myWiredNode);

  Simulator::Schedule (Seconds (2), &OranE2NodeTerminatorWired::Activate, wiredNodeTerminator);


After the Node E2 Terminator has been configured, the next step is to instantiate and configure any Reporter that will be operating from that node. The next listing shows the instantiation and configuration of an LTE UE Cell Information Reporter. All Reporters need to be provided a pointer to the Node E2 Terminator that they will be reporting to (line 4) and a Report Trigger that will indicate when to generate these reports (lines 5 and 6). As mentioned earlier, once the Reporter is fully configured, we need to add it to the Node E2 Terminator (line 8)::

  Ptr<OranE2NodeTerminatorLteUe> lteUeTerminator = CreateObject<OranE2NodeTerminatorLteUe> ();
  // ....
  Ptr<OranReporterLocation> lteCellInfoReporter = CreateObject<OranReporterLteUeCellInfo> ();
  lteCellInfoReporter->SetAttribute ("Terminator", PointerValue (lteUeTerminator));
  lteCellInfoReporter->SetAttribute ("Trigger",
      StringValue ("ns3::OranReportTriggerLteUeHandover[InitialReport=true]"));

  lteUeTerminator->AddReporter (lteCellInfoReporter);


Helper
======

The previous listings show the code that is necessary to include in a scenario in order to use the O-RAN models. However, it is possible to use the helper provided with the distribution to simplify and reduce the code.

The next listing shows all the code necessary to instantiate and configure the Near-RT RIC using the ``OranHelper``. This helper provides methods for each component, so that a single method can be used to specify the type of the component instance, along with any attributes that we want to configure, as shown on lines 11 (Data Repository), 12 (Default Logic Module), 13 (Additional Logic Module), 14 (Conflict Mitigation Module), and 15 (a Query Trigger). Near-RT RIC attributes can be configured through attributes of the helper, as on lines 4 to 9. When all the modules have been configured, the method ``CreateNearRtRic`` will instantiate all the modules, associate them with the Near-RT RIC, and return a pointer to the Near-RT RIC instance (line 17). The helper can also be used to activate and deactivate the RIC with the methods ``ActivateAndStartNearRtRic`` and ``DeactivateAndStopNearRtRic``::

  Ptr<OranNearRtRic> nearRtRic = nullptr;
  Ptr<OranHelper> oranHelper = CreateObject<OranHelper> ();

  oranHelper->SetAttribute ("Verbose", BooleanValue (false));
  oranHelper->SetAttribute ("LmQueryInterval", TimeValue (Seconds (1)));
  oranHelper->SetAttribute ("E2NodeInactivityThreshold", TimeValue (Seconds (2)));
  oranHelper->SetAttribute ("E2NodeInactivityIntervalRv", StringValue ("ns3::ConstantRandomVariable[Constant=2]"));
  oranHelper->SetAttribute ("LmQueryMaxWaitTime", TimeValue (Seconds (0.5)));
  oranHelper->SetAttribute ("LmQueryLateCommandPolicy", EnumValue (OranNearRtRic::DROP));

  oranHelper->SetDataRepository ("ns3::OranDataRepositorySqlite", "DatabaseFile", StringValue (dbFileName));
  oranHelper->SetDefaultLogicModule ("ns3::OranLmNoop", "Name", StringValue ("defaultLm"));
  oranHelper->AddLogicModule ("ns3::OranLmLte2LteDistanceHandover", "Verbose", BooleanValue (true), "Name", StringValue ("MyAdditionalLm"));
  oranHelper->SetConflictMitigationModule ("ns3::OranCmmSingleCommandPerNode", "Verbose", BooleanValue (true));
  oranHelper->AddQueryTrigger ("Crossover", "ns3::OranQueryTriggerCustom", "CustomCallback", CallbackValue (MakeCallback(&CustomLmQueryTriggerCallback)));

  nearRtRic = oranHelper->CreateNearRtRic ();

The configuration and instantiation of the nodes using the helper must be done in groups of nodes that will share the same type of Node E2 Terminator and Reporters. The next listing shows the configuration of Terminators for a group of LTE UE nodes (lines 11 through 20) and a group of wired nodes (lines 23 through 30). Each block of code has the same structure: First, the E2 Node Terminator is configured by defining its type and any attribute that we may need (lines 11-13 and 23-25). Then the method ``AddReporter`` is called as many times as different types of Reporters will be deployed on these nodes, specifying the type of Reporter and the attributes needed to configure it (lines 15-18 configure two Reporters for the LTE UEs, and lines 27-28 configure one Reporter for the wired nodes). Finally, the method ``DeployTerminators`` is invoked with the pointer to the Near-RT RIC and the NodeContainer with the nodes where the Terminators and Reporters will be deployed (lines 20 and 30), and an ``OranE2NodeTerminatorContainer`` that has all the Node E2 Terminators is returned::


  NodeContainer ueNodes, wiredNodes;

  Ptr<OranNearRtRic> nearRtRic = nullptr;
  OranE2NodeTerminatorContainer e2NodeTermsUes, e2NodeTermsWired;
  Ptr<OranHelper> oranHelper = CreateObject<OranHelper> ();
  oranHelper->SetAttribute ("Verbose", BooleanValue (true));
  // ....
  nearRtRic = oranHelper->CreateNearRtRic ();
  // ....
  // LTE UEs get Location Reporter and Cell ID Reporter
  oranHelper->SetE2NodeTerminator ("ns3::OranE2NodeTerminatorLteUe",
      "RegistrationIntervalRv", StringValue ("ns3::ConstantRandomVariable[Constant=5]"),
      "SendIntervalRv", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));

  oranHelper->AddReporter ("ns3::OranReporterLocation",
      "Trigger", StringValue ("ns3::OranReportTriggerPeriodic"));
  oranHelper->AddReporter ("ns3::OranReporterLteUeCellInfo",
      "Trigger", StringValue ("ns3::OranReportTriggerLteUeHandover[InitialReport=true]"));

  e2NodeTermsUes.Add (oranHelper->DeployTerminators (nearRtRic, ueNodes));

  // Wired nodes get only Location Reporter
  oranHelper->SetE2NodeTerminator ("ns3::OranE2NodeTerminatorWired",
      "RegistrationIntervalRv", StringValue ("ns3::ConstantRandomVariable[Constant=5]"),
      "SendIntervalRv", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));

  oranHelper->AddReporter ("ns3::OranReporterLocation",
      "Trigger", StringValue ("ns3::OranReportTriggerPeriodic"));

  e2NodeTermsWired.Add (oranHelper->DeployTerminators (nearRtRic, wiredNodes));


Modeling New Implementations
****************************

In this Section we will present shells of custom modules (Logic Module, Query Trigger, Conflict Mitigation Module, Reporter, Report Trigger, and Node E2 Terminator) that show that custom modules only need to focus on the logic they want to implement, as most of the overhead work is being taken care of by the parent classes. These shell classes can also be used as the starting point for the development of new models.


Logic Module
============

The next listing shows the shell of a custom Logic Module. Besides the constructor (line 24), destructor (line 32), and ``GetTypeId`` methods (line 13), we see that a custom LM only needs to implement the public method ``Run``, that returns a vector of Reports. In this ``Run`` method the basic workflow is described in the comments on lines 48-52, and it consists of retrieving information from the Data Storage (using the pointer to the Near-RT RIC to get access to the instance), run the algorithm that we want this LM to implement, and generate the Commands that are deemed necessary. These Commands are stored in a vector and this vector is returned::

  #include <ns3/log.h>
  #include <ns3/abort.h>
  #include <ns3/simulator.h>

  #include "oran-data-repository.h"

  namespace ns3 {

  NS_LOG_COMPONENT_DEFINE ("MyLm");

  NS_OBJECT_ENSURE_REGISTERED (MyLm);

  TypeId
  MyLm::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::MyLm")
      .SetParent<OranLm> ()
      .AddConstructor<MyLm> ()
    ;

    return tid;
  }

  MyLm::MyLm (void)
    : OranLm ()
  {
    NS_LOG_FUNCTION (this);

    m_name = "MyLm";
  }

  MyLm::~MyLm (void)
  {
    NS_LOG_FUNCTION (this);
  }

  std::vector<Ptr<OranCommand> >
  MyLm::Run (void)
  {
    NS_LOG_FUNCTION (this);

    std::vector<Ptr<OranCommand> > commands;

    if (m_active)
      {
        NS_ABORT_MSG_IF (m_nearRtRic == nullptr, "Attempting to run LM (" + m_name + ") with NULL Near-RT RIC");

        // Step 1: Retrieve relevant data from the Data Repository

        // Step 2: Run our algorithm

        // Step 3: Create the needed Commands and put them in the 'commands' vector
      }

    // Return the commands.
    return commands;
  }

  } // namespace ns3



Query Trigger
=============

The next listing shows the shell of a custom Query Trigger. The core of a Query Trigger is the method ``QueryLms``, that receives a Report and based on the type and contents of the report, returns a boolean value that, if true, will initiate the querying of the LMs::

  #include <ns3/log.h>
  #include <ns3/abort.h>

  #include "oran-report.h"

  namespace ns3 {

  NS_LOG_COMPONENT_DEFINE ("MyQueryTrigger");

  NS_OBJECT_ENSURE_REGISTERED (MyQueryTrigger);

  TypeId
  MyQueryTrigger::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::MyQueryTrigger")
      .SetParent<OranQueryTrigger> ()
      .AddConstructor<MyQueryTrigger> ()
    ;

    return tid;
  }

  MyQueryTrigger::MyQueryTrigger (void)
    : OranReporter ()
  {
    NS_LOG_FUNCTION (this);
  }

  MyQueryTrigger::~MyQueryTrigger (void)
  {
    NS_LOG_FUNCTION (this);
  }

  bool
  MyQueryTrigger::QueryLms (Ptr<OranReport> report)
  {
    NS_LOG_FUNCTION (this);

    bool queryLms = false;

    // Check type and contents of report, and, if needed, change the
    // value of queryLms

    return queryLms;
  }

  } // namespace ns3



Conflict Mitigation
===================


Similar to the Logic Modules, a custom Conflict Mitigation Module (like the one shown in the next listing) only needs to implement the constructor (line 24), destructor (line 32), Dispose (line 38), and ``GetTypeId`` methods (line 13), and the method that houses the logic specific to this CMM: the ``Filter`` method (line 45)::

  #include <ns3/log.h>
  #include <ns3/abort.h>

  #include "oran-command.h"
  #include "oran-near-rt-ric.h"

  namespace ns3 {

  NS_LOG_COMPONENT_DEFINE ("MyCmm");

  NS_OBJECT_ENSURE_REGISTERED (MyCmm);

  TypeId
  MyCmm::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::MyCmm")
      .SetParent<OranCmm> ()
      .AddConstructor<MyCmm> ()
    ;

    return tid;
  }

  MyCmm::MyCmm (void)
    : OranCmm ()
  {
    NS_LOG_FUNCTION (this);

    m_name = "My Custom CMM";
  }

  MyCmm::~MyCmm (void)
  {
    NS_LOG_FUNCTION (this);
  }

  void
  MyCmm::DoDispose (void)
  {
    NS_LOG_FUNCTION (this);

    OranCmm::DoDispose ();
  }

  std::vector<Ptr<OranCommand> >
  MyCmm::Filter (
    std::map<std::tuple<std::string, bool>,
    std::vector<Ptr<OranCommand> > > inputCommands)
  {
    NS_LOG_FUNCTION (this);

    NS_ABORT_MSG_IF (m_nearRtRic == nullptr, "Attempting to run Conflict Mitigation Module with NULL Near-RT RIC");

    std::vector<Ptr<OranCommand> > commands;

    if (m_active)
      {
        // TODO: Decide which commands we really want to transmit, and add them to the 'commands' vector
      }
    else
      {
        // This module is not active. Just copy the same set of commands as output
        for (auto commandSet : inputCommands)
          {
            commands.insert (commands.end (), commandSet.second.begin (), commandSet.second.end ());
          }
      }

    return commands;
  }

  } // namespace ns3


The ``Filter`` method receives a map in which the key is a tuple with the name of the LM that generated the associated Commands, and a boolean flag indicating if this is the Default LM of the RIC. The value associated with each tuple is a vector of all the Commands generated by that LM.

With all the Commands sorted this way, the CMM can implement the desired logic to filter them, as described in the comment on line 58. The Commands that pass this filter shall be stored in a vector, which will be returned by the method.



Node Reporter
=============

The next listing presents the shell for custom Reporters. Once again, we need to implement the constructor (line 23), destructor (line 29), and ``GetTypeId`` methods (line 12). For Reporters, the method that implements the specific data capture for this instance is the ``GenerateReports`` method (line 34). This method is expected to return a vector of Reports. The comments on lines 44-48 describe the usual workflow for generating these Reports, namely capture the information from the node, create Reports with the appropriate values filled in, and store these Reports in the vector that will be returned::

  #include <ns3/log.h>
  #include <ns3/abort.h>

  #include "oran-report.h"

  namespace ns3 {

  NS_LOG_COMPONENT_DEFINE ("MyReporter");

  NS_OBJECT_ENSURE_REGISTERED (MyReporter);

  TypeId
  MyReporter::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::MyReporter")
      .SetParent<OranReporter> ()
      .AddConstructor<MyReporter> ()
    ;

    return tid;
  }

  MyReporter::MyReporter (void)
    : OranReporter ()
  {
    NS_LOG_FUNCTION (this);
  }

  MyReporter::~MyReporter (void)
  {
    NS_LOG_FUNCTION (this);
  }

  std::vector<Ptr<OranReport> >
  MyReporter::GenerateReports (void)
  {
    NS_LOG_FUNCTION (this);

    std::vector<Ptr<OranReport> > reports;
    if (m_active)
      {
        NS_ABORT_MSG_IF (m_terminator == nullptr, "Attempting to generate reports in reporter with NULL E2 Terminator");

        // Step 1: Capture the information needed from the node methods or traces

        // Step 2: Instantiate and populate the values of a Report of the appropriate type

        // Step 3: Add the report to the 'reports' vector
      }
    return reports;
  }

  } // namespace ns3


Report Trigger
==============

The next listing shows a shell code for custom Report Triggers. The main component of a Report Trigger is a method that will monitor a trace, and based on that trace, it will invoke the method ``TriggerReports``. This method is shown in the listing on lines 74 to 81. In order for this method to work, we use the ``Activate`` method to connect the trace of interest to our method (lines 34 to 46), and the ``Deactivate`` method to disconnect the trace (lines 48 to 59, and 83 to 90)::

  #include <ns3/log.h>
  #include <ns3/simulator.h>

  #include "oran-reporter.h"

  namespace ns3 {

  NS_LOG_COMPONENT_DEFINE ("MyReportTrigger");

  NS_OBJECT_ENSURE_REGISTERED (MyReportTrigger);

  TypeId
  MyReportTrigger::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::MyReportTrigger")
      .SetParent<OranReportTrigger> ()
      .AddConstructor<MyReportTrigger> ()
    ;

    return tid;
  }

  MyReportTrigger::MyReportTrigger (void)
    : OranReportTrigger ()
  {
    NS_LOG_FUNCTION (this);
  }

  MyReportTrigger::~MyReportTrigger (void)
  {
    NS_LOG_FUNCTION (this);
  }

  void
  MyReportTrigger::Activate (Ptr<OranReporter> reporter)
  {
    NS_LOG_FUNCTION (this << reporter);

    if (! m_active)
      {
        traceofinterest->TraceConnectWithoutContext ("TraceName",
          MakeCallback (&MyReportTrigger::CheckTriggeredTrace, this));
      }

    OranReportTrigger::Activate (reporter);
  }

  void
  MyReportTrigger::Deactivate (void)
  {
    NS_LOG_FUNCTION (this);

    if (m_active)
      {
        DisconnectSink ();
      }

    OranReportTrigger::Deactivate ();
  }

  void
  MyReportTrigger::DoDispose (void)
  {
    NS_LOG_FUNCTION (this);

    if (m_active)
      {
        DisconnectTrace ();
      }

    OranReportTrigger::DoDispose ();
  }

  void
  MyReportTrigger::CheckTriggeredTrace (...params...)
  {
    NS_LOG_FUNCTION (this);

    NS_LOG_LOGIC ("Triggering report");
    TriggerReport ();
  }

  void
  MyReportTrigger::DisconnectTrace (void)
  {
    NS_LOG_FUNCTION (this);

    traceofinterest->TraceDisconnectWithoutContext ("TraceName",
      MakeCallback (&MyReportTrigger::CheckTriggeredTrace, this));
  }

  } // namespace ns3



Node E2 Terminator
==================

Finally, the next listing shows how, similarly to the other custom modules, custom Node E2 Terminators need to implement the constructor (line 20), destructor (line 26), and ``GetTypeId`` methods (line 9). Additionally, the ``GetNodeType`` method must be implemented to indicate what type of node this E2 Node Terminator can be attached to::

   #include <ns3/log.h>

   namespace ns3 {

   NS_LOG_COMPONENT_DEFINE ("MyE2NodeTerminator");

   NS_OBJECT_ENSURE_REGISTERED (MyE2NodeTerminator);

   TypeId
   MyE2NodeTerminator::GetTypeId (void)
   {
     static TypeId tid = TypeId ("ns3::MyE2NodeTerminator")
       .SetParent<OranE2NodeTerminator> ()
       .AddConstructor<MyE2NodeTerminator> ()
     ;

     return tid;
   }

   MyE2NodeTerminator::MyE2NodeTerminator (void)
     : OranE2NodeTerminator ()
   {
     NS_LOG_FUNCTION (this);
   }

   MyE2NodeTerminator::~MyE2NodeTerminator (void)
   {
     NS_LOG_FUNCTION (this);
   }

   OranNearRtRic::NodeType
   MyE2NodeTerminator::GetNodeType (void) const
   {
     NS_LOG_FUNCTION (this);

     return OranNearRtRic::WIRED;
   }

   void
   MyE2NodeTerminator::ReceiveCommand (Ptr<OranCommand> command)
   {
     NS_LOG_FUNCTION (this << command);

     if (m_active)
       {
         // TODO: Filter the commands we can process using the TypeId
         // TODO: Add processing of the receive commands.
       }
   }

   } // namespace ns3


The differentiating aspect between Node E2 Terminators is the type of Commands they can process. This specificity is implemented in the ``ReceiveCommand`` method (line 40). This method, when the Terminator is active, checks that it knows how to process the Command (by checking the ``TypeId`` of the Command), and then proceeds to interpret and act as directed by the Command.

