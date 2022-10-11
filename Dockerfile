FROM ubuntu:focal

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -y git cmake make ruby gcc python3 python3-pip

RUN pip install pyyaml

RUN useradd inav

USER inav

VOLUME /src

WORKDIR /src/build
ENTRYPOINT ["/src/cmake/docker.sh"]
