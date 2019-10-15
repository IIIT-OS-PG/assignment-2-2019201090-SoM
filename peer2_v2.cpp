#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <pthread.h>

#define PART_SIZE 524288
#define BUFF_SIZE 1024
#define TRACKER1_IP "127.0.0.1"
#define TRACKER1_PORT 10000
//#define PORT 5002
//#define IP "127.0.0.1"
//#define FILE_PATH "/home/night-fury/OS/assignment_2/peer2_file/"

typedef long long int ll;
const char * IP;
int PORT;

using namespace std;

pthread_mutex_t lock1;

struct thread_download_args
 {
 	string info;
}; 

bool sort1(pair <string, vector <int>> &x, pair <string, vector <int>> &y)
{
	return x.second.size() < y.second.size();
}

vector <string> processedString(string ,char );
ll getFileSize(string );
bool fileExist(char * );
string generateHash(string );
string generateBitMap(string );
string getBitMap(string , string , string );
void uploadFile(string , int );
vector < vector<string>> getDetailsFromTracker(string , int );
void createEmptyFile(string , ll );
void * downloadFileParts(void * );
void downloadFile(int, string , string );
void listFilesByGroupID(int );
int connectIpPort(string , string );
void sendBitMap(vector <string> & , int );
void * acceptRequest(void * );
int userLogin(string , string );
int createUser(string , string );
void * clientMenu(void * );

vector <string> processedString(string input_data, char ch)
{
	vector <string> processed_string;
	int input_length = input_data.length();
	string word = "";
	for(int i=0; i<input_length; i++)
	{
		if(input_data[i] == ch || input_data[i] == '\n')
		{
			processed_string.push_back(word);
			word = "";
		}
		else
			word += input_data[i];
	}
	processed_string.push_back(word);
	return processed_string;
}

ll getFileSize(string file_path)
{
	char temp_file_path[file_path.size()+1];
	strcpy(temp_file_path, file_path.c_str());
	int file = 0;
    if((file = open(temp_file_path, O_RDONLY)) < -1)
        return 1;
    struct stat fileStat;
    if(fstat(file, &fileStat) < 0)    
        return 1;
	return fileStat.st_size;
}

bool fileExist(char *filename)
{
	struct stat buffer;   
	return (stat (filename, &buffer) == 0);
}

string generateHash(string file_path)
{  
	char temp_file_path[file_path.size()+1];
	strcpy(temp_file_path, file_path.c_str());
	FILE *fp = fopen(temp_file_path, "rb");
	ll file_size = getFileSize(file_path);
	int n;
	unsigned char buffer1[PART_SIZE]; 
	string hash_value;
	while ((n = fread(buffer1, sizeof(char), PART_SIZE, fp)) > 0  && file_size > 0)
	{
		unsigned char obuf[5];
		unsigned char hash[PART_SIZE];
		SHA1(buffer1, sizeof(buffer1) - 1, obuf);
		for (int i = 0; i < 5; i++) 
		{
			sprintf((char*)&hash, "%02x", obuf[i]);
			string temp_str((char *)hash);
			hash_value += temp_str;
    	}
   	 	memset(buffer1, '\0', PART_SIZE);
		file_size = file_size - n;
	}
	fclose(fp);
	return hash_value;
}

string generateBitMap(string temp_file_path)
{
	ll file_size = getFileSize(temp_file_path);
	char file_path[temp_file_path.length()+1];
	strcpy(file_path, temp_file_path.c_str());
	FILE *fp = fopen(file_path, "rb");
	int pos = 0;
	string bitmap;
  	while(pos < file_size)
  	{
  		fseek(fp , pos, SEEK_SET);
  		char ch1;
  		fscanf(fp, "%c", &ch1);
  		fseek(fp , pos+PART_SIZE-1, SEEK_SET);
  		char ch2;
  		fscanf(fp, "%c", &ch2);
  		if(ch1 == '\0' && ch2 == '\0')
  			bitmap += "0";
  		else
  			bitmap += "1";
  		pos += PART_SIZE;
  	}
  	return bitmap;
}

