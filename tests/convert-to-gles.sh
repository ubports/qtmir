#!/bin/sh

set -eux

QUILT_EXIT=0
export QUILT_PATCHES=debian/gles-patches

# Ensure we start from a clean slate, even in the -gles package.
patch --strip=1 --reverse <debian/gles-patches/convert-to-gles.patch || true

quilt push -a || QUILT_EXIT=$?
quilt pop -a

exit $QUILT_EXIT
