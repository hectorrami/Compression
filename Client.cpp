#include <unistd.h>
#include <iostream>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <stdio.h> 
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <vector>
#include <algorithm>

/*function prototypes*/
std::vector<char> frequency(std::vector<char> vec);
void remove(std::string &str, char ele);
std::string Encode(std::string inputstring);
std::string Decode(std::string inputstring);
std::string DecodeVector(std::vector<char> vec);

/*struct to pass as parameter to pthread_create for child threads*/
struct mystruct {
    char symbol;
    std::string message;
    int size; 
    char binarycode[100];
    int portno;
    char **argv;
};

/*called when a system call fais*/
void error(char *msg){
    perror(msg);
    exit(0);
}

void *clientThread(void *argument)
{
    struct mystruct *arg = (struct mystruct*)argument;

    std::string str = arg->message;
    char symbol = arg->symbol;
    int size = arg->size;
    char** argv = arg->argv;

    //std::cout << "Here is the received msg: " << str << std::endl;
    //std::cout << "Symbol: " << symbol << std::endl;
    //std::cout << "Size: " << size << std::endl;

    int sockfd, portno, n; //sockfd is a file descriptor
    struct sockaddr_in serv_addr; //contain address of server where we are connecting
    struct hostent *server; //pointer to a struct hostent


    char buffer[256];
    
    //checking arguments
    // if (argc < 3) {
    //     fprintf(stderr,"usage %s hostname port\n", argv[0]);
    //     exit(0); 
    // }
    
    portno = arg->portno; 
    //creating a new socket.
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0)
        std::cerr << "ERROR opening socket\n";

    /*Need to read host name form command line*/
    // struct hostent *lochost = gethostbyname("localhost");
    // server = lochost;
    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    /*if null, the system can not locate a host*/
    if(server == NULL){
        std::cerr << "ERROR, no such host\n";
        exit(1);
    }

    /*sets everything in the buffer to 0*/
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
    serv_addr.sin_port = htons(portno);//converst host byte order to network byte order

    /*returns 0 for success -1 for failure*/
    if (connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            std::cerr << "ERROR connecting\n";
    
    /*MESSAGE SENDING*/
    //std::cout << "Please enter the message\n" << std::endl;
    bzero(buffer, 256);
    //fgets(buffer, 255, stdin); // reads the msg from stdin 
    for(int i =0; i < str.length(); i++)
        buffer[i] = str.at(i);
   
    n = write(sockfd,buffer,strlen(buffer)); //n is an int that cointains the num of chars written
    if(n < 0)
        std::cerr << "ERROR writing into socket\n";
    
    bzero(buffer, 256);

    n = read(sockfd, buffer, 255);
    if(n < 0)
        std::cerr << "ERROR Reading from socket\n";

    //std::cout << buffer << std::endl;
   
    /*SENDING ANOTHER MESSAGE*/
    bzero(buffer, 256);

    buffer[0] = symbol;

    n = write(sockfd,buffer,sizeof(buffer));
    if(n < 0)
        std::cerr << "ERROR writing into socke\n";

    bzero(buffer,256);    
    n = read(sockfd, buffer, size); 
    if(n < 0)
        std::cerr << "ERROR Reading from socket\n";
    
    std::cout << "Server: ";
    std::cout << buffer << std::endl;
    for(int i=0; i < strlen(buffer); i++)
        arg->binarycode[i] = buffer[i];

    /*END OF ANOTHER MESSAGE*/
    
    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char *argv[]){

    int portnumber;

    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

   portnumber = atoi(argv[2]);

   std::vector <char> mycharacters,sortedvector,parentvector;
   std::string original, input;
   
   /*Inputing data from file.txt*/
   while(getline(std::cin, input)){
        original.append(input);
        if(std::cin.good())
            original.append("<EOL>");
    }

   std::string encodedstring = Encode(original);

   for(int i =0; i < encodedstring.length();i++){
        mycharacters.push_back(encodedstring[i]);
   }
   /*sort vector*/
   sortedvector = frequency(mycharacters);
   std::cout << "Original Message:" << Decode(DecodeVector(mycharacters)) << std::endl;

   int NTHREADS = sortedvector.size();
   char mostfreq;

   /*declare a static struct*/
   static struct mystruct args[100];
   pthread_t tid[NTHREADS];

   /*=======THREAD CREATION=======*/
   for(int i =0; i < NTHREADS; i++){  
       //std::cout << "Creating thread "<< i << std::endl;
       args[i].message = encodedstring;
       args[i].symbol = sortedvector[i];
       args[i].size = encodedstring.length();
       args[i].portno = portnumber;
       args[i].argv = argv;

       mostfreq = sortedvector[i];

       if(pthread_create(&tid[i],NULL,clientThread, &args[i])){
           std::cerr << "Error creating thread\n";
           exit(0);
       }
       /*removes most frequent character*/
       remove(encodedstring, mostfreq);
       sleep(1);  
   }

    /*wait for child threads*/
    for(int i =0; i < NTHREADS; i++)
        pthread_join(tid[i], NULL);

    std::cout << "\n\n\n==========================================\n" << 
    "COMPRESSED MESSAGE AND SYMBOLS\n==========================================\n";
    for(int i =0; i < NTHREADS; i++){

        if(args[i].symbol == '@')
            std::cout << "Symbol <EOL> " << "code:" << args[i].binarycode << std::endl;
        else
            std::cout << "Symbol " << args[i].symbol << " code:" << args[i].binarycode << std::endl;
        
        /*Break if the string is empty*/
        if(args[i+1].message.empty())
            break;

        /*Decode string*/
    
        std::cout << "Remaining Message:" << Decode(args[i+1].message) << std::endl;
    }   
  return 0;
}

