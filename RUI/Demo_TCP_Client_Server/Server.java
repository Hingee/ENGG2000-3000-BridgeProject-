package RUI.Demo_TCP_Client_Server;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;

public class Server {
    static final int port = 9090;
    public static void main(String args[]) throws IOException {
        // create a server socket on port number 9090
        ServerSocket serverSocket = new ServerSocket(port);
        System.out.println("Server is running and waiting for client connection...");

        // Accept incoming client connection
        Socket clientSocket = serverSocket.accept();
        System.out.println("Client connected!");

        // Setup input and output streams for communication with the client
        BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
        PrintWriter out = new PrintWriter(clientSocket.getOutputStream(), true);

        while(true) {
            // Receive response from the server
            String response = in.readLine();
            System.out.println("Client says: " + response);
            if(response.equals("t")) {
                out.println("time");
            }else if(response.equals("QUIT")) {
                break;
            }else {
                out.println("OK");
            }
        }

        // Close the client socket
        clientSocket.close();
        // Close the server socket
        serverSocket.close();
    }
}
