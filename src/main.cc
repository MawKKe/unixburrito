
#include <iostream>
#include <string>
#include <vector>

#include <csignal>

#include <unix/inet.hpp>
#include <unix/signals.hpp>

volatile std::sig_atomic_t run = true;

void signalHandler(int w_sig __attribute__ ((unused)))
{
    run = false;
}

/*
int sigsetup(){
    using namespace unix;
    SigAction sa = SigAction::emptySet();
    sa.set_handler(signalHandler);
    sa.mask_add(Signal::User1);
    auto sig = Signal::Interrupt;
    if(unix::sigaction(sig, sa) < 0){
        std::cerr << "sigaction(): " << unix::errno_str(errno) << "\n";
        return -1;
    }
    std::cerr << "INFO: signal handler set up for " << unix::to_string(sig) << "\n";
    return 0;
}
*/

int server(int argc, const char* argv[]){

    if(argc < 3){
        std::cerr << "usage: <address or name> <port>\n";
        return -1;
    }

    auto h   = std::string(argv[1]);
    auto srv = std::string(argv[2]);


    auto _s = unix::inet::server_socket_udp(h, srv);

    if(!_s){
        std::cout << "Error opening socket..." << std::endl;
        return -1;
    }

    std::cout << "----------------------------------------\n";

    unix::inet::Socket & s = *_s;

    std::cout << "server bound to: \n" << s.getsockname() << std::endl;

    std::cout << "----------------------------------------\n";

    uint8_t buf[9000];

    while(run){
        auto p = s.recvfrom(buf, sizeof(buf), {});

        ssize_t n = p.first;
        std::cout << "Receive return: " << n << std::endl;

        if(n < 0){
            std::cerr << "recv(): " << unix::errno_str(errno) << std::endl;
        }

        else{
            std::cerr << "from: " << p.second << std::endl;
            std::cerr << "bytes: " << n << std::endl;
        }
    }
	std::cerr << "Exiting...";
    return 0;
}

int client(int argc, const char* argv[]){
    return -1;
}


int main(int argc, const char *argv[])
{
	if(unix::signals::handleInterrupt(signalHandler) < 0){
		std::cerr << "handleInterrupt() failed, exiting..\n";
		exit(1);
	}

    std::string progname(argv[0]);

    std::cout << "progname: " << progname << std::endl;

    if(cpp::element_in(progname, {"server", "./server"})){
        return server(argc, argv);
    }
    else if(cpp::element_in(progname, {"client", "./client"})){
        return client(argc, argv);
    }
    else{
        std::cout << "unknown progname: " << progname << "\n";
        exit(1);
    }

    return 0;
}
