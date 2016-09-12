#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <pthread.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int fd;
int port_number;
int stop = 1;
pthread_mutex_t lock1;
vector<double> temps;
string dist;
int curr_temp_pos = 0; // if larger than 3600, reset to 0
int full = 0;
char message[100];
int sensor_connected = 0;
bool T_sensor = true;


/*
 * This function reads data from the serial port and stores data respectively
 */
void* readData(void* p){
    char buf[100]; //read buffer
    int buffer_loc = 0;
    char msg[200];
    struct termios options; // struct to hold options
    tcgetattr(fd, &options); // associate with this fd
    cfsetispeed(&options, 9600); // set input baud rate
    cfsetospeed(&options, 9600); // set output baud rate
    tcsetattr(fd, TCSANOW, &options); // set options 
    // keep looping while user want the whole server to be stoped
    while(stop == 1){
        int bytes_read = read(fd, buf, 100);
        buf[bytes_read] = '\0';
        buf[bytes_read] = '\0';
        if(bytes_read > 0){
            for(int i=0;i<bytes_read;i++){
                if(buf[i]!='\n') {
                    msg[buffer_loc++] = buf[i]; 
                }
                else{
                    msg[buffer_loc]='\0';
                    // pthread_mutex_lock(&lock1);
                    if (T_sensor) {
                        double temp = atof(msg);
                        pthread_mutex_lock(&lock1);
                        if (full < 3600) {
                        	full++;
                        	if (full < 2) {
                        		pthread_mutex_unlock(&lock1);
                                buffer_loc = 0;
                        		continue;
                        	}
                        	temps.push_back(temp);
                        	curr_temp_pos++;
                        } else {
                        	if (curr_temp_pos >= 3600) {
                        		curr_temp_pos = 0;
                        	}
                        	temps[curr_temp_pos] = temp;
                        	curr_temp_pos++;
                        }
                        pthread_mutex_unlock(&lock1);
                        cout << "get temp = " << temp << endl;
                        buffer_loc =0;
                    }
                    else {
                        string message(msg);
                        dist = message;
                        cout << "distance = " << dist << endl;
                        buffer_loc =0;
                    }
                }
            }
        }
        else if (sensor_connected == 1) {
            sensor_connected = 0;
        }
    }
    return 0;
}

/*
 * This function return the most recently read temperature data
 */
double currTemp() {
	pthread_mutex_lock(&lock1);    
	double temp = curr_temp_pos == 0 ? -1 : temps[curr_temp_pos - 1]; 
	pthread_mutex_unlock(&lock1);
	return temp;
}

/*
 * This function return the average temperature from at most past one hour
 */
double avgTemp() {
	double total = 0;
	int count = 0;
	vector<double>::iterator it = temps.begin();
	while(it != temps.end()) {
		total += *it;
		count++;
		it++;
	}
	return total/count;
}

/*
 * This function return the highest temperature from at most past one hour
 */
double highestTemp() {
	double max = 0;
	vector<double>::iterator it = temps.begin();
	while(it != temps.end()) {
		double curr = *it;
		if (curr > max) {
			max = curr;
		}
		it++;
	}
	return max;
}

/*
 * This function return the lowest temperature from at most past one hour
 */
double lowestTemp() {
	double min = 200;
	vector<double>::iterator it = temps.begin();
	while(it != temps.end()) {
		double curr = *it;
		if (curr < min) {
			min = curr;
		}
		it++;
	}
	return min;
}

/*
 * This function calculate the temperature in Fahrenheit by providing the temperature in Celsius
 */
double CToF(double cTemp) {
    return cTemp * 1.8 + 32.0;
}

/*
 * This function is a helper function to attach the right message we want to send to pebble
 */
void send_msg(bool isC, string& reply, string addmsg, double temperature) {
	string temp;
	if (isC) {
		temp = to_string(temperature);
	} else {
		temp = to_string(CToF(temperature));
	}
	reply += "{\n\"name\": \"";
	reply += addmsg;
	reply += temp;
	reply += isC ? " °C" : " °F";
	reply += "\"\n}\n";
}

/*
 * This function starts the server
 */
