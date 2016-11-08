FROM sparrowlang/sparrowdeps

# Copy the Sparrow source files
RUN mkdir /sparrow
RUN mkdir /sparrow/src
RUN mkdir /sparrow/SparrowImplicitLib
COPY src /sparrow/src
COPY SparrowImplicitLib /sparrow/SparrowImplicitLib
COPY conanfile.txt /sparrow/
COPY CMakeLists.txt /sparrow/
COPY Macros.cmake /sparrow/
COPY LICENSE /sparrow/

# Copy the test files
RUN mkdir /sparrow_tests
COPY tests /sparrow_tests

# Do the build
WORKDIR /sparrow
RUN mkdir /sparrow/build
WORKDIR /sparrow/build
RUN conan install .. --build=missing
RUN cmake ..
RUN cmake --build . -- -j4
RUN cmake --build . -- install

# Ensure the newly installed libraries can be found
RUN ldconfig

# Test the build
WORKDIR /sparrow_tests
RUN python test.py

# Remove temporary files
RUN rm -rf /sparrow
