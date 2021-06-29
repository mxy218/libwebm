# Infra Build Scripts

This directory contains scripts to build libwebm in various configurations.
These scripts were created to support Jenkins integration pipelines. But also,
can be run locally.

## Environment

Most of these scripts were ported from Jenkins, so in order to be run locally
some environment variables must be set prior.

**CC** Global C compiler toolchain as used in Makefiles. e.g. as gcc, clang. \
**CXX** Global C/C++ compiler toolchain as used in Makefiles. e.g. g++, clang. \
**WORKSPACE** Traditionally, the Jenkins `WORKSPACE` path. If not defined, a
temporary directory will be used.

## LUCI Integration

[Builder Dashboard](https://ci.chromium/p/open-codecs) \
The new builders run these scripts on each CL. The current configuration
supports `refs/head/main` branch.

## Scripts

**compile.sh** Builds libwebm with supported configuration and supported host
toolchains.
