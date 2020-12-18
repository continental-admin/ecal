
# eCAL - enhanced Communication Abstraction Layer

Copyright (c) 2020, Continental Corporation.

🏠 http://ecal.io

#### License

[![License](https://img.shields.io/github/license/continental/ecal.svg?style=flat)](LICENSE.txt)

#### Build States

[![Build Windows Server 2019](https://github.com/continental/ecal/workflows/Build%20Windows%20Server%202019/badge.svg)](https://github.com/continental/ecal/actions?workflow=Build+Windows+Server+2019)

[![Build Ubuntu 18.04](https://github.com/continental/ecal/workflows/Build%20Ubuntu%2018.04/badge.svg)](https://github.com/continental/ecal/actions?workflow=Build+Ubuntu+18.04)

[![Build Ubuntu 20.04](https://github.com/continental/ecal/workflows/Build%20Ubuntu%2020.04/badge.svg)](https://github.com/continental/ecal/actions?workflow=Build+Ubuntu+20.04)

[![Build Ubuntu 20.04 (Iceoryx)](https://github.com/continental/ecal/workflows/Build%20Ubuntu%2020.04%20(Iceoryx)/badge.svg)](https://github.com/continental/ecal/actions?workflow=Build+Ubuntu+20.04+(Iceoryx))

[![Build macOS Catalina 10.15](https://github.com/continental/ecal/workflows/Build%20macOS%20Catalina%2010.15/badge.svg)](https://github.com/continental/ecal/actions?workflow=Build+macOS+Catalina+10.15)

## Preface

The **e**nhanced **C**ommunication **A**bstraction **L**ayer (eCAL) is a middleware that enables scalable, high performance interprocess communication on a single computer node or between different nodes in a computer network.
eCAL uses a **publish / subscribe** pattern to automatically connect different nodes in the network. It's choosing the best available data transport mechanisms - **UDP** for network communication and **High Performance Shared Memory** for local communication.

eCAL

* is fast (1 - 10 GB/s depends on payload size - check measured performance [here](https://continental.github.io/ecal/advanced/performance.html))
* provides publish / subscribe and client / server patterns
* is designed brokerless
* provides a C++ and C interface for easy integration into other languages (like python or csharp)
* supports intel and arm platforms
* has powerful tools for [recording](https://continental.github.io/ecal/getting_started/recorder.html), [replay](https://continental.github.io/ecal/getting_started/player.html) and [monitoring](https://continental.github.io/ecal/getting_started/monitor.html) all your data flows - decentralized
* integrates gently into your ROS2 environement with the brand new [eCAL RMW](https://github.com/continental/rmw_ecal)  

eCAL runs on

* Windows (stable)
* Linux (stable)
* QNX (stable)
* MacOS (experimental)
* FreeBSD (experimental)

Really ?? Just another pub/sub middleware ? Why ? What is the different to my beloved fully featured DDS ?

* it's stupid simple - no complex configuration effort for all kind of communication details or QOS settings
* it's message protocol agnostic - choose the message protocol that fits to your needs like [Google Protobuf](https://developers.google.com/protocol-buffers), [CapnProto](https://capnproto.org/), [Flatbuffers](https://google.github.io/flatbuffers/) ..
* it's using the standardized recording format [HDF5](https://www.hdfgroup.org/solutions/hdf5/)

What is on the roadmap ?

* more language bindings (rust, go-lang)
* and ?? tell us about your use case :-)

GitHub project using / used by eCAL

* eCAL & ROS2:
  * [ROS2 eCAL Middleware Layer](https://github.com/continental/rmw_ecal)
  * [ROS2 to eCAL Gateway](https://github.com/schilasky/ros2ecal)
  * [eCAL to ROS2 Gateway](https://github.com/schilasky/ecal2ros)
* [Iceoryx](https://github.com/eclipse/iceoryx)
* [GO language interface](https://github.com/Blutkoete/golang-ecal)
* [Dockerfiles for eCAL](https://github.com/Blutkoete/docker-ecal)

## Links

- **eCAL Documentation**: https://continental.github.io/ecal
- **Getting Started Tutorial**: https://continental.github.io/ecal/getting_started/introduction
- **Downloads & Releases**: https://github.com/continental/ecal/releases
