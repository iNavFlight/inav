FROM ubuntu:jammy

ARG USER_ID
ARG GROUP_ID
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -y git cmake make ruby gcc python3 python3-pip gcc-arm-none-eabi ninja-build

RUN pip install pyyaml

# if either of these are already set the same as the user's machine, leave them be and ignore the error
RUN if [ -n "$USER_ID" ]; then RUN addgroup --gid $GROUP_ID inav; exit 0; fi
RUN if [ -n "$USER_ID" ]; then  RUN adduser --disabled-password --gecos '' --uid $USER_ID --gid $GROUP_ID inav; exit 0; fi

RUN if [ -n "$USER_ID" ]; then USER inav; fi
RUN git config --global --add safe.directory /src

VOLUME /src

WORKDIR /src/build
ENTRYPOINT ["/src/cmake/docker.sh"]
