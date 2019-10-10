## Compilation

### Compile ffRDMA library
You can use the version provided by the compressed package or pull the master branch from GitHub directly
(github.com/thincats/ffrdma)

ffRDMA uses CMake as buildsystem
```sh
# in ffrdma dir
[ -d build ] || mkdir build
cd build
cmake ..
make -j
```
The compiled libfmpi.a is in the build directory

### Compile helper tool ffrun
ffrun uses go to compile and recommends go version >= 1.13
```sh
# in ffrdma dir
cd tools/ffrun
go build
```
Generate an executable file `ffrun`, recommend running environment glibc >= 2.9

### Compile the application code
#### Compile miniAMR - ffrdma version
It is recommended to use versions in compressed packages
```sh
tar -xvf RDMA_AMR-rdma.tar && cd RDMA_AMR-rdma
# Step into ffrdma dir to compile library code
cd ffrdma
[ -d build ] || mkdir build
cd build
cmake ..
make -j

cd ../../
make -j
```

#### Compile Gadget 2 - ffrdma Edition
Using Compressed package Version
Dependencies: Needs gsl
```sh
  # if use spack
  spack install gsl
  # if by compilation
  # Edit Makefile:
  -L ${PATH_TO_GSL}/lib -I ${PATH_TO_GSL}/include
```
```sh
tar -xvf RDMA_AMR-rdma.tar && cd RDMA_AMR-rdma
tar -xvf N-body.tar && cd N-body-sendrecv

# Step into ffrdma dir to compile library code
cd ffrdma
[ -d build ] || mkdir build
cd build
cmake ..
make -j

cd ../../
make -j
```

#### Compile Gadget2 - Original
To reduce unnecessary dependencies, we modified the Makefile of Gadget2 to disable some features.

We are not use the output as HDF5 format. And because fftw uses MPI internally, it is not conducive to code mixing

We also take out fftw. So the original version of the compilation needs to use the Makefile we've provided to reproduce the results.

Copy the Gadget2-Makefile.
```
cp ${XX_PATH}/Gadget2-Makefile ./
make -j -f Gadget2-Makefile
```

### Run

#### ffrun
The modified program will take some additional command line parameters to get the corresponding running environment, such as opening ports and discovering peer processes.

Additional command line parameters are defined as follows
```sh
<program> --r_myip <ip> --r_myport <port> --r_hostmap <ip1>:<port1>+<port2>...,<ip2>:<port1>+<port2>...,... -- <program origninal parameters>
```
Simple tests can be done by passing parameters manually, but after all, annoying. We use go to write an helper tool which is like mpirun.

The ffrun, which has these features:

- Generate startup script, startup parameters

- Running a specified number of processes based on the -H parameter (similar to mpirun -H)

- Compatible with singularity

- Integrating different rank output, displaying rank0's running wall time, managing multiple processes, and handling process exit

*no support*

- Automatic cross-node startup: Manual execution is required, but scripts can be written to start.

- Host Specifies Non-IP: The Host specified in the parameter must be ip. Domain name or host name is not supported for current version.

Gadget2 as an example:

One node with 4 processes
```sh
# when encounter --, the rest of args are treated as the origninal program's args
ffrun run -H 10.10.0.110:4 -p ./Gadget2 -- parameterfiles/cluster.param
```

One node with 4 processes in singularity
```sh
# -s adds singularity support, singularity exec is the same as directly call
ffrun run -H 10.10.0.110:4 -s -- singularity exec n-body-final.sif gadget.sh
# if requires root, simply pass sudo before singularity
ffrun run -H 10.10.0.110:4 -s -- sudo singularity exec n-body-final.sif gadget.sh
```

Two or more nodes, with eight processes

You can't simply start the process with the - H parameter. You need to use `ffrun generate ` to get `--hostmap`, and specify it in `ffrun run`.

You need to run two commands on two nodes, and if you can, try to run the one with the larger IP address first.
(The rank will be more higher)

```sh
# Firstly ffrun generate
ffrun generate -H 10.10.0.1:4,10.10.0.2:4 -p ./Gadget2
# copy the generated hostmap
# like 10.10.0.1:23960+29461+64662+15028,10.10.0.2:23960+29461+64662+15028

# On node1: 10.10.0.1 and On node2: 10.10.0.2
ffrun run --hostmap 10.10.0.1:23960+29461+64662+15028,10.10.0.2:23960+29461+64662+15028 -s -- singularity exec n-body-final.sif gadget.sh
```

