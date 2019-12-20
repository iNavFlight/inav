FROM ubuntu:bionic

# Configuration
VOLUME /home/src/
WORKDIR /home/src/
ARG TOOLCHAIN_VERSION_SHORT
ENV TOOLCHAIN_VERSION_SHORT ${TOOLCHAIN_VERSION_SHORT:-"8-2018q4"}
ARG TOOLCHAIN_VERSION_LONG
ENV TOOLCHAIN_VERSION_LONG ${TOOLCHAIN_VERSION_LONG:-"8-2018-q4-major"}

# Essentials
RUN mkdir -p /home/src && \
    apt-get update && \
    apt-get install -y software-properties-common ruby make git gcc wget curl bzip2

# Toolchain
RUN wget -P /tmp "https://developer.arm.com/-/media/Files/downloads/gnu-rm/$TOOLCHAIN_VERSION_SHORT/gcc-arm-none-eabi-$TOOLCHAIN_VERSION_LONG-linux.tar.bz2"
RUN mkdir -p /opt && \
	cd /opt && \
    tar xvjf "/tmp/gcc-arm-none-eabi-$TOOLCHAIN_VERSION_LONG-linux.tar.bz2" -C /opt && \
	chmod -R -w "/opt/gcc-arm-none-eabi-$TOOLCHAIN_VERSION_LONG"

ENV PATH="/opt/gcc-arm-none-eabi-$TOOLCHAIN_VERSION_LONG/bin:$PATH"
