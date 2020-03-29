#include <bits/stdc++.h>
#define INF 1e7
using namespace std;

class Value{
public:
        string next_hop;
        int cost;
        Value(string next_hop, int cost)
        {
                this->next_hop = next_hop;
                this->cost = cost;
        }
};

class RoutingTable{
public:
        map < string, Value* > routing_table;                     //dest, pair<next_hop, cost>
        map < string, int > last_found_clock;                     //neighbour, last found clock
        map < string, int > neighbours;                           //neighbour, cost
        map < string, int > down_neighbours;                      //down neighbours, last known cost from my_ip
        string my_ip;
        int last_clock = 0;

        RoutingTable()
        {

        }
        RoutingTable(string my_ip)
        {
                this->my_ip = my_ip;
        }
        string get_next_hop(string dest)
        {
                return routing_table[dest]->next_hop;
        }
        void print_routing_table()
        {
                const char separator    = ' ';
                const int nameWidth     = 16;
                const int numWidth      = 8;
                cout << "\n\nRouting table\n";
                cout << left << setw(nameWidth) << setfill(separator) << "destination";
                cout << left << setw(nameWidth) << setfill(separator) << "next hop";
                cout << left << setw(numWidth) << setfill(separator) << "cost";
                cout << "\n";
                for(auto it : routing_table)
                {
                        cout << left << setw(nameWidth) << setfill(separator) << it.first;
                        cout << left << setw(nameWidth) << setfill(separator) << it.second->next_hop;
                        cout << left << setw(numWidth) << setfill(separator) << it.second->cost;
                        cout << "\n";
                }
                cout << "\n";
        }
        void populate_routing_table(string st1, string st2, int cost)
        {
                if(st1 == my_ip)
                {
                        neighbours[st2] = cost;
                        routing_table[st2] = new Value(st2, cost);
                }
                else if(st2 == my_ip)
                {
                        neighbours[st1] = cost;
                        routing_table[st1] = new Value(st1, cost);
                }
                else
                {
                        if(neighbours.find(st1) == neighbours.end())
                                routing_table[st1] = new Value("-", INF);
                        if(neighbours.find(st2) == neighbours.end())
                                routing_table[st2] = new Value("-", INF);
                }
        }
        void update_cost(string str, int new_cost)         //str is my neighbour
        {
                int prev_cost = neighbours[str];
                neighbours[str] = new_cost;

                for(auto it : routing_table)
                {
                        if(it.second->next_hop == str) // kono ek dest er jonno next_hop == str
                        {
                                routing_table[it.first] = new Value(str, it.second->cost - prev_cost + new_cost);
                        }
                }
        }
        string routing_table_to_string()
        {
                string temp("rt ");
                for(auto it : routing_table)
                {
                        temp += it.first;
                        temp += " ";
                        temp += it.second->next_hop;
                        temp += " ";
                        temp += to_string(it.second->cost);
                        temp += " ";
                }
                return temp;
        }
        void update_routing_table(const char* str, const char* sender)
        {
                string __sender(sender);
                string temp(str);
                istringstream is(temp);

                int cost_sender_to_me = neighbours[__sender];

                for(int i = 0; i < routing_table.size(); i++)
                {
                        string dest, next_hop;
                        string __cost;
                        is >> dest >> next_hop >> __cost;

                        int cost = stoi(__cost);

                        int cost_through_this_neighbour = cost_sender_to_me + cost;

                        if( ((cost_through_this_neighbour < routing_table[dest]->cost)  &&  (my_ip != dest))//split horizon
                        ||  (routing_table[dest]->next_hop == __sender) )                                   //forced update

                        {
                                routing_table[dest] = new Value(__sender, cost_through_this_neighbour);
                        }
                }
        }
        void update_down_neighbours(const char *__ip)
        {
                string ip(__ip);
                last_found_clock[ip] = last_clock;

                //it was previously down
                if(down_neighbours.find(ip) != down_neighbours.end())
                {
                        cout << "link between " << my_ip << " & " << ip << " is now up\n";
                        int last_cost = down_neighbours[ip];
                        down_neighbours.erase(down_neighbours.find(ip));
                        Value *temp = routing_table[ip];
                        routing_table[ip] = new Value(ip, last_cost);
                        delete temp;
                }
        }
        void find_down_neighbours()
        {
                for(auto it : last_found_clock)
                {
                        if( (last_clock - it.second >= 3)   &&  (down_neighbours.find(it.first) == down_neighbours.end()))
                        {
                                //this neighbour is down, append in down_neighbours
                                down_neighbours[it.first] = neighbours[it.first];
                                cout << "link between " << my_ip << " & " << it.first << " is now down\n";

                                //routing_table update korte hobe
                                for(auto it2 : routing_table)
                                {
                                        //this neighbour is someone's next_hop
                                        if(it2.second->next_hop == it.first)
                                        {
                                                routing_table[it2.first] = new Value("-", INF);
                                        }
                                }
                        }
                }
        }
};