string getBitMap(string ip_address, string port_no, string file_path)
{
	string temp_str;
	int sockfd = connectIpPort(ip_address, port_no);
	if(sockfd < 0)
	{
		perror("connect failed");
		exit(EXIT_FAILURE);
	}
	else
	{
		string str1 = "bitmap:" + file_path + ":" + ip_address + ":" + port_no;
		char finalstr[str1.length() + 1];
		strcpy(finalstr, str1.c_str());
		send(sockfd, finalstr, sizeof(finalstr), 0);
		char data[BUFF_SIZE];
		recv(sockfd, data, sizeof(data), 0);
		temp_str = string(data);
	}
	close(sockfd);
	return temp_str;
}

void uploadFile(string file_path, int group_id)
{
	vector <string> upload_details = processedString(file_path, '/');
	string file_name = upload_details[upload_details.size()-1];
	string ip_address = TRACKER1_IP;
	string port_no = to_string(TRACKER1_PORT);
	int sockfd = connectIpPort(ip_address, port_no);
	string ipstring(IP);
	string portstring = to_string(PORT);
	ll temp_file_size = getFileSize(file_path);
	string file_size = to_string(temp_file_size);
	string hash_value = generateHash(file_path);
    string str1 = "upload:" + ipstring + ":" + to_string(PORT) + ":" + to_string(group_id) + ":" + file_name + ":" + file_path + ":" + file_size + ":" + hash_value;
	char finalstr[str1.length() + 1];
	strcpy(finalstr, str1.c_str());
	send(sockfd, finalstr, sizeof(finalstr), 0);
}

vector <vector <string>> getDetailsFromTracker(string file_name, int group_id)
{
	vector <string> temp_data_from_tracker;
	vector <vector <string>> data_from_tracker;
	string ip_address = TRACKER1_IP;
	string port_no = to_string(TRACKER1_PORT);
	int sockfd = connectIpPort(ip_address, port_no);
	char buffer[BUFF_SIZE];
	string str1 = "download:" + to_string(group_id) + ":" + file_name;
	char finalstr[str1.length() + 1];
	strcpy(finalstr, str1.c_str());
	send(sockfd, finalstr, sizeof(finalstr), 0);
	int n;
	while((n = recv(sockfd, buffer, BUFF_SIZE, 0)) > 0)
	{
		string temp_data(buffer);
		temp_data_from_tracker.push_back(temp_data);
		memset(buffer, '\0', BUFF_SIZE);
	} 
	for(int i=0; i<temp_data_from_tracker.size(); i++)
		data_from_tracker.push_back(processedString(temp_data_from_tracker[i], ':'));
	close(sockfd);
	return data_from_tracker;
}

void createEmptyFile(string temp_file_path, ll file_size)
{
	char file_path[temp_file_path.length()+1];
	strcpy(file_path, temp_file_path.c_str());
	FILE *fp = fopen(file_path, "w");
    fseek(fp, file_size-1 , SEEK_SET);
    fputc('\0', fp);
    fclose(fp);
}