/*FUNCTION DECLARATIONS*/
std::string Encode(std::string inputstring){  
    size_t found = inputstring.find("<EOL>");
    while(found != std::string::npos){
        inputstring.replace(found,5,"@");
        found = inputstring.find("<EOL>");
    }
    return inputstring;
}

std::string Decode(std::string inputstring){
    size_t found = inputstring.find("@");
    while(found != std::string::npos){
        inputstring.replace(found, 1,"<EOL>" );
        found = inputstring.find("@");
    }
    return inputstring;
}

std::string DecodeVector(std::vector<char> vec){
    std::string convert;
    for(int i =0; i <  vec.size(); i++){
        convert += vec[i];
    }
    return convert;
}
/*frequency function declaration*/
std::vector<char> frequency(std::vector<char> vec){

    std::vector<char> sortedvector;

     /*CALCULATING FREQUENCY*/
    int frequencyArray[256]; 
    int ascii;
    /*
        Initilize array of 256 indexes
        to 0 for every index.
    */ 
    for(int i =0 ;  i < 256;i++){
        frequencyArray[i] =0;
    }
    /*
        we set ascii value to the value of the char
        Example: for a, ascii = 97;
        we then incriment the value of the index in array[97]
        since everything is initilized to 0, everytime it 
        encounters the 97, the index will incriment by 1
        so we will have an array at index 97 with the 
        value being the number of times it encountered it(frequency);
    */
    for(int i =0 ; i < vec.size();i++){
        ascii = vec[i];
        frequencyArray[ascii]++;
    }
   
    /*
        SORTING IN DECESNDING ORDER AND INSERTING TO SORTED VECTOR;
    */
    for(int i =0; i < 256; i++){

        int max = frequencyArray[i];
        int index =0;

        for(int j =0; j< 256; j++){

            if(frequencyArray[j] > max){
            
            max = frequencyArray[j];
            index =j;
            }
            else if(frequencyArray[j]==max) {
                if (index>j)
                {
                    max = frequencyArray[j];
                }
                if (j==64)
                {
                    index = j; 
                }
            }

        }
        if(max == 0)
            break;

        char sorted = index;
        sortedvector.push_back(sorted);

        if(sorted == '@')
            std::cout << "<EOL> frequency = " << max << std::endl;
        else
            std::cout << sorted << " frequency = " << max << std::endl;
        
        frequencyArray[index] =0;
    }
    return sortedvector;
}

void remove(std::string &str, char ele){

     for(int i =0; i < str.length(); i++){
        if(str[i] == ele){
            str.erase(std::remove(str.begin(), str.end(), ele), str.end());
        }
     }
}
