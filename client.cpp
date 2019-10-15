#include "utility.h"

vector<string>command;
long int chunk_size=524288;
string client_ip,log_file;


void createPeerClient(string seederport,string fname,string file_needed)
{	   
	
	int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
     
    char buffer[1024] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(stoi(seederport)); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        //return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        //return -1; 
    } 
	
    char* reply_char_err=new char[file_needed.length()+1];
	strcpy(reply_char_err,file_needed.c_str());
	send(sock ,reply_char_err,strlen(reply_char_err),0);
	FILE *fp = fopen ( fname.c_str() , "wb" );
	size_t chunk=512;
	char Buffer [chunk] ; 
	long long file_size;

	long long n;
	if(recv(sock, &file_size, sizeof(file_size), 0)<0)
		perror("error\n");
	else
	cout<<file_size<<"filesize :client"<<endl;
	memset(Buffer , '\0', chunk);
	while ( ( n = recv( sock , Buffer ,   chunk, 0) ) > 0  &&  file_size > 0)
	{
	    
		fwrite (Buffer , sizeof (char), n, fp);
		memset ( Buffer , '\0', chunk);
		file_size = file_size - n;
		if(n<=0 || file_size<=0)
		{
			break;
		}
} 
	cout<<"Data transfer complete "<<endl;
	close(sock);
	fclose (fp); 	
   
}