void * downloadFileParts(void * td)
{
	struct thread_download_args *args = (struct thread_download_args *)td;
	string info = args->info;
	cout << info << endl;
	vector <string> download_info = processedString(info, ':');
	string file_name = download_info[0];
	string des_file_path = download_info[1];
	string ip_address = download_info[2];
	string port_no = download_info[3];
	string source_file_path = download_info[4];
	int group_id = stoi(download_info[5]);
	int sockfd = connectIpPort(ip_address, port_no);
	string str1 = "download:" + source_file_path;
	for(int i=6; i<download_info.size(); i++)
		str1 += ":" + download_info[i];
	char finalstr[str1.length() + 1];
	strcpy(finalstr, str1.c_str());
	send(sockfd, finalstr, sizeof(finalstr), 0);
	string temp_file_path = des_file_path + file_name;
	char file_path[temp_file_path.length()+1];
	strcpy(file_path, temp_file_path.c_str());
	pthread_mutex_lock(&lock1);
	for(int i=6; i<download_info.size(); i++)
	{
		FILE *fp = fopen(file_path, "r+");
		char buffer[BUFF_SIZE]; 
		int pos1 = stoi(download_info[i]);
		int total_file_size = getFileSize(file_path);
		int file_size1 = 0;
		int no_parts = ceil((double)total_file_size/PART_SIZE);
		if(no_parts == pos1)
			file_size1 = (total_file_size%PART_SIZE);
		else
			file_size1 = PART_SIZE;
		int n;
		fseek(fp, pos1*file_size1, SEEK_SET);
		while((file_size1 > 0) && (n = recv(sockfd, buffer, BUFF_SIZE, 0)) > 0)
		{
			fwrite(buffer, sizeof(char), n, fp);
			memset(buffer, '\0', BUFF_SIZE);
			file_size1 = file_size1 - n;
		} 
		fclose(fp);
	}
	pthread_mutex_unlock(&lock1);
	close(sockfd);
	string temp_upload = des_file_path + file_name;
	uploadFile(temp_upload, group_id);
}

