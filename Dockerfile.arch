FROM archlinux/base

# Install needed packages
RUN pacman -Syu --noconfirm llvm clang lld
RUN pacman -Syu --noconfirm boost
RUN pacman -Syu --noconfirm make cmake
RUN pacman -Syu --noconfirm grep python2

# Copy the Sparrow source files
RUN mkdir /sparrow
RUN mkdir /sparrow/src
RUN mkdir /sparrow/externals
RUN mkdir /sparrow/unittests
RUN mkdir /sparrow/SparrowImplicitLib
COPY src /sparrow/src
COPY externals /sparrow/externals
COPY unittests /sparrow/unittests
COPY SparrowImplicitLib /sparrow/SparrowImplicitLib
COPY CMakeLists.txt /sparrow/
COPY Macros.cmake /sparrow/
COPY LICENSE /sparrow/
COPY .git /sparrow/.git

# Copy the test files
RUN mkdir /sparrow_tests
COPY tests /sparrow_tests

# Do the build
WORKDIR /sparrow
RUN mkdir /sparrow/build
WORKDIR /sparrow/build
# RUN CC=clang CXX=clang++ cmake ..
RUN cmake ..
RUN cmake --build . -- -j4
RUN cmake --build . -- install

# Run the unit tests
WORKDIR /sparrow/build/bin
RUN ./SparrowUnitTests

# Ensure the newly installed libraries can be found
RUN ldconfig

# Test the build
WORKDIR /sparrow_tests
RUN python2 test.py

# Remove temporary files
RUN rm -rf /sparrow
