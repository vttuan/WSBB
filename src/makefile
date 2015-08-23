
CUDA_INSTALL_PATH ?= /usr/local/cuda

ROOTDIR    := ${HOME}/NVIDIA_GPU_Computing_SDK/C
COMMONDIR  := $(ROOTDIR)/common
SHAREDDIR  := ${HOME}/NVIDIA_GPU_Computing_SDK/shared
LIBDIR     := $(ROOTDIR)/lib

PG = -g
#PG = -pg
OPT = -O3 -Wall -Wno-deprecated -Wno-non-virtual-dtor $(PG) 
#OPT = -Wall -Wno-deprecated -Wno-non-virtual-dtor 
BIN = ../bin/
SRC = ../src/
MTH = ../int/
BRN = ../src/
HDRS = ../headers/

# Libs
ifneq ($(DARWIN),)
    LIB       := -L$(CUDA_INSTALL_PATH)/lib -L$(LIBDIR) -L$(COMMONDIR)/lib/$(OSLOWER) -L$(SHAREDDIR)/lib $(NVCUVIDLIB) 
else
  ifeq "$(strip $(HP_64))" ""
    ifeq ($(x86_64),1)
       LIB       := -L$(CUDA_INSTALL_PATH)/lib64 -L$(LIBDIR) -L$(COMMONDIR)/lib/$(OSLOWER) -L$(SHAREDDIR)/lib 
    else
       LIB       := -L$(CUDA_INSTALL_PATH)/lib -L$(LIBDIR) -L$(COMMONDIR)/lib/$(OSLOWER) -L$(SHAREDDIR)/lib
    endif
  else
    ifeq ($(i386),1)
       LIB       := -L$(CUDA_INSTALL_PATH)/lib -L$(LIBDIR) -L$(COMMONDIR)/lib/$(OSLOWER) -L$(SHAREDDIR)/lib
    else
       LIB       := -L$(CUDA_INSTALL_PATH)/lib64 -L$(LIBDIR) -L$(COMMONDIR)/lib/$(OSLOWER) -L$(SHAREDDIR)/lib
    endif
  endif
endif

# If dynamically linking to CUDA and CUDART, we exclude the libraries from the LIB
ifeq ($(USECUDADYNLIB),1)
     LIB += ${OPENGLLIB} $(PARAMGLLIB) $(RENDERCHECKGLLIB) $(CUDPPLIB) ${LIB} -ldl -rdynamic 
else
# static linking, we will statically link against CUDA and CUDART
  ifeq ($(USEDRVAPI),1)
     LIB += -lcuda   ${OPENGLLIB} $(PARAMGLLIB) $(RENDERCHECKGLLIB) $(CUDPPLIB) ${LIB} 
  else
     ifeq ($(emu),1) 
         LIB += -lcudartemu
     else 
         LIB += -lcudart
     endif
     LIB += ${OPENGLLIB} $(PARAMGLLIB) $(RENDERCHECKGLLIB) $(CUDPPLIB) ${LIB}
  endif
endif

ifeq ($(USECUFFT),1)
  ifeq ($(emu),1)
    LIB += -lcufftemu
  else
    LIB += -lcufft
  endif
endif

ifeq ($(USECUBLAS),1)
  ifeq ($(emu),1)
    LIB += -lcublasemu
  else
    LIB += -lcublas
  endif
endif

ifeq ($(USECURAND),1)
    LIB += -lcurand
endif

ifeq ($(USECUSPARSE),1)
  LIB += -lcusparse
endif

NVCC = /usr/local/cuda-5.0/bin/nvcc --ptxas-options=-v -gencode=arch=compute_10,code=\"sm_10,compute_10\" -Xptxas -dlcm=ca

INCLUDES  += -I. -I$(CUDA_INSTALL_PATH)/include -I$(COMMONDIR)/inc -I$(SHAREDDIR)/inc

