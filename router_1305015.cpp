#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <bits/stdc++.h>

#include "routing_table_1305015.cpp"

#define MAX_NEIGHBOURS 10
#define INF 1e7
#define BUFFER_LENGTH 1024

using namespace std;


string my_ip;
int last_clock = 0;


int sockfd;
int bind_flag;
socklen_t addrlen;
int sent_bytes;
int bytes_received;
char buffer[BUFFER_LENGTH];
struct sockaddr_in my_address;
struct sockaddr_in receiver;
struct sockaddr_in sender;


RoutingTable *my_routing_table;


char* substr(char* arr, int begin, int len)
{
    char* res = new char[len];
    for (int i = 0; i < len; i++)
        res[i] = *(arr + begin + i);
    res[len] = 0;
    return res;
}
char* get_message(char *arr)
{
        int len = strlen(arr);
        char *res = new char[len];
        for(int i = 0; i < len; ++i)
                res[i] = (char) (arr[i]);
        return res;
}

void send_routing_table()
{
        string temp = my_routing_table->routing_table_to_string();

        receiver.sin_family = AF_INET;
	receiver.sin_port = htons(4747);

	for(auto it : my_routing_table->neighbours)
	{
                inet_pton(AF_INET,it.first.c_str(),&receiver.sin_addr);

                sent_bytes=sendto(sockfd, temp.c_str(), BUFFER_LENGTH, 0, (struct sockaddr*) &receiver, sizeof(sockaddr_in));
	}
}

void send_hello_packet(const char *dest, const char *message, int message_len)
{
        string temp("frwd ");
        temp += dest;
        temp += " ";
        temp += to_string(message_len);
        temp += " ";
        temp += message;

        cout << message_len << " " << message << "\n";

        receiver.sin_family = AF_INET;
	receiver.sin_port = htons(4747);

	string __dest(dest);
	string next_hop = my_routing_table->get_next_hop(__dest);
        if(next_hop == "-")
        {
                printf("%s packet could not been forwarded\n", message);
                return;
        }
	inet_pton(AF_INET,next_hop.c_str(),&receiver.sin_addr);

	sent_bytes=sendto(sockfd, temp.c_str(), BUFFER_LENGTH, 0, (struct sockaddr*) &receiver, sizeof(sockaddr_in));

	printf("%s packet forwarded to %s (printed by %s)\n", message, next_hop.c_str(), my_ip.c_str());
}

void forward_message(char *buf)
{
        string temp(buf);
        istringstream is(temp);

        string dest, message_length_as_string, message;
        is >> dest >> message_length_as_string;
        getline(is, message);
        if(message[0] == ' ')
        {
                message = message.substr(1, message.size() - 1);
        }

        if(dest == my_ip)
        {
                printf("%s packet reached destination (printed by %s)\n", message.c_str(), my_ip.c_str());
        }
        else
        {
                send_hello_packet(dest.c_str(), message.c_str(), stoi(message_length_as_string));
        }
}

void read()
{
        printf("read started\n");
        while(true)
        {
                memset(buffer, '\0', sizeof(buffer));
                bytes_received = recvfrom(sockfd, buffer, BUFFER_LENGTH, 0, (struct sockaddr*) &sender, &addrlen);

                //from driver.py
                if(strcmp(substr(buffer, 0, 3), "clk") == 0)          //send routing table to neighbours
                {
                        //update global clock
                        last_clock = atoi(substr(buffer, 4, bytes_received - 4));
                        my_routing_table->last_clock = last_clock;

                        send_routing_table();

                        my_routing_table->find_down_neighbours();
                }
                else if(strcmp(substr(buffer, 0, 4), "show") == 0)      //show ip
                {
                        //parsing buffer
                        char ar[BUFFER_LENGTH];
                        inet_ntop(AF_INET, buffer + 4, ar, sizeof(ar));

                        //actual work
                        my_routing_table->print_routing_table();
                }
                else if(strcmp(substr(buffer, 0, 4), "cost") == 0)
                {
                        //parsing buffer
                        char ip1[BUFFER_LENGTH], ip2[BUFFER_LENGTH], cost[BUFFER_LENGTH];
                        inet_ntop(AF_INET, buffer + 4, ip1, sizeof(ip1));
                        inet_ntop(AF_INET, buffer + 8, ip2, sizeof(ip2));
                        inet_ntop(AF_INET, buffer + 12, cost, sizeof(cost));


                        //actual work
                        string s1(ip1);
                        string s2(ip2);
                        if(s1 == my_ip)
                                my_routing_table->update_cost(s2, atoi(cost));
                        else if(s2 == my_ip)
                                my_routing_table->update_cost(s1, atoi(cost));

                        my_routing_table->print_routing_table();//eta lagbe na asole
                }
                else if(strcmp(substr(buffer, 0, 4), "send") == 0)
                {
                        //parsing buffer
                        char ip1[BUFFER_LENGTH], ip2[BUFFER_LENGTH], cost[BUFFER_LENGTH];
                        inet_ntop(AF_INET, buffer + 4, ip1, sizeof(ip1));
                        inet_ntop(AF_INET, buffer + 8, ip2, sizeof(ip2));


                        int message_len = bytes_received - 14;
                        char *message = substr(buffer, 14, message_len);
                        send_hello_packet(ip2, message, message_len);
                }
                //from other routers
                else if(strcmp(substr(buffer, 0, 2), "rt") == 0)
                {
                        char ip[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &(sender.sin_addr), ip, INET_ADDRSTRLEN);

                        //update this ip's clock
                        my_routing_table->update_down_neighbours(ip);


                        my_routing_table->update_routing_table(buffer + 2, ip);
                }
                else if(strcmp(substr(buffer, 0, 4), "frwd") == 0)
                {
                        forward_message(substr(buffer, 5, bytes_received - 5));
                }
        }
}

void configure_myself(char *address)
{
        my_address.sin_family = AF_INET;
	my_address.sin_port = htons(4747);
	inet_pton(AF_INET,address,&my_address.sin_addr);
	my_ip = address;
	cout << "my ip address is : " << my_ip << "\n";


	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	bind_flag = bind(sockfd, (struct sockaddr*) &my_address, sizeof(sockaddr_in));
	if(bind_flag==0)printf("successful bind\n");
}

void configure_all_routers()
{
        ifstream fin;
        fin.open("topo.txt");
        string st1, st2;
        int cost;
        while(fin >> st1 >> st2 >> cost)
        {
                my_routing_table->populate_routing_table(st1, st2, cost);
        }
        my_routing_table->routing_table[my_ip] = new Value("-", 0);
        fin.close();
}



int main(int argc, char *argv[])
{
        cout << argc << endl;

        if(argc != 2){
		cout << argv[0] << " <ip address>\n";
		exit(1);
	}

        configure_myself(argv[1]);
        my_routing_table = new RoutingTable(my_ip);
        configure_all_routers();
        my_routing_table->print_routing_table();
        read();
        while(true);
        return 0;
}
