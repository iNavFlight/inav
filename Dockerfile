FROM ubuntu:jammy

ARG USER_ID
ARG GROUP_ID
ARG GDB

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y git cmake make ruby gcc python3 python3-yaml ninja-build gcc-arm-none-eabi

RUN if [ "$GDB" = "yes" ]; then apt-get install -y gdb; fi

# If a group and user with the same IDs already exist, rename the group and recreate the user after deleting the existing one.
RUN GROUP="$(id -n -g $GROUP_ID)"; if [ -n "$GROUP" ]; then groupmod -n inav "$GROUP"; else groupadd --gid $GROUP_ID inav; fi
RUN USER="$(id -n -u $USER_ID)"; if [ -n "$USER" ]; then userdel -r "$USER"; fi && useradd -m --uid $USER_ID --gid $GROUP_ID inav

USER inav

RUN git config --global --add safe.directory /src

WORKDIR /src/build

ENTRYPOINT ["/src/cmake/docker.sh"]
