#include <bits/stdc++.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFF_SIZE 1024
#define PORT 10000
#define TRACKER1_PATH "/home/night-fury/OS/assignment_2/trackerfolder/"
#define TRACKER1 "tracker_info.txt"
#define USERS_DETAILS_PATH "/home/night-fury/OS/assignment_2/trackerfolder/"
#define USERS_FILE "user_details.txt"

using namespace std;

void * sendDetails(void * );
void * acceptRequest(void *);
vector <string> detailsFromPeer(string );
int checkLogin(string );
bool createUser(string );
void uploadDetails(vector <string> &);
void sendDetails(string, void *);
void sendFilesList(string, void *);

vector <string> detailsFromPeer(string input_data)
{
	vector <string> data_from_peer;
	int input_length = input_data.length();
	string word = "";
	for(int i=0; i<input_length; i++)
	{
		if(input_data[i] == ':')
		{
			data_from_peer.push_back(word);
			word = "";
		}
		else
			word += input_data[i];
	}
	data_from_peer.push_back(word);
	return data_from_peer;
}

int checkLogin(string temp_login)
{
	vector <string> details_login = detailsFromPeer(temp_login);
	string users_path = USERS_DETAILS_PATH;
	string users_file = USERS_FILE;
	string temp_file_path = users_path + users_file;
    string token(details_login[2]);
    ifstream file(temp_file_path);
    vector <string> stored_details;
    int flag = 0;
    string line1;
    if(file.is_open())
    {
        string line;
        while (getline(file, line))
        {
            if(line.find(token) != string::npos)
            {
            	flag = 1;
            	line1 = line;
  				break;
            }      
        }
        file.close();
    }	
    else
    	cout << "Could not open file !!! " << endl;
    if(flag == 1)
    {
    	stored_details = detailsFromPeer(line1);
    	if((stored_details[2].compare(details_login[2]) == 0))
    	{
    		if(stored_details[3].compare(details_login[3]) == 0)
    			return 0;
    		else
    			return 1;
    	}
    }
    else
    	return 2;
}

bool createUser(string temp_create_user)
{
	string users_path = USERS_DETAILS_PATH;
	string users_file = USERS_FILE;
	string temp_file_path = users_path + users_file;
	char file_path[temp_file_path.length()+1];
	strcpy(file_path, temp_file_path.c_str());
	FILE *fp = fopen(file_path, "a");
	string user_details;
	if(fp == NULL) 
	{
        cout << "Could not open file !!! " << endl;
        return false;
	}
    else 
    {
    	char create_user[temp_create_user.length() + 1];
    	strcpy(create_user, temp_create_user.c_str());
    	fprintf(fp, "%s\n", create_user);
	    fclose(fp);
	    return true;
	}
}

void * acceptRequest(void * temp_sockfd)
{
	int sockfd = *((int *)temp_sockfd);
	char data[BUFF_SIZE];
	recv(sockfd, data, sizeof(data), 0);
	string temp_data(data);
	vector <string> data_from_peer = detailsFromPeer(temp_data);
	if((data_from_peer[0].compare("upload")) == 0)
	{
		uploadDetails(data_from_peer);
	}
	if((data_from_peer[0].compare("download")) == 0)
	{
		string temp_down = data_from_peer[1] + ":" + data_from_peer[2]; 
		sendDetails(temp_down, (void *) temp_sockfd);
	}
	if((data_from_peer[0].compare("list_files")) == 0)
	{
		string group_id = data_from_peer[1];
		sendFilesList(group_id, (void *) temp_sockfd);
	}
	if((data_from_peer[0].compare("login")) == 0)
	{
		string temp_login = data_from_peer[1] + ":" + data_from_peer[2] + ":" + data_from_peer[3] + ":" + data_from_peer[4];
		if(checkLogin(temp_login) == 0)
		{
			char status[BUFF_SIZE] = "success";
			send(sockfd, status, sizeof(status), 0); 
		}
		else if(checkLogin(temp_login) == 1)
		{
			char status[BUFF_SIZE] = "password_wrong";
			send(sockfd, status, sizeof(status), 0); 
		}
		else
		{
			char status[BUFF_SIZE] = "not_exists";
			send(sockfd, status, sizeof(status), 0); 
		}
	}
	if((data_from_peer[0].compare("create_user")) == 0)
	{
		string temp_create_user = data_from_peer[1] + ":" + data_from_peer[2] + ":" + data_from_peer[3] + ":" + data_from_peer[4];
		if(createUser(temp_create_user))
		{
			char status[BUFF_SIZE] = "success";
			send(sockfd, status, sizeof(status), 0);
		}
		else
		{
			char status[BUFF_SIZE] = "failed";
			send(sockfd, status, sizeof(status), 0);
		}
	}
	close(sockfd);
}

