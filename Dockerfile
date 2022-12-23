FROM ubuntu:focal

ARG USER_ID
ARG GROUP_ID
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -y git cmake make ruby gcc python3 python3-pip gcc-arm-none-eabi

RUN pip install pyyaml

# if either of these are already set the same as the user's machine, leave them be and ignore the error
RUN addgroup --gid $GROUP_ID users; exit 0;
RUN adduser --disabled-password --gecos '' --uid $USER_ID --gid $GROUP_ID user; exit 0;

USER user
RUN git config --global --add safe.directory /src

VOLUME /src

WORKDIR /src/build
ENTRYPOINT ["/src/cmake/docker.sh"]
