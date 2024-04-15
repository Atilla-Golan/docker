# Use Alpine Linux 3.17.4 as the base image
FROM alpine:3.17.4

# Install build essentials (GCC and standard C libraries)
# In Alpine, the build-base package includes gcc, g++, and make among others.
RUN apk add --no-cache gcc musl-dev make

# Set the working directory in the Docker image
WORKDIR /app

# Copy the C source codes into the Docker image
COPY client1.c /app/client1.c
COPY client2.c /app/client2.c

# Compile the C program to produce an executable named 'client1'.
# This command is straightforward since the build-base package includes GCC.
RUN gcc -o client1 client1.c
RUN gcc -o client2 client2.c

# Set the default command for the container
CMD ["sh"]