void downloadFile(int group_id, string file_name, string destination_path)
{
	vector <vector <string>> details_from_tracker;
	details_from_tracker = getDetailsFromTracker(file_name, group_id);
	int no_threads = details_from_tracker.size();
	map <string, string> bitmap_details;
	map <string, string>::iterator itr;

	for(int i=0; i<no_threads; i++)
	{
		string ip_address = details_from_tracker[i][0];
		string port_no = details_from_tracker[i][1];
		string file_path = details_from_tracker[i][4];
		string temp_str = getBitMap(ip_address, port_no, file_path);
		vector <string> bit_data = processedString(temp_str, ':');
		string temp_bit_data = bit_data[0] + ":" + bit_data[1] + ":" + file_path;
		bitmap_details[temp_bit_data] = bit_data[2];
	}
	map <string, string> file_bits; 
	map <string, vector<int>>::iterator itr1;
	vector <pair <string, vector <int>>> temp_v1;
	int file_size = stoi(details_from_tracker[0][5]);
	int size_bitmap = ceil((double)file_size/PART_SIZE);
	vector <int> bit_details(size_bitmap, 0);
    for(itr = bitmap_details.begin(); itr != bitmap_details.end(); ++itr) 
    { 
    	vector <int> temp_v;
    	for(int i=0; i<itr->second.length(); i++)
    	{
    		if(itr->second[i] == '1')
    			temp_v.push_back(i);
    	}
    	temp_v1.push_back(make_pair(itr->first, temp_v));
    } 
    sort(temp_v1.begin(), temp_v1.end(), sort1);
    if(temp_v1[0].second.size() == size_bitmap && no_threads > 1)
    {
    	int i=0;
    	int x = size_bitmap/no_threads;
    	int m = size_bitmap%no_threads;
    	int j;
	    while(i<no_threads)
	    {
	    	string str2;
	    	for(j=i*x; j<(i+1)*x; j++)
	    	{
	    		if(bit_details[temp_v1[i].second[j]] == 0)
	    		{
	    			str2 += to_string(temp_v1[i].second[j]) + ":";
	    			bit_details[temp_v1[i].second[j]] = 1;
	    		}
	    	}
	    	str2[str2.length()-1] = '\0';
	    	file_bits[temp_v1[i].first] = str2;
	    	i++;
	    }
	    i--;
	    if(j<size_bitmap)
	    {
	    	string str3;
	    	for(int k=j; k<size_bitmap; k++)
	    	{
	    		if(bit_details[temp_v1[i].second[k]] == 0)
	    		{
	    			str3 += to_string(temp_v1[i].second[k]) + ":";
	    			bit_details[temp_v1[i].second[k]] = 1;
	    		}
	    	}
	    	str3[str3.length()-1] = '\0';
	    	file_bits[temp_v1[i].first] += str3;
	    }
    }
    else
    {
		string str1;
	    for(int j=0; j<temp_v1[0].second.size(); j++)
	    {
	    	str1 += to_string(temp_v1[0].second[j]) + ":";
	    	bit_details[temp_v1[0].second[j]] = 1;
		}
		str1[str1.length()-1] = '\0';
		file_bits[temp_v1[0].first] = str1;
		int i=1;
	    while(i<no_threads)
	    {
	    	string str2;
	    	for(int j=0; j<temp_v1[i].second.size(); j++)
	    	{
	    		if(bit_details[temp_v1[i].second[j]] == 0)
	    		{
	    			str2 += to_string(temp_v1[i].second[j]) + ":";
	    			bit_details[temp_v1[i].second[j]] = 1;
	    		}
	    	}
	    	str2[str2.length()-1] = '\0';
	    	file_bits[temp_v1[i].first] = str2;
	    	i++;
	    }
	}
	vector <string> temp_infov;
	for(itr =file_bits.begin(); itr != file_bits.end(); ++itr) 
    { 
    	string temp_info = file_name + ":" + destination_path + ":" + itr->first + ":" + to_string(group_id) + ":" + itr->second;
    	temp_infov.push_back(temp_info);
    } 
    string temp_file_path1 = destination_path+file_name;
	char file_path[temp_file_path1.length()+1];
	strcpy(file_path, temp_file_path1.c_str());
    if(!fileExist(file_path))
		createEmptyFile(temp_file_path1, file_size);
	
	if (pthread_mutex_init(&lock1, NULL) != 0)
    {
        cout << "mutex init failed !!! " << endl;
    }

	pthread_t threads[no_threads];
	for(int i=0; i<no_threads; i++)
	{
		struct thread_download_args *struct_thread = new thread_download_args();
		struct_thread->info = temp_infov[i];
		if(pthread_create(&threads[i], NULL, downloadFileParts, (void *)struct_thread) != 0)
		{
			cout << "Thread creation failed !!!" << endl;
        	if(pthread_detach(threads[i]) != 0)
            	cout << "Thread detach failed !!!" << endl;
		}
	}
	for(int i=0; i<no_threads; i++)
	{
		pthread_join(threads[i], NULL);
	}
	pthread_mutex_destroy(&lock1);
	// string temp_upload = destination_path + file_name;
	// uploadFile(temp_upload, group_id);
}

void listFilesByGroupID(int group_id)
{
	vector <string> temp_data_from_tracker;
	vector <vector <string>> data_from_tracker;
	string ip_address = TRACKER1_IP;
	string port_no = to_string(TRACKER1_PORT);
	int sockfd = connectIpPort(ip_address, port_no);
	char buffer[BUFF_SIZE];
	string str1 = "list_files:" + to_string(group_id);
	char finalstr[str1.length() + 1];
	strcpy(finalstr, str1.c_str());
	send(sockfd, finalstr, sizeof(finalstr), 0);
	int n;
	while((n = recv(sockfd, buffer, BUFF_SIZE, 0)) > 0)
	{
		string temp_data(buffer);
		temp_data_from_tracker.push_back(temp_data);
		memset(buffer, '\0', BUFF_SIZE);
	} 
	for(int i=0; i<temp_data_from_tracker.size(); i++)
		data_from_tracker.push_back(processedString(temp_data_from_tracker[i], ':')); 
	set <string> file_names_by_group;
	for(int i=0; i<data_from_tracker.size(); i++)
		file_names_by_group.insert(data_from_tracker[i][3]);
	for (auto it = file_names_by_group.begin(); it != file_names_by_group.end(); it++) 
        cout << "Group ID : "  << group_id << "  File name : " << *it << endl; 
	close(sockfd);
}
	
