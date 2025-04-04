#!/bin/bash
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
# --------------------------------------------------------------------------

# This script can be used to generate the input data needed to train a
# Machine Learning (ML) classifier. This script first runs the simulations
# for a particular scenario that consists of four UEs and two eNodeBs. Each
# simulation corresponds to a particular "configuration" that serves as a
# category to indicate which UEs should be connected to which eNodeBs. In
# this scenario there are four possible combinations (or configurations).
# Once the simulations for all four configurations are finished executing the
# data needed from the database (DB) is compiled and processed. This data is
# processed in such a way so that at any given time we can determine which
# configuration produces the least amount of packet loss at the application
# layer. Once this process is completed the input data that can be used to
# train the ML model is output. This consists of 13 columns that correspond
# to the distance to eNodeB 1, the distance to eNodeB 2, and the packet loss
# that was experienced in the past second for each UE. In addition to those 12
# columns, the last column indicates to the ML model wich configuration
# provides the least amount of packet loss for all four UEs. Using these
# inputs to train the ML model, the model should be able to correlate UE
# distances from eNodeBs and UE packet losses with the configuration that
# minimizes the total packet loss. Therefore, once the ML model is trained,
# the 12 columns that indicate UE distances from eNodeBs and UE packet loss
# can be input to the ML model, and then the ML model can output a weight for
# each of the four  possible configurations that indicate which configuration
# will provide the least amount of packet loss in the next second.

# SQL statement to get the necessary statistics from the DB for all four UEs
# that are considered in the scenario.
sqlite="sqlite3"
sql="SELECT
  ue1loss.simulationtime,
  ue1loss.nodeid, SQRT(POWER(ue1loc.x - 0, 2) + POWER(ue1loc.y - 0, 2)), SQRT(POWER(ue1loc.x - 265, 2) + POWER(ue1loc.y - 0, 2)), ue1loss.loss,
  ue2loss.nodeid, SQRT(POWER(ue2loc.x - 0, 2) + POWER(ue2loc.y - 0, 2)), SQRT(POWER(ue2loc.x - 265, 2) + POWER(ue2loc.y - 0, 2)), ue2loss.loss,
  ue3loss.nodeid, SQRT(POWER(ue3loc.x - 0, 2) + POWER(ue3loc.y - 0, 2)), SQRT(POWER(ue3loc.x - 265, 2) + POWER(ue3loc.y - 0, 2)), ue3loss.loss,
  ue4loss.nodeid, SQRT(POWER(ue4loc.x - 0, 2) + POWER(ue4loc.y - 0, 2)), SQRT(POWER(ue4loc.x - 265, 2) + POWER(ue4loc.y - 0, 2)), ue4loss.loss,
  (ue1loss.loss + ue2loss.loss + ue3loss.loss + ue4loss.loss) / 4.0
FROM
    nodeapploss AS ue1loss
      INNER JOIN nodelocation AS ue1loc ON ue1loc.nodeid = ue1loss.nodeid AND ue1loss.simulationtime = ue1loc.simulationtime
    INNER JOIN nodeapploss AS ue2loss ON ue2loss.nodeid = 2 AND ue2loss.simulationtime = ue1loss.simulationtime
      INNER JOIN nodelocation AS ue2loc ON ue2loc.nodeid = 2 AND ue2loc.simulationtime = ue1loc.simulationtime
    INNER JOIN nodeapploss AS ue3loss ON ue3loss.nodeid = 3 AND ue3loss.simulationtime = ue1loss.simulationtime
      INNER JOIN nodelocation AS ue3loc ON ue3loc.nodeid = 3 AND ue3loc.simulationtime = ue1loc.simulationtime
    INNER JOIN nodeapploss AS ue4loss ON ue4loss.nodeid = 4 AND ue4loss.simulationtime = ue1loss.simulationtime
      INNER JOIN nodelocation AS ue4loc ON ue4loc.nodeid = 4 AND ue4loc.simulationtime = ue1loc.simulationtime
WHERE
  ue1loss.nodeid = 1
ORDER BY
  ue1loss.simulationtime ASC;"

# Run simulations for all four possible configurations to generate the data
# that we need to process
for ((i=0;i<4;i++))
do
  ./ns3 run "oran-lte-2-lte-ml-handover-example
    --use-oran=true
    --use-distance-lm=false
    --use-onnx-lm=false
    --use-torch-lm=false
    --start-config=${i}
    --db-file=oran-repository-config${i}.db
    --traffic-trace-file=traffic-trace-config${i}.tr
    --position-trace-file=position-trace-config${i}.tr
    --handover-trace-file=handover-trace-config${i}.tr
    --sim-time=10"
done

# Compile the results so that we can process them. This includes retrieving
# the data and associating it with the configuration that it belongs to.
for ((i=0;i<4;i++))
do
  echo ${sql} | ${sqlite} oran-repository-config${i}.db | sed 's/|/ /g' | awk -v config=${i} '{print $0, config}' > config${i}.csv
done

# Process the results and output the training data. First, sort the data
# based on the simulation time because the UE locations, i.e. distances
# should be the same, however, the packet loss may be different. Then sort
# the data based on the total loss that was computed so that at any given
# time the configurations are ordered so that the congfiguration that offers
# the least amount of loss is first. Then, default to configuration 1 when it
# along with any other congiruation provides the least amount of loss.
# Lastly, associate the 12 input columns with the correct configuration as
# the 13th column and output this as it is the training data.
cat config0.csv config1.csv config2.csv config3.csv | sort -n -t ' ' -k1,1 -k18,18 -k19,19 | awk '
BEGIN {
  min = 4;
  config = 1;
} {
  if (NR > 4 && NR % 4 == 1) {
    print l1, config;
    print l2, config;
    print l3, config;
    print l4, config;
    min = 4;
    config = 1;
  }

  if ($18 < min || ($18 == min && $19 == 1)) {
    min = $18;
    config = $19;
  }

  if (NR % 4 == 1) {
    l1 = $3" "$4" "$5" "$7" "$8" "$9" "$11" "$12" "$13" "$15" "$16" "$17;
  } else if (NR % 4 == 2) {
    l2 = $3" "$4" "$5" "$7" "$8" "$9" "$11" "$12" "$13" "$15" "$16" "$17;
  } else if (NR % 4 == 3) {
    l3 = $3" "$4" "$5" "$7" "$8" "$9" "$11" "$12" "$13" "$15" "$16" "$17;
  } else if (NR % 4 == 0) {
    l4 = $3" "$4" "$5" "$7" "$8" "$9" "$11" "$12" "$13" "$15" "$16" "$17;
  }
} END {
  print l1, config;
  print l2, config;
  print l3, config;
  print l4, config;
}
' | awk '
{
  if (NR > 4) {
    if (NR % 4 == 1) {
      print l1, $13;
      print l2, $13;
      print l3, $13;
      print l4, $13;
    }
  }

  if (NR % 4 == 1) {
    l1 = $1" "$2" "$3" "$4" "$5" "$6" "$7" "$8" "$9" "$10" "$11" "$12;
  } else if (NR % 4 == 2) {
    l2 = $1" "$2" "$3" "$4" "$5" "$6" "$7" "$8" "$9" "$10" "$11" "$12;
  } else if (NR % 4 == 3) {
    l3 = $1" "$2" "$3" "$4" "$5" "$6" "$7" "$8" "$9" "$10" "$11" "$12;
  } else if (NR % 4 == 0) {
    l4 = $1" "$2" "$3" "$4" "$5" "$6" "$7" "$8" "$9" "$10" "$11" "$12;
  }
}
'

