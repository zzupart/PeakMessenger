#include <iostream>
#include <asio.hpp>
#include <vector>
#include <thread>

using namespace asio;

class Message{
    public:
        Message();
     private:
};

class Client{
    public:
        Client(int);
    public:
        bool connect_to(ip::tcp::endpoint arg_endpoint);
        void disconnect();
        void message_send(char* arg_msg);
        void start();
        void stop();
    private:
        void wait_for_connection();
        void message_wait();
    private:
        io_context m_context;
        std::thread m_context_thr;
        ip::tcp::socket m_socket;
        ip::tcp::acceptor m_acceptor;
    public:
        bool is_connected() const {
            return m_socket.is_open();
        }
};
Client::Client(int port)
    : m_socket(m_context), m_acceptor(m_context, ip::tcp::endpoint(ip::tcp::v4(), port)){}
void Client::start(){
    try{
        wait_for_connection();
        m_context_thr = std::thread([this](){ m_context.run(); });
    }
    catch(std::exception &excp){
        std::cout << "[~]| ERROR: " << excp.what() << "\n";
        exit(0);
    }
}
void Client::stop(){
    disconnect();
    m_context.stop();
    if(m_context_thr.joinable()) m_context_thr.join();
}
void Client::wait_for_connection(){
    m_acceptor.async_accept([this](std::error_code arg_error, ip::tcp::socket arg_socket){
            if(!arg_error){
                ip::address conn_ip = arg_socket.remote_endpoint().address();
                std::cout << "[~]| Got new connection request by " << conn_ip.to_string();
                std::cout << "\nAccept it? (y / n) ";
                char yn;
                while(yn != 'y' && yn != 'n'){
                    yn = getchar();
                    if(yn == 'y'){
                        m_socket = std::move(arg_socket);
                        std::cout << "[~]| Connection accepted\n";
                    }
                    if(yn == 'n'){
                        std::cout << "[~]| Connection declined\n";
                        arg_socket.close();
                    }
                }
            }
            else std::cout << "[~]| ERROR: " << arg_error.message() << std::endl;
            wait_for_connection();
        });
}
bool Client::connect_to(ip::tcp::endpoint arg_endpoint){
    m_socket.async_connect(arg_endpoint, [this](const std::error_code &ec){
                if(ec){ 
                    std::cout << "[~]| ERROR: " << ec.message() << "\n";
                    return false;
                }
                else{
                    message_wait();
                    return true;
                }
            });
}
void Client::disconnect(){
    if(m_socket.is_open()) m_socket.close();
}
void Client::message_send(char* arg_msg){}
void Client::message_wait(){}

int main(){
    int user_port;
    std::cout << "Enter your port: ";
    std::cin >> user_port;
    Client client(user_port);
    client.start();
    bool quit = false;
    while(!quit){
        std::cout << "> ";
        char* cmd = new char;
        std::cin >> cmd;
        if(!strcmp(cmd, (char*)"connect")){
            if(client.is_connected()){
                client.disconnect();
            }
            char* ip = new char;
            int port;
            std::cout << "Enter ip: ";
            std::cin >> ip;
            std::cout << "Enter port: ";
            std::cin >> port;
            ip::tcp::endpoint endp(ip::address::from_string(ip), port);
            if(client.connect_to(endp)){
                std::cout << "[~]| Connected to " << ip << "\n";
            }
            else{
                std::cout << "[~]| Failed to connect\n";
            }
            delete ip;
        }
        else if(!strcmp(cmd, (char*)"message")){
            if(client.is_connected()){
                char* msg = new char;
                std::cin >> msg;
                client.message_send(msg);
                delete msg;
            }
            else{
                std::cout << "[~]| Not connected\n";
            }
        }
        else if(!strcmp(cmd, (char*)"disconnect")){
            if(client.is_connected()){
                client.disconnect();
                std::cout << "[~]| Disconnected\n";
            }
            else{ 
                std::cout << "[~]| Not connected\n";
            }
        }
        else if(!strcmp(cmd, (char*)"exit")){
            std::cout << "[~]| Exiting\n";
            client.stop();
            quit = true;
        }
        else std::cout << "[~]| Unknown command\n";
        delete cmd;
    }

    return 0;
}
