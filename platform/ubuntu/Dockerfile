FROM ubuntu:22.04  AS base-image

RUN apt update -y
RUN apt install -y make g++ libjack-dev

RUN apt install -y python3
RUN apt install -y python3-pip
RUN pip3 install scons

FROM ubuntu:22.04
COPY --from=base-image / /
