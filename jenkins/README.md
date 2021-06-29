# Jenkins Build Scripts
This directory contains scripts to build libwebm in various configurations. These scripts were created to support Jenkins integration pipelines. But also, can be run by the software engineer.

## Environment
Most of these scripts were ported from Jenkins, so in order to be run locally some environment variables must be set prior.

**CC** Global C compiler toolchain as used in Makefiles. e.g. as gcc, clang.
**CXX** Global C/C++ compiler toolchain as used in Makefiles. e.g. g++, clang.

## LUCI Integration
[Builder Dashboard](https://ci.chromium/p/open-codecs)  
The new builders run these scripts on each CL. Current configuration supports `refs/head/master` branch. 

## Scripts
**- compile** 
