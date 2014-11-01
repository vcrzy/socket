#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x501

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "8080"

int __cdecl main(int argc, char **argv)
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    char *sendbuf = "Testando envio de mensagem para o servidor... ";
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;

    // Validando parâmetros
    if (argc != 2) {
        printf("usage: %s Cliente conectado - OK\n", argv[0]);
        return 1;
    }

    // Inicializando Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup falhou, error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Definindo o hostname e a porta
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("Getaddrinfo falhou, error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Tentando conectar ao endereço até que seja bem sucedido
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Criando o socket para conectar ao servidor
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("Socket falhou, error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Conectando ao servidor
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Não foi possível conectar ao servidor!\n");
        WSACleanup();
        return 1;
    }

    // Enviando os dados
    iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
    if (iResult == SOCKET_ERROR) {
        printf("Falha no envio, error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    printf("Bytes enviados: %ld\n", iResult);

    // Finalizando a conexão a não ser que tenha mais dados para serem enviados
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("Falha na finalização, error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Receber até o peer fechar a conexão
    do {

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 )
            printf("Bytes rebidos: %d\n", iResult);
        else if ( iResult == 0 )
            printf("Conexão foi fechada!\n");
        else
            printf("Falha no recebimento, error: %d\n", WSAGetLastError());

    } while( iResult > 0 );

    // Limpando
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}
