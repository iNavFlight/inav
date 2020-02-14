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
    # make without arguments builds the default
    # target, so make sure ${SELECTED_TARGETS} is
    # not empty
    if [ -n "${SELECTED_TARGETS}" ]; then
        BUILD_SUFFIX=${BUILD_SUFFIX} V=0 CFLAGS=-Werror make ${SELECTED_TARGETS}
    fi
else
    echo "${SELECTED_TARGETS}"
fi