$(BIN)peer : $(BIN)main.o $(BIN)peer.o $(BIN)problem.o $(BIN)problems.o $(BIN)solutions.o $(BIN)eliminate_pareto.o $(BIN)work.o $(BIN)works.o $(BIN)ttime.o $(BIN)weights.o $(BIN)tree.o $(BIN)explorer.o $(BIN)BigInteger.o $(BIN)BigIntegerUtils.o $(BIN)BigUnsigned.o $(BIN)BigUnsignedInABase.o $(BIN)bound_flowshop.o $(BIN)decompose_beginend.o $(BIN)gpu.o $(BIN)instance_flowshop.o $(BIN)select_worst.o $(BIN)pbab.o $(BIN)bound_abstract.o $(BIN)arguments.o $(BIN)communication.o $(BIN)node.o $(BIN)packets.o $(BIN)protocol.o $(BIN)statistics.o $(BIN)bb_problem_calculator.o
	$(NVCC) -g -o $(BIN)peer $(BIN)main.o $(BIN)peer.o $(BIN)problem.o $(BIN)problems.o $(BIN)solutions.o $(BIN)eliminate_pareto.o $(BIN)work.o $(BIN)works.o $(BIN)ttime.o $(BIN)weights.o $(BIN)tree.o $(BIN)explorer.o $(BIN)BigInteger.o $(BIN)BigIntegerUtils.o $(BIN)BigUnsigned.o $(BIN)BigUnsignedInABase.o $(BIN)bound_flowshop.o $(BIN)decompose_beginend.o $(BIN)gpu.o $(BIN)instance_flowshop.o $(BIN)select_worst.o $(BIN)pbab.o $(BIN)bound_abstract.o $(BIN)arguments.o $(BIN)communication.o $(BIN)node.o $(BIN)packets.o $(BIN)protocol.o $(BIN)statistics.o $(BIN)bb_problem_calculator.o $(INCLUDES) $(LIB) -lpthread $(PG)
	g++ $(SRC)rand.cpp  -o $(BIN)rand
	g++ -pthread $(SRC)work_stealing/manager_fixed_random_work_stealing.cpp -o  $(SRC)work_stealing/manager_fixed_random_work_stealing
	g++ -pthread $(SRC)work_stealing/manager_gpu_cpu_cluster.cpp -o  $(SRC)work_stealing/manager_gpu_cpu_cluster
	
$(BIN)main.o : $(SRC)main.cpp 
	$(NVCC) -g -o $(BIN)main.o -c $(SRC)main.cpp $(INCLUDES) $(LIB)
	
$(BIN)bound_flowshop_gpu.o : $(SRC)bound_flowshop_gpu.cu 
	$(NVCC) -g -o $(BIN)bound_flowshop_gpu.o -c $(SRC)bound_flowshop_gpu.cu $(INCLUDES) $(LIB) $(PG)
	
$(BIN)gpu.o : $(SRC)gpu.cu 
	$(NVCC) -g -o $(BIN)gpu.o -c $(SRC)gpu.cu $(INCLUDES) $(LIB) $(PG)

$(BIN)instance_flowshop.o : $(SRC)instance_flowshop.cpp
	g++ -o $(BIN)instance_flowshop.o -c $(SRC)instance_flowshop.cpp $(OPT)

$(BIN)decompose_beginend.o : $(SRC)decompose_beginend.cpp
	g++ -o $(BIN)decompose_beginend.o -c $(SRC)decompose_beginend.cpp $(INCLUDES) $(LIB) $(OPT)

$(BIN)bound_abstract.o : $(SRC)bound_abstract.cpp
	g++ -o $(BIN)bound_abstract.o -c $(SRC)bound_abstract.cpp $(OPT)

$(BIN)bound_flowshop.o : $(SRC)bound_flowshop.cpp
	g++ -o $(BIN)bound_flowshop.o -c $(SRC)bound_flowshop.cpp $(OPT)

$(BIN)peer.o : $(SRC)peer.cpp 
	g++ -o $(BIN)peer.o -c $(SRC)peer.cpp $(OPT)

$(BIN)tree.o : $(SRC)tree.cpp 
	g++ -o $(BIN)tree.o -c $(SRC)tree.cpp $(OPT)

$(BIN)explorer.o : $(SRC)explorer.cu
	$(NVCC) -g -o $(BIN)explorer.o -c $(SRC)explorer.cu $(INCLUDES) $(LIB) $(PG) #-lcutil

