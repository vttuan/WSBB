===========================
This is an implementation of workstealing for Branch and Bound taking into account
the heterogeneous of computing platform (multicores, multi cpus and multi gpus). The
scheduling flowshop Taillard is chosen to as an example.

For more infomation about the flowshop Taillard, please refer to:

http://mistic.heig-vd.ch/taillard/problemes.dir/ordonnancement.dir/flowshop.dir/best_lb_up.txt

=========================== 

Protocol Deployment - there are two steps:

i) Manager: (./src/work_stealing/manager.cpp) ebstablish a termination detection tree
for work stealing.

./manager -n [#compute nodes] -c [tree degree]

ii) Peer: (./src/main.ccp) computing instance

./peer -i [instance number] -u [#cpu cores] -g [#devices] -p [#port number] -c [manager host]

Example:

./manager -n 128 -c 10

==> build a tree with 128 nodes and each node has 10 children at max

./peer -i 21 -u 8 -g 2 -p 10000 -c 192.168.1.1

==> launch an instance with 8 cpu cores, 2 gpu devices to solve the flowshop Ta21. The program
will run at port 10000 and connect to the manager running at machine 192.168.1.1
  
