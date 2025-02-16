FROM archlinux:latest

ARG USERNAME=username
ARG USER_UID=1000
ARG USER_GID=$USER_UID
ARG WORKSPACE=$WORKSPACE

# Создаем директорию /etc/sudoers.d/, если она не существует
RUN mkdir -p /etc/sudoers.d/

RUN echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME
RUN chmod 0440 /etc/sudoers.d/$USERNAME
# Create the user
RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME -p "" \
		&& usermod -aG wheel $USERNAME

# ********************************************************
# * Anything else you want to do like clean up goes here *
# ********************************************************

RUN pacman -Sy --noconfirm reflector
RUN reflector --latest 10 --sort rate --save /etc/pacman.d/mirrorlist
RUN pacman -S --noconfirm \
		base-devel \
		llvm \
		clang \
		gdb \
		cmake \
		ninja \
		git \
		sudo \
		openssh

RUN pacman -Sy --noconfirm \
		mesa \
		glad \
		boost \
		spdlog \
		glfw

# WORKDIR /home/${USERNAME}/TFCADIR
#WORKDIR $WORKSPACE

#COPY ../ .

# [Optional] Set the default user. Omit if you want to keep the default as root.
USER $USERNAME

CMD [ "/bin/bash" ]

# RUN git submodule update --init --recursive
# RUN cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug build
# RUN cmake -B build -S . -j$(nproc)