$(BIN)problem.o : $(SRC)problem.cpp 
	g++ -o $(BIN)problem.o -c $(SRC)problem.cpp $(OPT)

$(BIN)solutions.o : $(SRC)solutions.cpp 
	g++ -o $(BIN)solutions.o -c $(SRC)solutions.cpp $(OPT)

$(BIN)problems.o : $(SRC)problems.cpp 
	g++ -o $(BIN)problems.o -c $(SRC)problems.cpp $(OPT)

$(BIN)eliminate_pareto.o : $(SRC)eliminate_pareto.cpp 
	g++ -o $(BIN)eliminate_pareto.o -c $(SRC)eliminate_pareto.cpp $(OPT)

$(BIN)work.o : $(SRC)work.cpp 
	g++ -o $(BIN)work.o -c $(SRC)work.cpp $(OPT)

$(BIN)works.o : $(SRC)works.cpp 
	g++ -o $(BIN)works.o -c $(SRC)works.cpp $(OPT)

$(BIN)weights.o : $(SRC)weights.cpp 
	g++ -o $(BIN)weights.o -c $(SRC)weights.cpp $(OPT)

$(BIN)ttime.o : $(SRC)ttime.cpp 
	g++ -o $(BIN)ttime.o -c $(SRC)ttime.cpp $(OPT)

$(BIN)BigInteger.o : $(MTH)BigInteger.cc
	g++ -o $(BIN)BigInteger.o -c $(MTH)BigInteger.cc $(OPT)

$(BIN)BigIntegerUtils.o : $(MTH)BigIntegerUtils.cc
	g++ -o $(BIN)BigIntegerUtils.o -c $(MTH)BigIntegerUtils.cc $(OPT)

$(BIN)BigUnsigned.o : $(MTH)BigUnsigned.cc
	g++ -o $(BIN)BigUnsigned.o -c $(MTH)BigUnsigned.cc $(OPT)

$(BIN)BigUnsignedInABase.o : $(MTH)BigUnsignedInABase.cc
	g++ -o $(BIN)BigUnsignedInABase.o -c $(MTH)BigUnsignedInABase.cc $(OPT)

$(BIN)pbab.o : $(SRC)pbab.cpp 
	g++ -o $(BIN)pbab.o -c $(SRC)pbab.cpp $(OPT)

$(BIN)arguments.o : $(SRC)arguments.cpp 
	g++ -o $(BIN)arguments.o -c $(SRC)arguments.cpp $(OPT)

$(BIN)select_worst.o : $(SRC)select_worst.cpp 
	g++ -o $(BIN)select_worst.o -c $(SRC)select_worst.cpp $(OPT)

$(BIN)bb_problem_calculator.o : $(SRC)bb_problem_calculator.cpp 
	g++ -o $(BIN)bb_problem_calculator.o -c $(SRC)bb_problem_calculator.cpp $(OPT)
	
$(BIN)communication.o : $(SRC)work_stealing/communication.cpp 
	g++ -o $(BIN)communication.o -c $(SRC)work_stealing/communication.cpp $(OPT)

$(BIN)node.o : $(SRC)work_stealing/node.cpp 
	g++ -o $(BIN)node.o -c $(SRC)work_stealing/node.cpp $(OPT)

$(BIN)packets.o : $(SRC)work_stealing/packets.cpp 
	g++ -o $(BIN)packets.o -c $(SRC)work_stealing/packets.cpp $(OPT)

$(BIN)protocol.o : $(SRC)work_stealing/protocol.cpp 
	g++ -o $(BIN)protocol.o -c $(SRC)work_stealing/protocol.cpp $(OPT)

$(BIN)statistics.o : $(SRC)work_stealing/statistics.cpp 
	g++ -o $(BIN)statistics.o -c $(SRC)work_stealing/statistics.cpp $(OPT)
	
clean :
	rm -fr $(BIN)*
	rm -fr $(SRC)work_stealing/manager_fixed_random_work_stealing
	rm -fr $(SRC)work_stealing/manager_gpu_cpu_cluster