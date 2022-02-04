/**
 * Author:	Luca
 * Github:	https://github.com/lucadev23
 * Twitch:	https://www.twitch.tv/lucadav
 * Created:	17.05.2021
 * Modified: 04.02.2022
 * version: 2.0
 **/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include<fcntl.h>
#include "definitions.h"

void contaQuantiJPEG(int*, unsigned int*);
void conservaOffsetJPEG(int*, unsigned int*);
void stampaVettoreOffset(unsigned int*, unsigned int*);
void estrapolaJPEG(int*,unsigned int*, unsigned int*);
void prendiDaQui(FILE *, int, int*);

int main(int argc, char* argv[]){
    unsigned int* offset_jpeg;
    int fileIn; 
    unsigned char buffer='\0';
    int flag=0, conta=1;
    unsigned int offset=0;
    int controllo;
    unsigned int num_jpeg=0;
    unsigned int i;

    if(argc<2){
        fprintf(stderr, "Uso: %s <fileNameToControl>\n",argv[0]);
        exit(EXIT_FAILURE);
    }
    fileIn = open(argv[1],O_RDONLY);
    if(fileIn==-1){
        fprintf(stderr,"Errore apertura file\n");
        exit(EXIT_FAILURE);
    }
    if( lseek(fileIn, 1, SEEK_SET) == ( (off_t) -1) ){
        fprintf(stderr,"Errore posizionamento nel file\n");
        exit(EXIT_FAILURE);
    }
    contaQuantiJPEG(&fileIn, &num_jpeg);
    printf("Conteggio: %d\n", num_jpeg);
    // rimetto il puntatore al primo elemento del file
    if( lseek(fileIn, 1, SEEK_SET) == ( (off_t) -1) ){
        fprintf(stderr,"Errore posizionamento nel file\n");
        exit(EXIT_FAILURE);
    }
    offset_jpeg = (unsigned int*)malloc(sizeof(unsigned int)*num_jpeg);
    if(offset_jpeg==NULL){
        fprintf(stderr, "Impossibile allocare l'area di memoria necessaria\n");
        exit(EXIT_FAILURE);
    }
    conservaOffsetJPEG(&fileIn, offset_jpeg);
    stampaVettoreOffset(offset_jpeg, &num_jpeg);
    estrapolaJPEG(&fileIn, offset_jpeg, &num_jpeg);

    close(fileIn);
    return EXIT_SUCCESS;
}

void contaQuantiJPEG(int* fd, unsigned int* conteggio){
    unsigned int offset=0;
    char buffer;
    int flag=0;

    while(read(*fd, &buffer, 1) != 0){
        offset++;
        switch(buffer){
            case 'J':
                if(flag==0){
                    flag++;
                }
                else{
                    flag=0;
                }
                break;
            case 'F':
                if(flag==1){
                    flag++;
                }
                else if(flag==3){
                    (*conteggio)++;
                    flag=0;
                }
                else{
                    flag=0;
                }
                break;
            case 'I':
                if(flag==2){
                    flag++;
                }
                else{
                    flag=0;
                }
                break;
            default:
                flag=0;
                break;
        }
    }
}

void conservaOffsetJPEG(int* fd, unsigned int* vettoreOffset){
    unsigned int offset=0;
    unsigned int conteggio=0;
    char buffer;
    int flag=0;

    while(read(*fd, &buffer, 1) != 0){
        offset++;
        switch(buffer){
            case 'J':
                if(flag==0){
                    flag++;
                }
                else{
                    flag=0;
                }
                break;
            case 'F':
                if(flag==1){
                    flag++;
                }
                else if(flag==3){
                    //printf("Offset-->%d\n", (offset-3));
                    vettoreOffset[conteggio]=(offset-3);
                    conteggio++;
                    flag=0;
                }
                else{
                    flag=0;
                }
                break;
            case 'I':
                if(flag==2){
                    flag++;
                }
                else{
                    flag=0;
                }
                break;
            default:
                flag=0;
                break;
        }
    }
}

void stampaVettoreOffset(unsigned int* vettoreOffset, unsigned int* dimensioneVettore){
    unsigned int i;
    for(i=0;i<(*dimensioneVettore);i++){
        fprintf(stdout,"Offset[%d]--> %d\n", i, vettoreOffset[i]);
    }
}

void estrapolaJPEG(int* fd,unsigned int* vettore, unsigned int* dimensione){
    unsigned int offset=0;
    unsigned int i=0;
    char fileName[10]; // sto ipotizzando 5 caratteri usati per il '.jpeg' e altri 5 per le possibili 9999 foto + 1 carattere per il '\0'
    int fileOut;
    unsigned char buffer='\0';
    const unsigned char ff = 0b11111111;
    const unsigned char d9 = 0b11011001;
    const unsigned char d8 = 0b11011000;
    const unsigned char e0 = 0b11100000;
    const unsigned char b00 = 0b00000000;
    const unsigned char b10 = 0b00010000;
    const unsigned char j = 0b01001010;
    const unsigned char f = 0b01000110;
    const unsigned char bi = 0b01001001;
    bool flagFF=false;

    for(i=0;i<(*dimensione);i++){
        memset(fileName,'\0',10);
        sprintf(fileName,"%d", i);
        strcat(fileName, ".jpeg");
        buffer='\0';
        flagFF=false;
        if( lseek(*fd, vettore[i], SEEK_SET) == ( (off_t) -1) ){
            fprintf(stderr,"Errore posizionamento nell'offset %d per l'immagine %d\n", vettore[i], i);
            continue;
        }
        offset=vettore[i];
        fileOut=open(fileName,O_CREAT | O_TRUNC | O_WRONLY,0666);
        if(fileOut==-1){
            fprintf(stderr,"Errore creazione file %d.jpeg\n",i);
            continue;
        }
        write(fileOut,&ff,(size_t)sizeof(unsigned char));
        write(fileOut,&d8,(size_t)sizeof(unsigned char));
        write(fileOut,&ff,(size_t)sizeof(unsigned char));
        write(fileOut,&e0,(size_t)sizeof(unsigned char));
        write(fileOut,&b00,(size_t)sizeof(unsigned char));
        write(fileOut,&b10,(size_t)sizeof(unsigned char));
        write(fileOut,&j,(size_t)sizeof(unsigned char));
        write(fileOut,&f,(size_t)sizeof(unsigned char));
        write(fileOut,&bi,(size_t)sizeof(unsigned char));
        write(fileOut,&f,(size_t)sizeof(unsigned char));
        while(read(*fd, &buffer, 1) != 0){
            offset++;
            write(fileOut,&buffer,(size_t)sizeof(unsigned char));
            if(buffer==FF)
                flagFF=true;
            else    
                if(flagFF==true && buffer==D9)
                    break;
                else
                    flagFF=false;
            /*
            * questo è il caso in cui un file jpeg non tiene le intestazioni di fine
            * file, perciò il programma prosegue fin quando non ne trova uno
            * andando anche a "prendere" l'immagine successiva.
            * Perciò catturo questo fenomeno e lo controllo andandolo a bloccare.
            */ 
            if(i<((*dimensione)-1) && offset==vettore[i+1]){
                fprintf(stderr,"Usato break per foto %s\n", fileName);
                break;
            }
            /*
            * Se però invece è l'ultima immagine a non avere l'intestazione di fine file,
            * in questa versione non controllerò questo fenomeno, e lascerò che
            * il programma continui a copiare tutto il contenuto del file fino alla fine.
            */
        }
        close(fileOut);
        printf("File %d.jpeg creato!\n", i);
    }
    
}
