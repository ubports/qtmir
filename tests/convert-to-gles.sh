#!/bin/sh

set -eux

QUILT_EXIT=0
export QUILT_PATCHES=debian/gles-patches

quilt push -a || QUILT_EXIT=$?
quilt pop -a

exit $QUILT_EXIT
