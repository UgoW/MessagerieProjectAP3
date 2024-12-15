# COMSEC - Secure Communication System

COMSEC is a secure communication system that allows users to send messages, create and join channels, and list connected users. This project is implemented in C and uses sockets for network communication.

## Features

- **Messaging**: Send private messages to other users.
- **Channels**: Create and join channels for group communication.
- **User Listing**: List all connected users.
- **Channel Listing**: List all available channels.
- **Exit**: Exit the application.

## Commands

- `/msg [username] [content]`: Start messaging with other users.
- `/list`: List all connected users.
- `/exit`: Exit the application.
- `/create [channel_name]`: Create a channel.
- `/join [channel_name]`: Join a channel.
- `/list_channels`: List all channels.
- `/leave`: Leave the channel.

## Installation

1. Clone the repository:
    ```sh
    git clone https://github.com/UgoW/MessagerieProjectAP3.git
    cd MessagerieProjectAP3
    ```

3. Launch and build the projet:
    ```sh
    docker-compose up --build 
    ```
   
## Usage

### Server

1. Run the server:
    ```sh
    docker exec -it server bash
   ./builddir/server
    ```

### Client

1. Run the client:
    ```sh
    docker exec -it client bash
    ./builddir/client
     ```

2. Enter your username when prompted.

3. Use the commands listed above to interact with the system.

## File Structure

- `Includes/`: Header files.
- `src/`: Source code files.
- `Makefile`: Build script.

## Perspectives for Evolution

- **Better Protocol Management**: Improve the protocol handling for more efficient communication.
- **Secure Communication**: Implement RSA, AES, or SSL/TLS for secure communication.
- **Better Error Handling**: Enhance input error management for a more robust system.

## Dependencies

- `pthread`: POSIX thread library for handling multiple clients.
- `socket`: For network communication.

## Authors

* [Ugo Warembourg](mailto:ugo.warembourg@student.junia.com)