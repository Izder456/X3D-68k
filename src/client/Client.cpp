// This file is part of X3D.
//
// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#include "Client.hpp"
#include "error/Log.hpp"
#include "engine/Engine.hpp"

void x_client_init(X_Client* client)
{
    client->connectedToServer = 0;
    client->currentTransfer.flags = (X_FileFlags)0;
}

static void send_connect_request(X_Socket* socket)
{
    X_Packet packet;
    char buf[1];
    x_packet_init(&packet, X_PACKET_CONNECT, buf, 0);
    x_socket_send_packet(socket, &packet);
}

bool x_client_connect(X_Client* client, const char* address)
{
    client->connectedToServer = x_socket_open(&client->socket, address);
    
    if(client->connectedToServer)
        send_connect_request(&client->socket);
    
    return client->connectedToServer;
}

void x_client_request_file(X_Client* client, const char* name)
{
    char packetBuf[256];
    X_Packet requestPacket;
    x_packet_init_empty(&requestPacket, X_PACKET_REQUEST_FILE, packetBuf);
    x_packet_write_str(&requestPacket, name);
    
    x_socket_send_packet(&client->socket, &requestPacket);
}

static void handle_request_file_response(X_Client* client, X_Packet* packet)
{
    bool success = x_packet_read_byte(packet);
    if(!success)        // TODO: how to handle a failed transfer?
        return;
    
    client->transferSize = x_packet_read_int(packet);
    client->transferTotalReceived = 0;
    
    char fileName[X_FILENAME_MAX_LENGTH];
    x_packet_read_str(packet, fileName);
    
    x_file_open_writing(&client->currentTransfer, fileName);
    
    x_console_printf(x_engine_get_console(), "Receiving file %s (%d bytes)...\n", fileName, client->transferSize);
}

static bool transfer_is_active(X_Client* client)
{
    return x_file_is_open(&client->currentTransfer);
}

static void handle_file_transfer_chunk(X_Client* client, X_Packet* packet)
{
    if(!transfer_is_active(client))
        return;
    
    x_file_write_buf(&client->currentTransfer, packet->size, packet->data);
    client->transferTotalReceived += packet->size;
    
    x_console_printf(x_engine_get_console(), "total %d left %d!\n", client->transferSize, client->transferTotalReceived);
    
    if(client->transferTotalReceived == client->transferSize)
    {
        x_console_printf(x_engine_get_console(), "File received!\n");
        x_file_close(&client->currentTransfer);
    }
}

static void handle_packets(X_Client* client)
{
    X_Packet* packet;
    while((packet = x_socket_receive_packet(&client->socket)) != NULL)
    {
        x_console_printf(x_engine_get_console(), "Has packet: %d! size %d\n", packet->type, packet->size);
        switch(packet->type)
        {
            case X_PACKET_REQUEST_FILE_RESPONSE:
                handle_request_file_response(client, packet);
                break;
                
            case X_PACKET_FILE_CHUNK:
                handle_file_transfer_chunk(client, packet);
                break;
                
            default:
                break;
        }
    }
}

void x_client_update(X_Client* client)
{
    if(!client->connectedToServer || !x_socket_connection_is_valid(&client->socket))
        return;
    
    handle_packets(client);
}
