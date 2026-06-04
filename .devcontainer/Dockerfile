FROM  ubuntu:24.04

ARG ANDROID_SDK_VERSION=35
ARG ANDROID_BUILD_TOOLS_VERSION=35.0.0
ARG ANDROID_NDK_VERSION=r26d
ARG CMDLINE_TOOLS_URL=https://dl.google.com/android/repository/commandlinetools-linux-11076708_latest.zip
ENV DEBIAN_FRONTEND=noninteractive
USER root

# Common build dependencies (from xcsoar.readthedocs.io/en/latest/build.html)

RUN apt-get update && apt-get install -y --no-install-recommends \
        make \
        cmake \
        ninja-build \
        g++ \
        libtool \
        autoconf \
        pkg-config \
        libfmt-dev \
        librsvg2-bin \
        xsltproc \
        imagemagick \
        gettext \
        sox \
        git \
        quilt \
        zip \
        unzip \
        m4 \
        automake \
        default-jdk-headless \
        vorbis-tools \
        curl \
        ca-certificates \
        xz-utils \
        python3 \
        pip \
        pipx \
        sudo \
    && rm -rf /var/lib/apt/lists/*

# Android SDK

ENV ANDROID_HOME=/home/ubuntu/opt/android-sdk-linux

RUN mkdir -p ${ANDROID_HOME}/cmdline-tools && \
    curl -fsSL ${CMDLINE_TOOLS_URL} -o /tmp/cmdline-tools.zip && \
    unzip -q /tmp/cmdline-tools.zip -d ${ANDROID_HOME}/cmdline-tools && \
    mv ${ANDROID_HOME}/cmdline-tools/cmdline-tools ${ANDROID_HOME}/cmdline-tools/latest && \
    rm /tmp/cmdline-tools.zip

ENV PATH="${ANDROID_HOME}/cmdline-tools/latest/bin:${PATH}"

RUN yes | sdkmanager --licenses > /dev/null 2>&1 && \
    sdkmanager "build-tools;${ANDROID_BUILD_TOOLS_VERSION}" "platforms;android-${ANDROID_SDK_VERSION}"
# Android NDK

ENV ANDROID_NDK_HOME=/home/ubuntu/opt/android-ndk-${ANDROID_NDK_VERSION}

RUN curl -fsSL https://dl.google.com/android/repository/android-ndk-${ANDROID_NDK_VERSION}-linux.zip -o /tmp/ndk.zip && \
    unzip -q /tmp/ndk.zip -d /home/ubuntu/opt && \
    rm /tmp/ndk.zip

RUN chown -R ubuntu:ubuntu /home/ubuntu/opt && \
    echo 'ubuntu:ubuntu' | chpasswd

RUN echo 'export PATH="$PATH:/home/ubuntu/.local/bin"' >> /home/ubuntu/.bashrc && \
    echo 'eval "$(register-python-argcomplete pipx)"' >> /home/ubuntu/.bashrc

USER ubuntu

RUN pipx ensurepath && pipx install pre-commit

WORKDIR /workspaces/XCSoar
