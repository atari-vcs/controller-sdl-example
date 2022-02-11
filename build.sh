#!/bin/bash
#
# Copyright 2022 Collabora, Ltd.
#
# SPDX-License-Identifier: MIT
#

# This will build the example in the Atari VCS build container, and can then be run on the VCS.
exec docker run -it --rm -v $(pwd):/build -w /build ghcr.io/atari-vcs/vcs-build-container:base make
