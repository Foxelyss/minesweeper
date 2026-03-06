FROM ubuntu:22.04

RUN apt update
RUN apt install software-properties-common -y
RUN apt install bash -y

RUN add-apt-repository universe

RUN apt install scons clang -y

USER 1000:1000

ENTRYPOINT ["/bin/bash"]
