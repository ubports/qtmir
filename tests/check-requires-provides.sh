#!/bin/bash

# don't run the test on the gles build
dummy=`grep qtmir-desktop debian/control`
if [ $? -ne 0 ]; then
    exit 0
fi

requires=`grep unity-shell-application= CMakeLists.txt | sed -E 's/.*=(.*)\)/\1/'`
provides=`grep unity-application-impl- debian/control | sed -E 's/.*unity-application-impl-(.*),/\1/'`
if [ "$requires" -eq "$provides" ]; then
    exit 0
else
    echo "Error: We require unity-shell-application=$requires but provide unity-application-impl-$provides"
    exit 1
fi
