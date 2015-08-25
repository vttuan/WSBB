===========================

This is an implementation of workstealing for Branch and Bound while taking
into account the heterogeneous of computing platform (multicores, multi cpus
and multi gpus). The scheduling flowshop Taillard is chosen to be a challenge
for this implmentation. Due to high irregularlity produced by branch and bound
mechanism while solving  the flowshop instances, reaching high performance is
a challenge for heterogeneous computing platform. For more infomation about
the flowshop Taillard, please refer to:

http://mistic.heig-vd.ch/taillard/problemes.dir/ordonnancement.dir/flowshop.dir/best_lb_up.txt

=========================== 

Protocol deployment of the implementation - there are two steps:

i) Manager: (./src/work_stealing/manager.cpp) ebstablish a termination detection tree
for work stealing.

./manager -n [#compute nodes] -c [tree degree]

ii) Peer: (./src/main.ccp) computing instance

./peer -i [instance number] -u [#cpu cores] -g [#devices] -p [#port number] -c [manager host]

Example:

./manager -n 128 -c 10

==> build a tree with 128 nodes and each node has 10 children at max. When receiving
the last join message from the 128th node, the manager will build a termination
detection tree and send this information to the corresponding peers. Moreover,
the manager also broadcasts a message to all peers in order to let them start the
computation.

./peer -i 21 -u 8 -g 2 -p 10000 -c 192.168.1.1

==> launch an instance with 8 cpu cores, 2 gpu devices to solve the flowshop Ta21.
The program will run at port 10000 and connect to the manager running at machine
192.168.1.1. This command should be launched at each machine. 