void* start_server(void* p) {

	// structs to represent the server and client
	struct sockaddr_in server_addr, client_addr;
	int sock;
	// 1. socket: creates a socket descriptor that you later use to make other system calls
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}
	int temp;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(int)) == -1) {
		perror("Setsockopt");
		exit(1);
	}
	// configure the server
	server_addr.sin_port = htons(port_number); // specify port number
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server_addr.sin_zero),8);

	// 2. bind: user the socket and associate it with the portnumber
	if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
		perror("Unable to bind");
		exit(1);
	}

	// 3. listen: indicates that we want to listen to the port to which we bound; second arg is number of allowed connections
	if (listen(sock, 5) == -1) {
		perror("Listen");
		exit(1);
	}

	// once you get here, the server is set up and about to start listening
	cout << "Server configured to listen on port " << port_number << endl;
	fflush(stdout);

	// 4. accept: wait here until we get a connection on that port
	int sin_size = sizeof(struct sockaddr_in);

	bool isCelsius = true;
	bool isStop = false;
    // keep looping while user want the whole server to be stoped
	while (stop == 1) {
		int s_fd = accept(sock, (struct sockaddr *)&client_addr,(socklen_t *)&sin_size);
        printf("Server got a connection from (%s, %d)\n", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
      
        // buffer to read data into
        char request[1024];
      
        // 5. recv: read incoming message into buffer
        int bytes_received = recv(s_fd,request,1024,0);
        if (bytes_received == 0) {
            close(s_fd);
            continue;
        }
        // null-terminate the string
        request[bytes_received] = '\0';
        printf("Here comes the message:\n");
        printf("%s\n", request);
        
        // string reply = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
        string reply = "";
        if (sensor_connected == 1) {
            char* token = strtok(request, " ");
            token = strtok(NULL, " ");
            string req(token);
            // cout << "******request = " << req << endl;
            if (req == "/current") {
            	send_msg(isCelsius, reply, "Current temperature is ", currTemp());
            	// cout << "********sendmsg = " << reply << endl;
            }
            else if (req == "/average") {
            	send_msg(isCelsius, reply, "Average temperature is ", avgTemp());
            }
            else if (req == "/high") {
            	send_msg(isCelsius, reply, "The highest temperature is ", highestTemp());
            }
            else if (req == "/low") {
            	send_msg(isCelsius, reply, "The lowest temperature is ", lowestTemp());
            }
            else if (req == "/Celsius") {
            	isCelsius = true;
            	reply += "{\n\"name\": \"Now temperature is in Celsius.\"\n}\n";
            	write(fd, "c", 1);
            }
            else if (req == "/Fahrenheit") {
            	isCelsius = false;
            	reply += "{\n\"name\": \"Now temperature is in Fahrenheit.\"\n}\n";
            	write(fd, "f", 1);
            }
            else if (req == "/stop") {
            	isStop = true;
            	reply += "{\n\"name\": \"Stop temperature reporting.\"\n}\n";
                write(fd, "s", 1);
            }
            else if (req == "/resume") {
            	isStop = false;
            	reply += "{\n\"name\": \"Resume temperature reporting.\"\n}\n";
                write(fd, "r", 1);
            }
            else if (req == "/proximity") {
                isStop = false;
                T_sensor = false;
                reply += "{\n\"name\": \"Read data from proximity sensor.\"\n}\n";
                write(fd, "p", 1);
            }
            else if (req == "/distance") {
                isStop = false;
                reply += "{\n\"name\": \"Distance is ";
                reply += dist;
                reply += "\"\n}\n";
            }
            else if (req == "/temp") {
                isStop = false;
                T_sensor = true;
                reply += "{\n\"name\": \"Read data from temperature sensor.\"\n}\n";
                write(fd, "t", 1);
            } else {
                isStop = false;
                isCelsius = true;
                cout << "req = " << req << endl;
                string temp = req.substr(18);
                cout << "temp = " << temp << endl;
                reply += "{\n\"name\": \"hi weather!\"\n}\n";
                write(fd, temp.c_str(), temp.length());
            }
            // 6. send: send the message over the socket
            // note that the second argument is a char*, and the third is the number of chars
            if (isStop) {
            	reply = "{\n\"name\": \"Stop temperature reporting.\"\n}\n";
            }
        }
        else {
            reply = "{\n\"name\": \"Sensor is disconnected.\"\n}\n";
        }
        cout << "sendmsg = " << reply << endl;
        send(s_fd, reply.c_str(), reply.length(), 0);
        // 7. close: close the socket connection
        close(s_fd);
	}
    close(sock);
    cout << "Server closed connection" << endl;
    return 0;
}

/*
 * this function allows user to stop server by entering 'q' in command line
 */
void* stop_server (void* p) {
  string command;
  cout << "Enter q to stop server" << endl;
  while(1) {
    cin >> command;
    if (command == "q") {
      stop = 0;
      break;
    }
  }
  return 0;
}

/*
 * The main function to run
 */
int main(int argc, char *argv[]) {

	// check the number of arguments
	if (argc != 3) {
		cout << "Usage: server [port_number] [arduino_file_path]" << endl;
		return -1;
	}
	try {
		port_number = stoi(argv[1]);
	} catch (exception& e) {
		cout << "Invalid port number" << endl;
	}
	if (port_number <= 1024) {
		cout << "Please specify a port number greater than 1024" << endl;
		return -1;
	}
	// open the file
	fd = open(argv[2], O_RDWR);
	if (fd == -1) {
		cout << "Open file failed, Arduino is not connected!" << endl;
		return -1;
	} else {
        sensor_connected = 1;
    }

	// pthread_mutex_init(&lock2, NULL);
    pthread_mutex_init(&lock1, NULL);

	pthread_t s, g, stop;
    pthread_create(&g, NULL, &readData, NULL);
    pthread_create(&s, NULL, &start_server, NULL);
    pthread_create(&stop, NULL, &stop_server, NULL);
    pthread_join(g, NULL);
    pthread_join(s, NULL);
    pthread_join(stop, NULL);
    close(fd);

	return 0;
}

















