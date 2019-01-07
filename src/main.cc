
#include <iostream>
#include <string>
#include <vector>

#include <csignal>

#include <unix/inet.hpp>
#include <unix/signals.hpp>

#include <unix/epoll.hpp>

// Kinda like in python you say "import Foo as bar'
namespace unix = _unix;

volatile std::sig_atomic_t run = true;

void signalHandler(int w_sig __attribute__ ((unused)))
{
    run = false;
}

void handle_in(unix::inet::Socket & s){
    uint8_t buf[9000];

    auto p = s.recvfrom(buf, sizeof(buf), {});

    ssize_t n = p.first;
    std::cout << "Receive return: " << n << std::endl;

    if(n < 0){
        std::cerr << "recv(): " << unix::errno_str(errno) << std::endl;
    }

    else{
        std::cerr << "from:  " << *p.second << std::endl;
        std::cerr << "bytes: " << n << std::endl;
        std::cerr << "data:  " << std::string((const char*)buf, n) << "\n";
        std::reverse(std::begin(buf), std::begin(buf)+n-1);

        ssize_t n2 = s.sendto(buf, n, *p.second, {unix::inet::SendFlag::DontWait});

        if(n2 < 0){
            std::cerr << "sendto(): " << unix::errno_str(errno) << std::endl;
        }
        if(n2 != n){
            std::cerr << "omg sendto has problems, gosh: " << std::to_string(n2) << "\n";
        }
    }
    std::cerr << "---\n";

}

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

	s.listen(100);

    using namespace _unix::epoll;

    auto epoll = Epoll();

    epoll.add(s, {EpollEventType::Input, EpollEventType::EdgeTrigger});

    using namespace std::chrono_literals;

    while(run){
        //std::cerr << "DEBUG: waiting..\n";
        EventList<10> evts;
        auto n_ev = epoll.wait(evts, 500ms);
        if(n_ev < 0){
            // TODO: could be a non-error value, such as timeout expiration
            std::cerr << "ERROR - Epoll::wait(): " << unix::errno_str(errno) << std::endl;;
            break;
        }
        if(n_ev > 0){
            std::cerr << "epoll_wait returned: " << n_ev << std::endl;
        }
        for(int i = 0; i < n_ev; ++i){
            if(evts[i].matches(s) && (evts[i] & EpollEventType::Input)){
                handle_in(s);
            }
            else{
                std::cerr << "Unknown socket or event type" << std::endl;
            }
        }

    }
	std::cerr << "Exiting...";
    return 0;
}

int client(int argc, const char* argv[]){
	if(argc < 3){
		std::cerr << "usage: <address or name> <port>\n";
		return -1;
	}

	auto h   = std::string(argv[1]);
	auto srv = std::string(argv[2]);

	auto _s = unix::inet::client_socket_any(h, srv);

	if(!_s){
		std::cout << "Error opening socket..." << std::endl;
		return -1;
	}

	unix::inet::Socket & s = *_s;

	std::cout << "----------------------------------------\n";
	std::cout << "client connected to: \n" << s.getpeername() << std::endl;
	std::cout << "----------------------------------------\n";

	std::string input;

	while(run){
		std::getline(std::cin, input);

		auto p = s.send(input);

		if(p < 0){
			std::cerr << "ERROR send(): " << unix::errno_str(errno) << std::endl;
			continue;
		}
		if(p != (ssize_t)input.size()){
			std::cerr << "WARNING: send() returned " << p << ", requested: " << input.size() << std::endl;
			continue;
		}

	}
	std::cerr << "Exiting...";
	return 0;
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
