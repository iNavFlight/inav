FROM ubuntu:xenial
LABEL maintainer Andy Schwarz <flyandi@yahoo.com>

# Configuration
VOLUME /home/src/
WORKDIR /home/src/

# Essentials
RUN mkdir -p /home/src && \
    apt-get update && \
    apt-get install -y software-properties-common python-software-properties ruby make git gcc wget curl bzip2 lib32ncurses5 lib32z1

# Toolchain
ENV TOOLCHAIN=
ENV TOOLCHAIN_ID=
RUN wget -P /tmp https://developer.arm.com/-/media/Files/downloads/gnu-rm/6-2017q2/gcc-arm-none-eabi-6-2017-q2-update-linux.tar.bz2
RUN mkdir -p /opt && \
	cd /opt && \
    tar xvjf /tmp/gcc-arm-none-eabi-6-2017-q2-update-linux.tar.bz2 -C /opt && \
	chmod -R -w /opt/gcc-arm-none-eabi-6-2017-q2-update

ENV PATH="/opt/gcc-arm-none-eabi-6-2017-q2-update/bin:${PATH}"