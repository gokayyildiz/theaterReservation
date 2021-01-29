#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include <bits/stdc++.h>


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_teller = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_printer = PTHREAD_MUTEX_INITIALIZER;

struct client_data{
    int id ;
    std::string client_name;
    int arrival_time;
    int service_time;
    int requested_seat;
} *all_clients;

int theaterCapacity;

bool activate_tellers ;

bool isAvailable[3];

client_data buffers[3];

int *reservedSeats; // each index represents the customer id, and value represents assigned saet number

bool *theaterHall; // each index correspons to (index+1)th seat number and value represents if it is taken or not

bool *continueSignal;//continue signal for clients after releasing

void* clientThread(void *param) {
    int offset = *(int *) param;
    int local_arrive = all_clients[offset].arrival_time;
    std::string local_name = all_clients[offset].client_name;
    int local_service = all_clients[offset].service_time;
    int local_request = all_clients[offset].requested_seat;

   usleep(local_arrive*1000);
    
        // Start critical section

        
        pthread_mutex_lock(&mutex);

        while(1){
        if(!isAvailable[0] && !isAvailable[1] && !isAvailable[2])
            continue;
        
        if(isAvailable[0]){
            buffers[0].id = offset;
            buffers[0].arrival_time = local_arrive;
            buffers[0].client_name = local_name;
            buffers[0].requested_seat = local_request;
            buffers[0].service_time = local_service;
            isAvailable[0] = false;
           // printf("this is the client id %d, sent to teller A\n", offset);
            break;

        }else if(isAvailable[1]){          
            buffers[1].id = offset;
           buffers[1].arrival_time = local_arrive;
            buffers[1].client_name = local_name;
           buffers[1].requested_seat = local_request;
            buffers[1].service_time = local_service;
            isAvailable[1] = false;
           // printf("this is the client id %d, sent to teller B\n", offset);
            break;
        }else if(isAvailable[2]){          
            buffers[2].id = offset;
             buffers[2].arrival_time = local_arrive;
             buffers[2].client_name = local_name;
            buffers[2].requested_seat = local_request;
             buffers[2].service_time = local_service;
            isAvailable[2] = false;
           // printf("this is the client id %d, sent to teller C\n", offset);
            break;
        }
        
        }
        // End critical section
        pthread_mutex_unlock(&mutex);

      /*  while(!continueSignal[offset]);
        
        pthread_mutex_lock(&mutex_printer);
        std::cout<< local_name;
        printf(" requests seat %d, reserves seat %d. Signed by Teller\n", local_request, reservedSeats[offset]);


        pthread_mutex_unlock(&mutex_printer);
*/



       // usleep(local_service*1000);
 
    pthread_exit(NULL);
}


void* tellerThread(void *param){
    int offset = *(int *) param;
    char teller;
    switch(offset){
        case 1:
            teller = 'A';
            isAvailable[0] = true;
            break;
        case 2:
            teller = 'B';
            isAvailable[1] = true;
            break;
        case 3:
            teller = 'C';
            isAvailable[2]= true;
            break;
    }
    printf("Teller %c has arrived.\n", teller);

    do{
        
        // DO THE ESSENTIAL WORK IN HERE
        while(isAvailable[offset-1]);

        pthread_mutex_lock(&mutex_teller);

        //printf("this is the client id %d, arrival time: %d, service time: %d, and seat: %d at %c\n", buffers[offset-1].id,buffers[offset-1].arrival_time,buffers[offset-1].service_time,buffers[offset-1].requested_seat, teller);
       
        if(theaterHall[buffers[offset-1].requested_seat -1]){// if reserved
            for(int i = 0; i< theaterCapacity; i++){
                if(theaterHall[i])// if reserved
                    continue;
                else{
                    theaterHall[i] = true;
                    reservedSeats[buffers[offset-1].id] = i+1;
                    break;
                }
            }
        }else{
            theaterHall[buffers[offset-1].requested_seat -1] = true;
            reservedSeats[buffers[offset-1].id] = buffers[offset-1].requested_seat;
        }


        pthread_mutex_unlock(&mutex_teller);

        usleep(buffers[offset-1].service_time * 1000);
     pthread_mutex_lock(&mutex_teller);
        std::cout << buffers[offset-1].client_name;
        printf(" requests seat %d, reserves seat %d. Signed by Teller %c.\n",buffers[offset-1].requested_seat,reservedSeats[buffers[offset-1].id], teller);
     //  continueSignal[buffers[offset-1].id] = true;
        isAvailable[offset-1] = true;
        
        //printf("teller %c is available\n", teller);
        
     pthread_mutex_unlock(&mutex_teller);


    }while(activate_tellers);
  // printf("Teller %c has ended.\n", teller);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]){
     std::ifstream inFile(argv[1]);
    std::string theaterName;
    int numOfClients;
    inFile>> theaterName;
    inFile >> numOfClients;
    theaterCapacity = 60;
    
    
    theaterHall = new bool[theaterCapacity]; // change it later in accordance with different theater halls
    // each chair is initialized to 0 which indicates that it is empty

    all_clients = new client_data[numOfClients];
    reservedSeats = new int[numOfClients];
    std::string temp;
    for(int i= 0; i < numOfClients; i++){
        reservedSeats[i] = 0; // initialley all seats are 0, before assigning to customers
        all_clients[i].id = i;
        inFile>>temp;
        std::stringstream ss(temp);  
        std::string sub;
        getline(ss, sub, ',');  
        all_clients[i].client_name = sub;
        getline(ss, sub, ',');  
        all_clients[i].arrival_time = stoi(sub);
        getline(ss, sub, ',');  
        all_clients[i].service_time = stoi(sub);
        getline(ss, sub, '\n');  
        all_clients[i].requested_seat= stoi(sub);

    }
    printf("Welcome to the Sync-Ticket!\n");

    activate_tellers = true;
    // we read the input file so far
    pthread_t teller_tids[3];
    int tel_enum[3];
    tel_enum[0] = 1;
    tel_enum[1] = 2;
    tel_enum[2] = 3;
    for(int i = 0 ; i < 3 ; i++){
        pthread_create(&teller_tids[i], NULL, tellerThread, &tel_enum[i]);
    }



    pthread_t client_tids[numOfClients]; // client thread IDs
    for(int i = 0 ; i < numOfClients ; i++){
        pthread_create(&client_tids[i], NULL, clientThread, &all_clients[i].id);
    }

  


 for(int i = 0 ; i < numOfClients ; i++){//clint thread joins
        pthread_join(client_tids[i], NULL);
    }

    activate_tellers = false;
	
for(int i = 0 ; i < 3 ; i++){
        pthread_join(teller_tids[i], NULL);
    }
    printf("All clients received service.\n");

    delete[] all_clients;
    delete[] reservedSeats;
    delete[] theaterHall;
    delete[] continueSignal;

    return 0;
}
