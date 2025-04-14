FROM alpine:3.20

#Install the required packages
RUN apk update && \
    apk add --no-cache \
    build-base \
    cmake=3.29.3-r0 \
    wget \
    gdal-dev \
    opencv-dev \
    rocksdb-dev \
    libzip-dev \
    git \
    ninja && \
    rm -rf /var/cache/apk/*

# Set the working directory
WORKDIR /usr/src/app

# Copy the source to the working directory
COPY cmake/ ./cmake/
COPY img/ ./img/
COPY imgtrial/ ./imgtrial/
COPY include/ ./include/
COPY lib/ ./lib/
COPY src/ ./src/
COPY startproject/ ./startproject/
COPY tests/ ./tests/
COPY var/ ./var/
COPY external/ ./external/
COPY CMakeLists.txt ./
COPY Doxyfile ./
COPY LICENSE ./
COPY Pipeline.cmake ./
COPY README.md ./

# Build the project
RUN mkdir build && \
    cd build && \
    cmake -G Ninja .. && \
    ninja

# Run the project
WORKDIR /usr/src/app/build
CMD ["./server"]

