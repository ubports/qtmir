#!/bin/sh

set -eux

# Skip this test inside qtmir-gles package as the patch is already applied.
[ "$(dpkg-parsechangelog --show-field Source)" = "qtmir" ] || exit 0

QUILT_EXIT=0
export QUILT_PATCHES=debian/gles-patches

quilt push -a || QUILT_EXIT=$?
quilt pop -a

exit $QUILT_EXIT
