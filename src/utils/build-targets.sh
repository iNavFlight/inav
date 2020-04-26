#!/bin/sh

set -e

usage()
{
   echo "Usage ${0} [-n] [-S suffix] -s START -c COUNT]" >&2;
   echo "Build COUNT targets starting at index START" >&2;
   echo "Targets are retrieved via make targets" >&2;
   echo "-n prints the targets instead of building them" >&2;
   echo "-S sets the BUILD_SUFFIX" >&2;
   exit 1;
}

is_number()
{
    ! [ -z ${1} ] && [ ${1} -eq ${1} ] 2>/dev/null
}

START=
COUNT=
DRY_RUN=
BUILD_SUFFIX=

args=`getopt nS:s:c: $*`
set -- $args
for i
do
    case "$i"
    in
        -n)
            DRY_RUN=1
            shift;;
        -S)
            BUILD_SUFFIX=${2}; shift;
            shift;;
        -s)
            START=${2}; shift;
            shift;;
        -c)
            COUNT=${2}; shift;
            shift;;
        --)
            shift; break;;
    esac
done

if ! is_number ${START} || ! is_number ${COUNT}; then
    usage;
fi

ALL_TARGETS=$(make targets | grep Valid | awk -F' *: *' '{print $2}')
START_IDX=$(expr ${START} + 1)
END_IDX=$(expr ${START} + ${COUNT})
SELECTED_TARGETS=$(echo ${ALL_TARGETS} | cut -d ' ' -f ${START_IDX}-${END_IDX})

if [ -z "${DRY_RUN}" ]; then
    for target in ${SELECTED_TARGETS}; do
        BUILD_SUFFIX=${BUILD_SUFFIX} V=0 CFLAGS=-Werror make ${target}
        # Cleanup intermediate files after building each target.
        # Otherwise we run out of space on GH CI.
        # XXX: Make sure we save the .hex file for artifact
        # generation
        mkdir -p obj/dist
        mv obj/*.hex obj/dist
        V=0 make clean_${target}
    done
else
    echo "${SELECTED_TARGETS}"
fi
