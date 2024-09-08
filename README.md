# Client/Server Database System

## Description
This project implements a simple client/server database system using C and socket programming. The server stores and retrieves messages, functioning as a basic database system. Clients can connect to the server to send, retrieve, and manage messages.

## Features
- **Client-Server Communication:** The server handles multiple client connections using sockets.
- **Message Storage:** Clients can store messages on the server.
- **Message Retrieval:** Clients can request stored messages from the server.
- **Concurrent Clients:** The server can manage multiple clients concurrently.

## Usage
1. **Server:**
   - Start the server to begin listening for client connections.
   - The server will store and retrieve messages based on client requests.

2. **Client:**
   - Connect to the server.
   - Send commands to store or retrieve messages from the server.