#### Run miniAMR
Two nodes with 16 processes(8 + 8)

ffrdma on host
```sh
# on node1 and node2
ffrun run --hostmap 10.10.0.112:23126+39936+46102+57849+57008+17854+21771+26142,10.10.0.110:23126+39936+46102+57849+57008+17854+21771+26142 -p ./miniAMR.x -- --num_refine 4 --max_blocks 4000 --init_x 1 --init_y 1 --init_z 1 --npx 4 --npy 2 --npz 2 --nx 8 --ny 8 --nz 8 --num_objects 2 --object 2 0 -1.10 -1.10 -1.10 0.030 0.030 0.030 1.5 1.5 1.5 0.0 0.0 0.0 --object 2 0 0.5 0.5 1.76 0.0 0.0 -0.025 0.75 0.75 0.75 0.0 0.0 0.0 --num_tsteps 10 --checksum_freq 4 --stages_per_ts 16
```

ffrdma on singularity
```sh
# on node1 and node2
ffrun run --hostmap 10.10.0.112:23126+39936+46102+57849+57008+17854+21771+26142,10.10.0.110:23126+39936+46102+57849+57008+17854+21771+26142 -s -- singularity exec miniAMR-final.sif miniAMR.x --num_refine 4 --max_blocks 4000 --init_x 1 --init_y 1 --init_z 1 --npx 4 --npy 2 --npz 2 --nx 8 --ny 8 --nz 8 --num_objects 2 --object 2 0 -1.10 -1.10 -1.10 0.030 0.030 0.030 1.5 1.5 1.5 0.0 0.0 0.0 --object 2 0 0.5 0.5 1.76 0.0 0.0 -0.025 0.75 0.75 0.75 0.0 0.0 0.0 --num_tsteps 10 --checksum_freq 4 --stages_per_ts 16
```

openmpi
```sh
mpirun -x PATH -x LD_LIBRARY_PATH -hostfile hostfile miniAMR.x --num_refine 4 --max_blocks 4000 --init_x 1 --init_y 1 --init_z 1 --npx 4 --npy 2 --npz 2 --nx 8 --ny 8 --nz 8 --num_objects 2 --object 2 0 -1.10 -1.10 -1.10 0.030 0.030 0.030 1.5 1.5 1.5 0.0 0.0 0.0 --object 2 0 0.5 0.5 1.76 0.0 0.0 -0.025 0.75 0.75 0.75 0.0 0.0 0.0 --num_tsteps 10 --checksum_freq 4 --stages_per_ts 16
```

#### Gadget2

ffrdma on host

Before running, create the `cluster ` directory under the current directory
```sh
# on both node1 and node2 run
ffrun run --hostmap 10.10.0.110:57466+30290+58662+56160+11931+18990+64148+22931+49499+44053+63808+15239+62803+21368+12825+61725,10.10.0.112:57466+30290+58662+56160+11931+18990+64148+22931+49499+44053+63808+15239+62803+21368+12825+61725 -p ./Gadget2 -- parameterfiles/cluster.param
```

ffrdma on singularity

Because Gadget2 needs to write runtime logs to the file system, it needs overlay FS support from singularity. To prevent unnecessary overlay setup, we wrote a script to move Gadget2 and necessary files to TEMPFS at runtime. The results can be viewed in `tmp/singularity/Gadget2-singularity`.

The script `gadget.sh` has been written to the singularity images `n-body-final.sif`

```sh
# on both node1 and node2 run
ffrun run --hostmap 10.10.0.110:57466+30290+58662+56160+11931+18990+64148+22931+49499+44053+63808+15239+62803+21368+12825+61725,10.10.0.112:57466+30290+58662+56160+11931+18990+64148+22931+49499+44053+63808+15239+62803+21368+12825+61725 -s -- singularity exec n-body-final.sif gadget.sh
```

openmpi
```sh
mpirun -x PATH -x LD_LIBRARY_PATH --prefix $TCP_MPI_PREFIX -hostfile hostfile ./Gadget2 parameterfiles/cluster.param
```