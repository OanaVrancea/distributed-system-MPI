#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>

using namespace std;

int main (int argc, char *argv[])
{
    int numtasks, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    int N;
    vector<int> arr;
    vector<int> final_arr;
    vector<int> final_arr_cluster0;
    vector<int> final_arr_cluster1;
    vector<int> final_arr_cluster2;

    int arr_size;
    int arr_size_cluster0;
    int arr_size_cluster1;
    int arr_size_cluster2;
    int done = 0;

    int iterations_per_worker = 0; 
    int size;
    int extra_size;
    int size_to_send;

    int master;
    int recv;
    MPI_Status status;

    vector<int> cluster0;
    vector<int> cluster1;
    vector<int> cluster2;

    vector<int> cluster0_recv;
    vector<int> cluster1_recv;
    vector<int> cluster2_recv;

    int size_cluster0_recv = 0;
    int size_cluster1_recv = 0;
    int size_cluster2_recv = 0;

    /*
        Fiecare proces coordonator isi citeste fisierul si salveaza in vectorul
        specific lui rangurile proceselor worker pe care le coordoneaza 
    */
    if (rank == 0) {      
        int nr_nodes;
        int number;
        ifstream infile("cluster0.txt");
        infile >> nr_nodes;
        size_cluster0_recv = nr_nodes;
        while (nr_nodes > 0)
        {
            infile >> number;
            cluster0.push_back(number);
            nr_nodes--;
        }      
    } else if(rank == 1){
        int nr_nodes;
        int number;
        ifstream infile("cluster1.txt");
        infile >> nr_nodes;
        size_cluster1_recv = nr_nodes;
        while (nr_nodes > 0)
        {
            infile >> number;
            cluster1.push_back(number);
            nr_nodes--;
        }      
    } else if(rank == 2){
        int nr_nodes;
        int number;
        ifstream infile("cluster2.txt");
        infile >> nr_nodes;
        size_cluster2_recv = nr_nodes;
        while (nr_nodes > 0)
        {
            infile >> number;
            cluster2.push_back(number);
            nr_nodes--;
        }      
    }

    /*
        Procesele coordonator trimit mesaje de send proceselor worker
        pentru a ii anunta cine este coordonatorul lor
    */
    if (rank == 0) {    
        master = 0;  
        for(int i = 0; i < cluster0.size(); i++){
            MPI_Send(&master, 1, MPI_INT, cluster0[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster0[i]);
        } 
    } else if (rank == 1) {    
        master = 1;  
        for(int i = 0; i < cluster1.size(); i++){
            MPI_Send(&master, 1, MPI_INT, cluster1[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster1[i]);
        } 
    }  else if (rank == 2) {    
        master = 2;  
        for(int i = 0; i < cluster2.size(); i++){
            MPI_Send(&master, 1, MPI_INT, cluster2[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster2[i]);
        } 
    } else {
        //Procesele worker primesc rangul coordonatorului clusterului in care se
        //afla si il salveaza in variabila master
        MPI_Recv(&master, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
    }

    /*
        Procesele coordonator pot comunica intre ele si isi trimit unul catre 
        celalalt topologia cunoscuta pentru cluster-ul pe care il coordoneaza
        Initial isi trimit dimensiunile vectorilor cluster0, cluster1 si cluster2
    */
    if(rank == 0){
        MPI_Send(&size_cluster0_recv, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("M(%d,1)\n", rank);
        MPI_Send(&size_cluster0_recv, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("M(%d,2)\n", rank);
        MPI_Recv(&size_cluster1_recv, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&size_cluster2_recv, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

        cluster1_recv.resize(size_cluster1_recv);
        cluster2_recv.resize(size_cluster2_recv);
    }

    if(rank == 1){
        MPI_Send(&size_cluster1_recv, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        printf("M(%d,0)\n", rank);
        MPI_Send(&size_cluster1_recv, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("M(%d,2)\n", rank);

        MPI_Recv(&size_cluster0_recv, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&size_cluster2_recv, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

        cluster0_recv.resize(size_cluster0_recv);
        cluster2_recv.resize(size_cluster2_recv);
    }

    if(rank == 2){
        MPI_Send(&size_cluster2_recv, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        printf("M(%d,0)\n", rank);
        MPI_Send(&size_cluster2_recv, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("M(%d,1)\n", rank);

        MPI_Recv(&size_cluster0_recv, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&size_cluster1_recv, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

        cluster0_recv.resize(size_cluster0_recv);
        cluster1_recv.resize(size_cluster1_recv);
    }

    /*
        Procesele coordonator isi trimit vectorii cluster0, cluster1 si cluster2
    */

    if(rank == 0){
        MPI_Send(cluster0.data(), cluster0.size(), MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("M(%d,1)\n", rank); 
        MPI_Send(cluster0.data(), cluster0.size(), MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("M(%d,2)\n", rank); 

        MPI_Recv(cluster1_recv.data(), size_cluster1_recv, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(cluster2_recv.data(), size_cluster2_recv, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

        for(int i = 0; i < cluster1_recv.size() ; i++){
            cluster1.push_back(cluster1_recv[i]);
        }

        for(int i = 0; i < cluster2_recv.size(); i++){
            cluster2.push_back(cluster2_recv[i]);
        }
    }

    if(rank == 1){
        MPI_Send(cluster1.data(), cluster1.size(), MPI_INT, 0, 0, MPI_COMM_WORLD);
        printf("M(%d,0)\n", rank); 
        MPI_Send(cluster1.data(), cluster1.size(), MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("M(%d,2)\n", rank); 

        MPI_Recv(cluster0_recv.data(), size_cluster0_recv, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(cluster2_recv.data(), size_cluster2_recv, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

        for(int i = 0; i < cluster0_recv.size(); i++){
            cluster0.push_back(cluster0_recv[i]);
        }

        for(int i = 0; i < cluster2_recv.size(); i++){
            cluster2.push_back(cluster2_recv[i]);
        }
    }

    if(rank == 2){
        MPI_Send(cluster2.data(), cluster2.size(), MPI_INT, 0, 0, MPI_COMM_WORLD);
        printf("M(%d,0)\n", rank); 
        MPI_Send(cluster2.data(), cluster2.size(), MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("M(%d,1)\n", rank);  

        MPI_Recv(cluster0_recv.data(), size_cluster0_recv, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(cluster1_recv.data(), size_cluster1_recv, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

        for(int i = 0; i < cluster0_recv.size(); i++){
            cluster0.push_back(cluster0_recv[i]);
        }

        for(int i = 0; i < cluster1_recv.size(); i++){
            cluster1.push_back(cluster1_recv[i]);
        }
    }

    /*
        In acest moment topologia este cunoscuta de catre toate procesele coordonator, iar fiecare
        dintre acestia trimite intreaga topologie catre procesele worker din clusterele pe
        care le coordoneaza
    */

     if (rank == 0) { 
        for(int i = 0; i < cluster0.size(); i++){

            MPI_Send(&size_cluster0_recv, 1, MPI_INT, cluster0[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster0[i]);
            MPI_Send(&size_cluster1_recv, 1, MPI_INT, cluster0[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster0[i]);
            MPI_Send(&size_cluster2_recv, 1, MPI_INT, cluster0[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster0[i]);

            MPI_Send(cluster0.data(), size_cluster0_recv, MPI_INT, cluster0[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster0[i]);
            MPI_Send(cluster1.data(), size_cluster1_recv, MPI_INT, cluster0[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster0[i]);
            MPI_Send(cluster2.data(), size_cluster2_recv, MPI_INT, cluster0[i], 0, MPI_COMM_WORLD);  
            printf("M(%d,%d)\n", rank, cluster0[i]);
        }
        
    } 
    
    if (rank == 1) {   
        for(int i = 0; i < cluster1.size(); i++){

            MPI_Send(&size_cluster0_recv, 1, MPI_INT, cluster1[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster1[i]);  
            MPI_Send(&size_cluster1_recv, 1, MPI_INT, cluster1[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster1[i]);  
            MPI_Send(&size_cluster2_recv, 1, MPI_INT, cluster1[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster1[i]);  

            MPI_Send(cluster0.data(), size_cluster0_recv, MPI_INT, cluster1[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster1[i]);  
            MPI_Send(cluster1.data(), size_cluster1_recv, MPI_INT, cluster1[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster1[i]);  
            MPI_Send(cluster2.data(), size_cluster2_recv, MPI_INT, cluster1[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster1[i]);  
 
        } 
         
    } 
    
     if (rank == 2) {    
        for(int i = 0; i < cluster2.size(); i++){

            MPI_Send(&size_cluster0_recv, 1, MPI_INT, cluster2[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster2[i]);  
            MPI_Send(&size_cluster1_recv, 1, MPI_INT, cluster2[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster2[i]);
            MPI_Send(&size_cluster2_recv, 1, MPI_INT, cluster2[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster2[i]);

            MPI_Send(cluster0.data(), size_cluster0_recv, MPI_INT, cluster2[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster2[i]);
            MPI_Send(cluster1.data(), size_cluster1_recv, MPI_INT, cluster2[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster2[i]);
            MPI_Send(cluster2.data(), size_cluster2_recv, MPI_INT, cluster2[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster2[i]);
   
        }
       
    }


    if(rank != 0 && rank != 1 && rank != 2){
        if(master == 0){
            MPI_Recv(&size_cluster0_recv, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&size_cluster1_recv, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&size_cluster2_recv, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

            cluster0_recv.resize(size_cluster0_recv);
            cluster1_recv.resize(size_cluster1_recv);
            cluster2_recv.resize(size_cluster2_recv);

            MPI_Recv(cluster0_recv.data(), size_cluster0_recv, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(cluster1_recv.data(), size_cluster1_recv, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(cluster2_recv.data(), size_cluster2_recv, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

            for(int i = 0; i < cluster0_recv.size(); i++){
                cluster0.push_back(cluster0_recv[i]);
            }

            for(int i = 0; i < cluster1_recv.size(); i++){
                cluster1.push_back(cluster1_recv[i]);
            }

            for(int i = 0; i < cluster2_recv.size(); i++){
                cluster2.push_back(cluster2_recv[i]);
            }
        }

        if(master == 1){
            MPI_Recv(&size_cluster0_recv, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&size_cluster1_recv, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&size_cluster2_recv, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

            cluster0_recv.resize(size_cluster0_recv);
            cluster1_recv.resize(size_cluster1_recv);
            cluster2_recv.resize(size_cluster2_recv);

            MPI_Recv(cluster0_recv.data(), size_cluster0_recv, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(cluster1_recv.data(), size_cluster1_recv, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(cluster2_recv.data(), size_cluster2_recv, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

            for(int i = 0; i < cluster0_recv.size(); i++){
                cluster0.push_back(cluster0_recv[i]);
            }

            for(int i = 0; i < cluster1_recv.size(); i++){
                cluster1.push_back(cluster1_recv[i]);
            }

            for(int i = 0; i < cluster2_recv.size(); i++){
                cluster2.push_back(cluster2_recv[i]);
            }
        }

        if(master == 2){
            MPI_Recv(&size_cluster0_recv, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&size_cluster1_recv, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&size_cluster2_recv, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

            cluster0_recv.resize(size_cluster0_recv);
            cluster1_recv.resize(size_cluster1_recv);
            cluster2_recv.resize(size_cluster2_recv);

            MPI_Recv(cluster0_recv.data(), size_cluster0_recv, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(cluster1_recv.data(), size_cluster1_recv, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(cluster2_recv.data(), size_cluster2_recv, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

            for(int i = 0; i < cluster0_recv.size(); i++){
                cluster0.push_back(cluster0_recv[i]);
            }

            for(int i = 0; i < cluster1_recv.size(); i++){
                cluster1.push_back(cluster1_recv[i]);
            }

            for(int i = 0; i < cluster2_recv.size(); i++){
                cluster2.push_back(cluster2_recv[i]);
            }
        }
    }

    /*
        In momentul in care topologia este aflata de catre un proces, aceasta este afisata
    */

    if(cluster0.size() + cluster1.size() + cluster2.size() + 3 == numtasks){
        printf("%d -> 0:", rank);
        for(int i = 0; i < cluster0.size() - 1; i++){
            printf("%d,", cluster0[i]);
        }
         printf("%d 1:",  cluster0[cluster0.size() - 1]);
         for(int i = 0; i < cluster1.size() - 1; i++){
            printf("%d,", cluster1[i]);
        }
         printf("%d 2:",  cluster1[cluster1.size() - 1]);
         for(int i = 0; i < cluster2.size() - 1; i++){
            printf("%d,", cluster2[i]);
        }
         printf("%d\n",  cluster2[cluster2.size() - 1]);
    }

 //--------------------- REALIZAREA CALCULELOR --------------------------------------
    /*
        Procesul 0 citeste N si construieste vectorul, calculeaza numarul de iteratii pe care le vor
        efectua procesele si il salveaza in variabila size, iar in cazul in care numarul numarul de
        iteratii nu se imaprte exact la nmarul de procese worker, salvam in extra_size un numar de iteratii
        suplimentare ce vor fi realizate de un proces worker
   */
    if(rank == 0){
        N = atoi(argv[1]);
        //alocare si completare arr
        for(int i = 0; i < N; i++){
            arr.push_back(i);
        }

        size = N / (numtasks - 3);
        extra_size = N % (numtasks - 3);

        arr_size = arr.size();
        arr_size_cluster0 = cluster0.size() * size;
        arr_size_cluster1 = cluster1.size() * size;
        arr_size_cluster2 = cluster2.size() * size + extra_size;

        /*
            Procesul 0 trimite catre procesele 1 si 2 numarul de iteratii,
            dimensiunea portiunii de vector pe care o prelucreaza fiecare
            cluster si portiunea de vector corespunzatoare
        */

        MPI_Send(&size, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("M(%d,1)\n", rank);
        MPI_Send(&size, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("M(%d,2)\n", rank);

        MPI_Send(&extra_size, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("M(%d,1)\n", rank);
        MPI_Send(&extra_size, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("M(%d,2)\n", rank);

        MPI_Send(&arr_size_cluster0, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("M(%d,1)\n", rank);
        MPI_Send(&arr_size_cluster1, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("M(%d,1)\n", rank);
        MPI_Send(&arr_size_cluster2, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("M(%d,1)\n", rank);

        MPI_Send(&arr_size_cluster0, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("M(%d,2)\n", rank);
        MPI_Send(&arr_size_cluster1, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("M(%d,2)\n", rank);
        MPI_Send(&arr_size_cluster2, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("M(%d,2)\n", rank);

        MPI_Send(arr.data() + arr_size_cluster0, arr_size_cluster1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("M(%d,1)\n", rank);
        MPI_Send(arr.data() + arr_size_cluster1 + arr_size_cluster0, arr_size_cluster2, MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("M(%d,2)\n", rank);

        arr.resize(arr_size_cluster0);

    }

    /*
        Procesele 1 si 2 primesc aceste informatii legate de partea de vector
        pe care trebuie sa o prelucreze
    */

    if(rank == 1){
        MPI_Recv(&size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&extra_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&arr_size_cluster0, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&arr_size_cluster1, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&arr_size_cluster2, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        arr_size = arr_size_cluster1;
        arr.resize(arr_size);
        MPI_Recv(arr.data(), arr_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    }
    
    
    if(rank == 2){
        MPI_Recv(&size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&extra_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&arr_size_cluster0, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&arr_size_cluster1, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&arr_size_cluster2, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        arr_size = arr_size_cluster2;
        arr.resize(arr_size);
        MPI_Recv(arr.data(), arr_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    }

    /*
        Fiecare proces coordonator trimite catre procesele worker din clusterul
        in care se afla dimensiunea vectorului pe care trebuie sa o prelucreze un
        proces worker, precum si portiunea de vector corespunzatoare
    */
    if(rank == 0){
        for(int i = 0; i < cluster0.size(); i++){
            size_to_send = size;
            MPI_Send(&size_to_send, 1, MPI_INT, cluster0[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster0[i]);
            MPI_Send(arr.data() + i * size, size_to_send, MPI_INT, cluster0[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster0[i]);
        } 
    }

    if(rank == 1){
        for(int i = 0; i < cluster1.size(); i++){
            size_to_send = size;
            MPI_Send(&size_to_send, 1, MPI_INT, cluster1[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster1[i]);
            MPI_Send(arr.data() + i * size , size_to_send, MPI_INT, cluster1[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster1[i]);
        } 
    }

    if(rank == 2){
        for(int i = 0; i < cluster2.size(); i++){
            if(i == cluster2.size() - 1){
                size_to_send = extra_size + size;
            } else {
                size_to_send = size;
            }
            MPI_Send(&size_to_send, 1, MPI_INT, cluster2[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster2[i]);
            MPI_Send(arr.data() + i * size , size_to_send, MPI_INT, cluster2[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster2[i]);
        } 
    }

    /*
        Procesele worker primesc vectorul pe care trebuie sa il prelucreze, efectueaza calculele
        si trimit vectoril inapoi la procesul lor coordonator
    */

    if(rank > 2){
        MPI_Recv(&iterations_per_worker, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        arr_size = iterations_per_worker;
        arr.resize(arr_size);
        MPI_Recv(arr.data(), iterations_per_worker, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        for(int i = 0; i < arr_size; i++){
            arr[i] *= 2;
        }
        MPI_Send(arr.data(), iterations_per_worker, MPI_INT, master, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, master);
    }

    /*
        Procesele coordonator primesc o cate portiune de vector prelucrata de la fiecare proces
        worker din cluster-ul lor si o asambleaza
        Procesele 1 si 2 trimit portiunea lor catre procesul 0
    */

    if(rank == 0){
        arr.clear();
        vector<int> aux(size);

        for(int i = 0; i < cluster0.size(); i++){
            MPI_Recv(aux.data(), size, MPI_INT, cluster0[i], 0, MPI_COMM_WORLD, &status);
            for(int j = 0; j < size; j++){
                final_arr_cluster0.push_back(aux[j]);
            }
        }

    }

    if(rank == 1){
        arr.clear();
        vector<int> aux(size);

        for(int i = 0; i < cluster1.size(); i++){
            MPI_Recv(aux.data(), size, MPI_INT, cluster1[i], 0, MPI_COMM_WORLD, &status);
            for(int j = 0; j < size; j++){
                final_arr_cluster1.push_back(aux[j]);
            }
        }

          MPI_Send(final_arr_cluster1.data(), final_arr_cluster1.size(), MPI_INT, 0, 0, MPI_COMM_WORLD);
          printf("M(%d,0)\n", rank);
    }

    if(rank == 2){
        arr.clear();
        vector<int> aux;

        for(int i = 0; i < cluster2.size(); i++){
            int recv_size = size;
            if(i == cluster2.size() - 1){
                recv_size += extra_size;
            }
            aux.resize(recv_size);
            MPI_Recv(aux.data(), size + extra_size, MPI_INT, cluster2[i], 0, MPI_COMM_WORLD, &status);
            for(int j = 0; j < recv_size; j++){
                final_arr_cluster2.push_back(aux[j]);
            }
        }

        MPI_Send(final_arr_cluster2.data(), final_arr_cluster2.size(), MPI_INT, 0, 0, MPI_COMM_WORLD);
        printf("M(%d,0)\n", rank);
    }

    /*
        Procesul 0 construieste vectorul final si il afiseaza
    */

    if(rank == 0){
        final_arr_cluster1.resize(arr_size_cluster1);
        final_arr_cluster2.resize(arr_size_cluster2);
        MPI_Recv(final_arr_cluster1.data(), arr_size_cluster1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(final_arr_cluster2.data(), arr_size_cluster2, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

        for(int i = 0; i < arr_size_cluster0; i++){
            final_arr.push_back(final_arr_cluster0[i]);
        }

        for(int i = 0; i < arr_size_cluster1; i++){
            final_arr.push_back(final_arr_cluster1[i]);
        }

        for(int i = 0; i < arr_size_cluster2; i++){
            final_arr.push_back(final_arr_cluster2[i]);
        }

        sleep(1);

        printf("Rezultat: ");

        for(int i = 0; i < final_arr.size(); i++){
            printf("%d ", final_arr[i]);
        }

         printf("\n");
    }

    MPI_Finalize();

}