int connectIpPort(string ip_address, string port_no)
{
	char ip_add[ip_address.length()+1];
	strcpy(ip_add, ip_address.c_str());
	int port = stoi(port_no);
	int sockfd;
	if((sockfd = socket (AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed"); 
        exit(EXIT_FAILURE);
	}
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = inet_addr(ip_add);
	int addrlen = sizeof(sockaddr);
	if(connect (sockfd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("connect failed");
		exit(EXIT_FAILURE);
	}
	return sockfd;
}

void sendBitMap(vector <string> &request_details, int temp_sockfd)
{
	string bitmap = generateBitMap(request_details[1]);
	string str1 = request_details[2] + ":" + request_details[3] + ":" + bitmap;
	char finalstr[str1.length() + 1];
	strcpy(finalstr, str1.c_str());
	send(temp_sockfd, finalstr, sizeof(finalstr), 0);
	close(temp_sockfd);
}

void * acceptRequest(void * temp_sockfd)
{
	int sockfd = *((int *)temp_sockfd);
	char data[BUFF_SIZE];
	recv(sockfd, data, sizeof(data), 0);
	string temp_data(data);
	vector <string> request_details;
	request_details = processedString(temp_data, ':');
	if(request_details[0].compare("bitmap") == 0)
	{
		string bitmap = generateBitMap(request_details[1]);
		string str1 = request_details[2] + ":" + request_details[3] + ":" + bitmap;
		char finalstr[str1.length() + 1];
		strcpy(finalstr, str1.c_str());
		send(sockfd, finalstr, sizeof(finalstr), 0);
	}
	else if(request_details[0].compare("download") == 0)
	{
		string temp_file_path = request_details[1];
		char file_path[temp_file_path.length()+1];
		strcpy(file_path, temp_file_path.c_str());
		for(int i=2; i<request_details.size(); i++)
		{
			FILE *fp = fopen(file_path, "rb");
			int pos1 = stoi(request_details[i]);
			int total_file_size = getFileSize(file_path);
			int file_size = 0;
			int no_parts = ceil((double)total_file_size/PART_SIZE);
			if(no_parts == pos1)
				file_size = total_file_size%PART_SIZE;
			else
				file_size = PART_SIZE;
	    	if(fp == NULL)
	    		cout << "Could not open file !!! " << endl;
	    	char buffer[BUFF_SIZE]; 
	    	int n;
	    	fseek(fp, pos1*file_size, SEEK_SET);
			while ((file_size > 0) && (n = fread(buffer, sizeof(char), BUFF_SIZE, fp)) > 0)
			{
				send(sockfd, buffer, n, 0);
	   	 		memset(buffer, '\0', BUFF_SIZE);
				file_size = file_size - n;
			}	
			fclose(fp);
		}
	}
	close(sockfd);
}

int userLogin(string userid, string password)
{
	string ip_add = TRACKER1_IP;
	string port_no = to_string(TRACKER1_PORT);
	int sockfd = connectIpPort(ip_add, port_no);
	string str1 = "login:" + string(IP) + ":" + to_string(PORT) + ":" + userid + ":" + password; 
	char finalstr[str1.length() + 1];
	strcpy(finalstr, str1.c_str());
	send(sockfd, finalstr, sizeof(finalstr), 0);
	char data[BUFF_SIZE];
	recv(sockfd, data, sizeof(data), 0);
	string status(data);
	if(status.compare("success") == 0)
		return 0;
	else if(status.compare("password_wrong") == 0)
		return 1;
	else if(status.compare("not_exists") == 0)
		return 2;
}

int createUser(string userid, string password)
{
	int x = userLogin(userid, password);
	if(x == 0 || x == 1)
		return 1;
	else
	{
		string ip_add = TRACKER1_IP;
		string port_no = to_string(TRACKER1_PORT);
		int sockfd = connectIpPort(ip_add, port_no);
		string str1 = "create_user:" + string(IP) + ":" + to_string(PORT) + ":" + userid + ":" + password; 
		char finalstr[str1.length() + 1];
		strcpy(finalstr, str1.c_str());
		send(sockfd, finalstr, sizeof(finalstr), 0);
		char data[BUFF_SIZE];
		recv(sockfd, data, sizeof(data), 0);
		string status(data);
		if(status.compare("success") == 0)
			return 0;
		else if(status.compare("failed") == 0)
			return 1;
	}
}

void * clientMenu(void *temp_sockfd)
{
	int sockfd = *((int *)temp_sockfd);
	string userid, password;
	while(true)
	{
		cout << endl << "login/create_user : " << endl;
		string choice;
		cin >> choice;
		if((choice.compare("login")) == 0)
		{
			cout << endl << "User ID : ";
			cin >> userid;
			cout << endl;
			const char * pass = getpass("Password : ");
			string password(pass);
			int x = userLogin(userid, password);
			if(x == 0)
			{	
				cout << endl << "logged in successfully !!!" << endl;
				while (true)
				{
					cout << endl << "Enter command : " << endl;
					string input_command;
					cin >> input_command;
					if((input_command.compare("upload_file")) == 0)
					{
						cout << "Enter filepath and group id : " << endl;
						string input1;
						int input2;
						cin >> input1 >> input2;
						uploadFile(input1, input2);
					}
					else if((input_command.compare("download_file")) == 0)
					{
						cout << "Enter groupid, filename and destination path : " << endl;
						int input1;
						string input2;
						string input3;
						cin >> input1 >> input2 >> input3;
						downloadFile(input1, input2, input3);
					}
					else if((input_command.compare("list_files")) == 0)
					{
						cout << "Enter the groupid : " << endl;
						int input1;
						cin >> input1;
						listFilesByGroupID(input1);
					}
					else if((input_command.compare("logout")) == 0)
					{
						cout << endl << "logging out ..." << endl;
						break;
					}
				}	
			}
			else if(x == 1)
				cout << endl << "wrong Password !!!" << endl;
			else if(x == 2)
				cout << endl << "User doesn't exists !!!" << endl;
		}
		else if((choice.compare("create_user")) == 0)
		{
			cout << "create_user" << endl;
			cout << endl << "User ID : ";
			cin >> userid;
			cout << endl << "Password : ";
			cin >> password;
			int y = createUser(userid, password);
			if(y == 1)
				cout << endl << "User already exists !!!" << endl;
			else
				cout << endl << "User created ..." << endl;
		}
		else if((choice.compare("quit")) == 0)
			exit(0);
	}
}

int main(int argc, char const *argv[])
{
	
	IP = argv[1];
	PORT = atoi(argv[2]);
	int server_fd;
	if((server_fd = socket (AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed"); 
	    exit(EXIT_FAILURE);
	}
	pthread_t client_thread;
	int *sock1 = &server_fd;
	if(pthread_create(&client_thread, NULL, clientMenu, (void *) sock1) != 0)
	{
		cout << "client thread creation failed !!!" << endl;
       	if(pthread_detach(client_thread) != 0)
           	cout << "client thread detach failed !!!" << endl;
	}
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons( PORT );
	address.sin_addr.s_addr = inet_addr(IP);
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
		pthread_t server_thread;
		int *sock2 = &sockfd;
		if(pthread_create(&server_thread, NULL, acceptRequest, (void *) sock2) != 0)
		{
			cout << "server thread creation failed !!!" << endl;
        	if(pthread_detach(server_thread) != 0)
            	cout << "server thread detach failed !!!" << endl;
		}
	}
	close(server_fd);
	
	return 0;
}	
