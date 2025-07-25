FROM ubuntu:jammy

ARG USER_ID
ARG GROUP_ID
ARG GDB

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y git cmake make ruby gcc python3 python3-pip gcc-arm-none-eabi ninja-build

RUN if [ "$GDB" = "yes" ]; then apt-get install -y gdb; fi

RUN pip install pyyaml

# add inav user and group
RUN addgroup --gid $GROUP_ID inav
RUN adduser --disabled-password --gecos '' --uid $USER_ID --gid $GROUP_ID inav

USER inav

RUN git config --global --add safe.directory /src

WORKDIR /src/build

ENTRYPOINT ["/src/cmake/docker.sh"]