void uploadDetails(vector <string> &data_from_peer)
{
	string tracker1_path = TRACKER1_PATH;
	string tracker1 = TRACKER1;
	string temp_file_path = tracker1_path + tracker1;
	string temp_token = data_from_peer[1] + ":" + data_from_peer[2] + ":" + data_from_peer[3] + ":" + data_from_peer[4] + ":";
	ifstream file(temp_file_path);
	int flag = 0;
    if(file.is_open())
    {
        string line;
        while (getline(file, line))
        {
            if(line.find(temp_token) != string::npos)
            {
              	flag = 1;
              	break;
            }      
        }
        file.close();
    }	
    else
    	cout << "Could not open file !!! " << endl;
    if(flag == 0)
    {
		char file_path[temp_file_path.length()+1];
		strcpy(file_path, temp_file_path.c_str());
		FILE *fp = fopen(file_path, "a");
		string upload_details;
		if(fp == NULL) 
	        cout << "Could not open file !!! " << endl;
	    else 
	    {
		    for(int i=1; i<data_from_peer.size()-1; i++)
		    	upload_details += data_from_peer[i] + ":";
		    upload_details += data_from_peer[data_from_peer.size()-1];	 
		    char upload_details_final[upload_details.length()+1];
		    strcpy(upload_details_final, upload_details.c_str());
		    fprintf(fp, "%s\n", upload_details_final);
		    fclose(fp);
		}
	}
}


void sendDetails(string file_name, void * temp_sockfd)
{
	int sockfd = *((int *)temp_sockfd);
	string tracker1_path = TRACKER1_PATH;
	string tracker1 = TRACKER1;
	string temp_file_path = tracker1_path + tracker1;
    string token(file_name);
    ifstream file(temp_file_path);
    if(file.is_open())
    {
        string line;
        while (getline(file, line))
        {
            if(line.find(token) != string::npos)
            {
              	char buffer[BUFF_SIZE];
               	strcpy(buffer, line.c_str());
               	send(sockfd, buffer, BUFF_SIZE, 0);
               	memset(buffer, '\0', BUFF_SIZE);
            }      
        }
        file.close();
    }	
    else
    	cout << "Could not open file !!! " << endl;
}

void sendFilesList(string group_id, void * temp_sockfd)
{
	int sockfd = *((int *)temp_sockfd);
	string tracker1_path = TRACKER1_PATH;
	string tracker1 = TRACKER1;
	string temp_file_path = tracker1_path + tracker1;
	string temp_group_id = ":" + group_id + ":";
	string token(temp_group_id);
    ifstream file(temp_file_path);
    if(file.is_open())
    {
        string line;
        while (getline(file, line))
        {
            if(line.find(token) != string::npos)
            {
              	char buffer[BUFF_SIZE];
               	strcpy(buffer, line.c_str());
               	send(sockfd, buffer, BUFF_SIZE, 0);
               	memset(buffer, '\0', BUFF_SIZE);
            }      
        }
        file.close();
    }	
    else
    	cout << "Could not open file !!! " << endl;
}

int main()
{
	int server_fd;
	if((server_fd = socket (AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed"); 
	    exit(EXIT_FAILURE);
	}
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons( PORT );
	address.sin_addr.s_addr = INADDR_ANY;
	//address.sin_addr.s_addr = inet_addr("127.0.0.1");
	int addrlen = sizeof(sockaddr);
	if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("bind failed"); 
        exit(EXIT_FAILURE);
	}

	if(listen(server_fd, 10) < 0)
	{
		perror("listen failed"); 
	    exit(EXIT_FAILURE);
	}
	while(true)
	{
		int sockfd;
		if((sockfd = accept( server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
		{
			perror("accept failed");
			exit(EXIT_FAILURE);
		}
		pthread_t thread1;
		int *sock = &sockfd;
		if(pthread_create(&thread1, NULL, acceptRequest, (void *) sock) != 0)
		{
			cout << "Thread creation failed !!!" << endl;
        	if(pthread_detach(thread1) != 0)
            	cout << "Thread detach failed !!!" << endl;
		}
	}
	close(server_fd);
	return 0;
}	