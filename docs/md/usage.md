﻿# Setup and Usage

## Requirements
This pipeline has only been tested on VCK190 using the following software.

* Vitis Software Platform 2022.2
* xilinx-versal-common image
* X86 XRT (only for software system emulation)
* Python 3

## Setup
Setup python environment. Anaconda/miniconda is an option. Install the requirements in your environment. Run `conda activate o2v` before scripts in `./python`.
```
conda create -n o2v python=3.10 -y
conda activate o2v
which python # (conda path)/bin/python
pip install -r python/requirements.txt
```

Setup following lines in `./sample_env_setup.sh`. Run `source sample_env_setup.sh` to setup environment in your shell.
```
export PLATFORM_REPO_PATHS=/tools/Xilinx/Vitis/2022.2/base_platforms
export XILINX_VITIS=/tools/Xilinx/Vitis/2022.2
export COMMON_IMAGE_VERSAL=/tools/xilinx-versal-common-v2022.2
export XILINX_X86_XRT=/opt/xilinx/xrt
export PYTHON3_LOCATION=/usr/bin
```

## Usage - Generate

```
# takes onnx model and numpy tensor, with batch as first dimension
$ python/generate.py ./models/my_model.onnx ./data/data.npy
```

Example truncated stdout
```
Saving tensor of shape (1, 1, 28, 28) into ../data/input.txt
WARNING: fusing Conv+Relu
Padding Conv weights (6, 1, 5, 5) to (6, 1, 5, 8)
Saving tensor of shape (1, 1, 28, 28) into ../data/k0conv_in_shape1x1x28x28.txt
...
WARNING: Shape not implemented, skipping...
WARNING: Constant not implemented, skipping...
...
Found matching output /Reshape_output_0 and k5pool output
Disabled output padding, may result in choosing scalar op instead of vector.
...
Generating MNIST txt for 100 data points
```

Example generated files
```
models/
├── lenet_mnist_inter.onnx              # generated onnx model to output at all layers
├── lenet_mnist.onnx
└── lenet_mnist.pkl
data/
├── input.txt                           # generated input data
├── input_host.txt                      # generated batch input data for e2e test
├── k0conv_goldenout.txt                # intermediate inputs and outputs for verification
├── k0conv_in.txt
├── k14gemm_goldenout.txt
├── k14gemm_in.txt
├── k16gemm_goldenout.txt
├── k16gemm_in.txt
├── k18gemm_goldenout.txt               # generated output data
├── k18gemm_goldenout_host.txt          # generated batch output data for e2e host
├── k18gemm_in.txt
├── k2pool_goldenout.txt
├── k2pool_in.txt
├── k3conv_goldenout.txt
├── k3conv_in.txt
├── k5pool_goldenout.txt
├── k5pool_in.txt
design/
├── aie_src
│   └── graph_lenet_mnist.cpp           # aiengine computation graph for aiecompiler
├── host_app_src
│   └── lenet_mnist_aie_app.cpp         # host code to load data and run compiled graph
├── system_configs
│   ├── lenet_mnist.cfg                 # configuration for v++ linking
│   └── lenet_mnist_output_inter.cfg
└── trafficgen
    ├── xtg_lenet_mnist_output_inter.py # traffic generators for testing computation graph
    └── xtg_lenet_mnist.py
```

## Usage - Simulation, Verification, Profiling, Hardware Build
`make help` for full details. Only key (and probably insufficient) examples here. <br />
<b>If you run into issues running the make, try to run each recipe one by one.</b>

```
OPTIONS:
Use the make recipes with required values for options mentioned below-
    TARGET      sw_emu(default)|hw_emu|hw, build target
    GRAPH       lenet (default),           target graph as per design/aie_src/graph_[].cpp
    EXTIO       0 (default) | 1,           traffic gen usage, graph runs only, redundant for system due to host script
    DOUT        1 (default) | 0,           if enable output intermediates, max 6-7 outputs, AIE has <= 8 cascade channels

## Functional check: runs graph in x86simulator
$ TARGET=sw_emu GRAPH=my_model [EXTIO=1] [DOUT=0] make graph clean_reports aiesim

## Functional check: runs system software emulation (x86 graph, sysC kernels)
$ TARGET=sw_emu GRAPH=my_model [DOUT=0] make graph kernels xsa application package clean_reports run_emu

## Functional/performance check: runs graph in aiesimulator
$ TARGET=hw_emu GRAPH=my_model [EXTIO=1] [DOUT=0] make graph clean_reports aiesim_profile

## Functional/performance check: runs system in hardware emulation with QEMU (sysC graph, kernels NoC, DDR)
$ TARGET=hw_emu GRAPH=my_model [DOUT=0] make graph kernels xsa application package run_emu


## Hardware: create hardware image, flash the SD card
$ TARGET=hw GRAPH=my_model [DOUT=0] make graph kernels xsa application package
$ sudo dd if=build/my_model/hw/package/sd_card.img of=/dev/(DEVICE) conv=fsync status=progress
```

## Directory structure
```
design/
├── aie_src
│   ├── *.cc            # kernel
│   ├── *.h
│   ├── graph*.cpp      # unit test
│   └── graph*.h        # graph
├── directives          # directives for linking
├── exec_scripts        # bash scripts to be packaged
├── host_app_src
│   └── *_aie_app.cpp   # host script for PS, used in system run
├── pl_src              # data moving kernels
├── profiling_configs   # xrt.inis for hardware profiling
├── system_configs      # .cfg for v++ linking
│   ├── *.cfg
│   └──  *_output_inter.cfg
├── trafficgen          # traffic gen python scripts for graph runs
│   ├── xtg_*.py
│   └──  xtg_*_output_inter.py
└── vivado_metrics_scripts
```