void *makeMyServer(void *clientIP){
    // cout<<"Created makeMyServer"<<endl;
    int server_fd;
    struct sockaddr_in address;
    int opt = 1, new_socket;
    int addrlen = sizeof(address);
    string cip = *(string *)clientIP;
    string client_ip, SPORT;
    char *s_ip;
    vector<string> cmd = splitstring(cip, ':');
    client_ip = cmd[0];
    SPORT = cmd[1];
    s_ip = new char[client_ip.length()];
    strcpy(s_ip, client_ip.c_str());
    //  Connection establishment code.
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        //logprinter("socket failed -- server");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        //logprinter("setsocketopt -- server");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    //address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(stoi(SPORT));

    if (inet_pton(AF_INET, s_ip, &address.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        //logprinter("Invalid address/ Address not supported -- server");
        return clientIP;
    }

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        //logprinter("bind failed -- server");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("listen");
        //logprinter("listen error -- server");
        exit(EXIT_FAILURE);
    }
    
      
    
    	while(1)
    	{ 
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
                       
            exit(EXIT_FAILURE);
        }
        int client_port_num=ntohs(address.sin_port);
		string client_port_str=to_string(client_port_num);
		int* argument=(int *)malloc(sizeof(*argument));
		*argument=new_socket;

		write_in_log("connection established with "+client_port_str);
	int conn_sock=new_socket;
	char buffer[512]={0};
	int status=read(conn_sock,buffer,1024);
	if(status==0)
		return clientIP;
	string fname=string(buffer);
	cout<<fname<<endl;
	
	size_t chunk=512; 

    long long filesize=getfile_size(fname);
    cout<<filesize<<endl;
    
    send(conn_sock,&filesize,sizeof(filesize),0);
    FILE *fp= fopen(fname.c_str(),"rb");
    char buffer1[chunk];
    long long n;
    while ( ( n = fread( buffer1 , sizeof(char) , chunk , fp ) ) > 0  && filesize > 0 )
    {
    	
        send (conn_sock,buffer1, n, 0 );
        memset ( buffer1 , '\0', chunk);
        filesize = filesize - n ;
        if(n<=0 || filesize <=0)
        {
            break;
        }
    }



	close(conn_sock);      
   }
    
    
}
int main(int argc, char* argv[])
{
	
	if(argc!=3)
	{
		cout<<"invalid arguments"<<endl;
		exit(1);
	}
	log_file="log_file";
	ofstream output;
	output.open(log_file,fstream::out);
	output.close();

	string c_ip,c_port;
	string t1_ip,t1_port;
	string t2_ip,t2_port;

	write_in_log("in_main(client)");
	client_ip=string(argv[1]);
	
	command=splitstring(client_ip,':');
	c_ip=command[0],c_port=command[1];

	t1_ip="127.0.0.1",t1_port="5000";

	t2_ip="127.0.0.1",t2_port="6000";

	
	pthread_t serverthread;
	pthread_attr_t thread_attr2;
    int res2 = pthread_attr_init(&thread_attr2);
    if (res2 != 0) {
        perror("Attribute creation failed");
        exit(EXIT_FAILURE);
    }
   
    if( pthread_create( &serverthread , &thread_attr2 ,  makeMyServer , (void*)&client_ip) < 0)
    {
        perror("could not create thread");
        return 1;
    }

	int sock_fd=socket(AF_INET,SOCK_STREAM,0);
	if(sock_fd<0)
	{
		cout<<"error in connection from client";
		write_in_log("error in connection from client");
		exit(1);
	}
	struct sockaddr_in serv_addr;
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(stoi(t1_port));
	char* t1_ip_char=new char[t1_ip.length()+1];
	strcpy(t1_ip_char,t1_ip.c_str());

	if (inet_pton(AF_INET, t1_ip_char, &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        
        return -1;
    }

	int ret=connect(sock_fd,(struct sockaddr *)&serv_addr  , sizeof(serv_addr));
	if(ret<0)
	{
		cout<<"connection failed"<<endl;
		write_in_log("connection failed");
		exit(1);
	}

	write_in_log("connection established ");
	int flag=0;
	string operation,username,passwd,upfile_name;
	while(1)
	{
		cout<<">>> ";
		cin>>operation;

		if(operation=="login")
		{
			int validbit;
			cin>>username>>passwd;
			string del=":";
			string data=operation+del+username+del+passwd;
			write_in_log(data);
			char* data_char=new char[data.length()+1];
			strcpy(data_char,data.c_str());
			send(sock_fd,data_char,strlen(data_char),0);
			recv(sock_fd,&validbit,sizeof(validbit),0);
			if(validbit==1)
			{
				flag=1;
				cout<<"You are now logged in"<<endl;
				write_in_log("login successful");
			}	
			else
			{
				cout<<"wrong username or password"<<endl;
				write_in_log("login unsuccessful");
			}



		}
		else if(operation=="create_user")
		{
			int validbit;
			cin>>username>>passwd;
			string del=":";
			string data=operation+del+username+del+passwd;
			write_in_log(data);
			if(flag==0){
				char* data_char=new char[data.length()+1];
				strcpy(data_char,data.c_str());
				send(sock_fd,data_char,strlen(data_char),0);
				recv(sock_fd,&validbit,sizeof(validbit),0);
				if(validbit==1)
				{
				
					cout<<"user created successfully "<<endl;
					write_in_log("user created successfully");
				}
				else
				{
					cout<<"unsuccessful creation : You are already logged in"<<endl;
					write_in_log("unsuccessful creation");
				}	
			}
			
			else
			{
				cout<<"unsuccessful creation"<<endl;
				write_in_log("unsuccessful creation");
			}

		}
		else if(operation=="upload_file")
		{
			if(flag)
			{
				cin>>upfile_name;
				string grpid;
				cin>>grpid;

				char buffer[1024]={0};
				string del=":";
				string data=operation+del+upfile_name;
				write_in_log(data);
				string upfile_hash=upload(upfile_name);
				if(upfile_hash=="$")
					continue;
				long long fsize=getfile_size(upfile_name);
				cout<<"file size is "<<fsize<<" Bytes"<<endl;
				string request=operation+del+upfile_hash+del+c_ip+del+upfile_name+del+c_port+del+grpid+del+to_string(fsize)+del+username;
				char *upfile_hash_char;
				upfile_hash_char=new char[request.length()+1];
				strcpy(upfile_hash_char,request.c_str());
				send(sock_fd,upfile_hash_char,strlen(upfile_hash_char),0);
				read(sock_fd,buffer,1024);
				write_in_log(string(buffer));
				cout<<string(buffer)<<endl;

			}
			else
			{
				cout<<"Please login first"<<endl;
				write_in_log("user not found");
			}
		}
		else if(operation=="download_file")
		{
			string grpid,file_name,dest;
			cin>>grpid>>file_name>>dest;
			if(flag)
			{
				char buffer[4096]={0};
				string del=":";
				string request=operation+del+file_name+del+grpid;
				char* request_char=new char[request.length()+1];
				strcpy(request_char,request.c_str());
				send(sock_fd,request_char,strlen(request_char),0);
				read(sock_fd,buffer,4096);
				cout<<string(buffer)<<endl;
				std::vector<string> seeder_vec=splitstring(string(buffer),'|');
				long long file_size=stoi(seeder_vec[(seeder_vec.size())-1]);
				int seederscount=seeder_vec.size()-1;
				seederinfo seederlist[seederscount];
				string testip,testport;
				for(int i=0;i<seeder_vec.size()-1;i++)
				{
					string ipandport=seeder_vec[i];
					std::vector<string> temp=splitstring(ipandport,':');
					seederinfo sinfo;
					sinfo.seeder_ip=temp[0];
					sinfo.seeder_port=temp[1];
					testip=temp[0];
					testport=temp[1];
					sinfo.filepath=file_name;
					sinfo.destpath=dest;
					sinfo.numofseeders=seederscount;
					seederlist[i]=sinfo;
				}
				
				createPeerClient(testport,dest,file_name);
				string upfile_hash=upload(dest);
				if(upfile_hash=="$")
					continue;
				long long fsize=getfile_size(dest);
				
			    request="upload_file"+del+upfile_hash+del+c_ip+del+dest+del+c_port+del+grpid+del+to_string(fsize)+del+username;
				char *upfile_hash_char;
				upfile_hash_char=new char[request.length()+1];
				strcpy(upfile_hash_char,request.c_str());
				send(sock_fd,upfile_hash_char,strlen(upfile_hash_char),0);
				read(sock_fd,buffer,1024);
				cout<<buffer<<endl;
				request="add_download"+del+username+del+grpid+del+dest;
				char* request_down=new char[request.length()+1];
				strcpy(request_down,request.c_str());
				send(sock_fd,request_down,strlen(request_down),0);

			}
			else
			{
				cout<<"Please login first"<<endl;
				write_in_log("user not found");

			}
		}
		else if(operation=="create_group")
		{
			string grpid;
			cin>>grpid;
			if(flag)
			{
			char buffer[1024]={0};
			string del=":";
			string request=operation+del+grpid+del+username;
			char* request_char=new char[request.length()+1];
			strcpy(request_char,request.c_str());
			send(sock_fd,request_char,strlen(request_char),0);
			read(sock_fd,buffer,1024);
			write_in_log(string(buffer));
			cout<<string(buffer)<<endl;
		}
		else
		{
				cout<<"Please login first"<<endl;
				write_in_log("user not found");
		}

		}
		else if(operation=="list_groups")
		{
			if(flag)
			{
			char buffer[1024]={0};
			string del=":";
			string request=operation;
			char* request_char=new char[request.length()+1];
			strcpy(request_char,request.c_str());
			send(sock_fd,request_char,strlen(request_char),0);
			read(sock_fd,buffer,1024);
			write_in_log(string(buffer));
			string grps=string(buffer);
			std::vector<string> grpnames;
			grpnames=splitstring(grps,':');
			for(int itr=0;itr<grpnames.size();itr++)
				cout<<grpnames[itr]<<endl;
		}
		else
		{
				cout<<"Please login first"<<endl;
				write_in_log("user not found");
		}


		}
		else if(operation=="join_group")
		{
			if(flag==1)
			{
				char buffer[1024]={0};
				string grpid;
				cin>>grpid;
				string del=":";
				string request=operation+del+grpid+del+username;
				char* request_char=new char[request.length()+1];
				strcpy(request_char,request.c_str());
				send(sock_fd,request_char,strlen(request_char),0);
				read(sock_fd,buffer,1024);
				write_in_log(string(buffer));
				cout<<string(buffer)<<endl;

			}
			else
			{
				cout<<"Please login first"<<endl;
				write_in_log("user not found");

			}
			

		}
		else if(operation=="leave_group")
		{
			if(flag==1)
			{
				char buffer[1024]={0};
				string grpid;
				cin>>grpid;
				string del=":";
				string request=operation+del+grpid+del+username;
				char* request_char=new char[request.length()+1];
				strcpy(request_char,request.c_str());
				send(sock_fd,request_char,strlen(request_char),0);
				read(sock_fd,buffer,1024);
				write_in_log(string(buffer));
				cout<<string(buffer)<<endl;

			}
			else
			{
				cout<<"Please login first"<<endl;
				write_in_log("user not found");

			}

		}
		else if(operation=="list_requests")
		{
			if(flag==1)
			{
				char buffer[4096]={0};
				string grpid;
				cin>>grpid;
				string del=":";
				string request=operation+del+grpid+del+username;
				char* request_char=new char[request.length()+1];
				strcpy(request_char,request.c_str());
				send(sock_fd,request_char,strlen(request_char),0);
				read(sock_fd,buffer,1024);
				write_in_log(string(buffer));
				request=string(buffer);
				std::vector<string> join_req;
				join_req=splitstring(request,':');
				for(int itr=0;itr<join_req.size();itr++)
					cout<<join_req[itr]<<endl;

			}
			else
			{
				cout<<"Please login first"<<endl;
				write_in_log("user not found");

			}

		}
		else if(operation=="accept_request")
		{
			if(flag==1)
			{
				char buffer[1024]={0};
				string grpid,uid;
				cin>>grpid>>uid;
				string del=":";
				string request=operation+del+grpid+del+uid+del+username;
				char* request_char=new char[request.length()+1];
				strcpy(request_char,request.c_str());
				send(sock_fd,request_char,strlen(request_char),0);
				read(sock_fd,buffer,1024);
				write_in_log(string(buffer));
				cout<<string(buffer)<<endl;
			}
			else
			{
				cout<<"Please login first"<<endl;
				write_in_log("user not found");

			}	

		}
		else if(operation=="list_files")
		{
			if(flag==1)
			{
				char buffer[4096]={0};
				string grpid;
				cin>>grpid;
				string del=":";
				string request=operation+del+grpid+del+username;
				char* request_char=new char[request.length()+1];
				strcpy(request_char,request.c_str());
				send(sock_fd,request_char,strlen(request_char),0);
				read(sock_fd,buffer,1024);
				write_in_log(string(buffer));
				request=string(buffer);
				std::vector<string> files_list;
				files_list=splitstring(request,':');
				for(int itr=0;itr<files_list.size();itr++)
					cout<<files_list[itr]<<endl;

			}
			else
			{
				cout<<"Please login first"<<endl;
				write_in_log("user not found");

			}

		}
		else if(operation=="logout")
		{
			if(flag==1)
			{
				char buffer[4096]={0};
				string del=":";
				string request=operation+del+username;
				char* request_char=new char[request.length()+1];
				strcpy(request_char,request.c_str());
				send(sock_fd,request_char,strlen(request_char),0);
				read(sock_fd,buffer,1024);
				write_in_log(string(buffer));
				cout<<string(buffer)<<endl;
				flag=0;

			}
			else
			{
				cout<<"you are not logged in! first login."<<endl;
			}
		}
		else if(operation=="stop_share")
		{
			string grpid;
			string stop_share_file;
			cin>>grpid>>stop_share_file;
			if(flag==1)
			{
				char buffer[4096]={0};
				string del=":";
				long long file_exist_flag=getfile_size(stop_share_file);
				if(file_exist_flag==-1)
				{
					cout<<"file does not exist"<<endl;
					continue;
				}
				string request=operation+del+grpid+del+stop_share_file+del+username;
				char* request_char=new char[request.length()+1];
				strcpy(request_char,request.c_str());
				send(sock_fd,request_char,strlen(request_char),0);
				read(sock_fd,buffer,1024);
				write_in_log(string(buffer));
				cout<<string(buffer)<<endl;

			}
			else
			{
				cout<<"you are not logged in! first login."<<endl;
			}

		}
		else if(operation=="show_downloads")
		{
				string del=":";
				char buffer[4096]={0};
				string request=operation+del+username;
				char* request_char=new char[request.length()+1];
				strcpy(request_char,request.c_str());
				send(sock_fd,request_char,strlen(request_char),0);
				read(sock_fd,buffer,4096);
				request=string(buffer);
				std::vector<string> down_files;
				down_files=splitstring(request,'|');
				for(int itr=0;itr<down_files.size();itr++)
					cout<<down_files[itr]<<endl;

		}
		

	}
	
	return 0;


}

std::vector<string> splitstring(string str,char delim)
{
	std::vector<string> vec;
	int i=0;
	string s="";
	while(i<str.length())
	{
		if(str[i]==delim)
		{
			vec.push_back(s);
			s="";
			i++;
		}
		else
		{
			s=s+str[i];
			i++;
		}
	}
	vec.push_back(s);
	return vec;
}

void write_in_log(string str)
{
	ofstream logger(log_file, std::ios_base::out | std::ios_base::app );
    time_t now;
    time(&now);
    char *date=ctime(&now);
    date[strlen(date)-1]='\0';
    logger<<date<<" "<<"CLIENT "<<str<<endl;  
}
