/**
 * Author:	Luca
 * Github:	https://github.com/lucadev23
 * Twitch:	https://www.twitch.tv/lucadav
 * Created:	17.05.2021
 **/

#include <stdio.h>
#include <stdbool.h>

int main(void) { 
    FILE *fileIn, *fileOut;
    unsigned char buffer;
    unsigned char ff = 0b11111111;
    unsigned char d9 = 0b11011001;
    unsigned char d8 = 0b11011000;
    bool flagFF=false;


    fileIn = fopen("Sample Unallocated Clusters", "rb");
    fileOut = fopen("out","wb");         
    fseek(fileIn, 172833, SEEK_SET); 
    /* 
    il valore 172833 è l'offset, ovvero la posizione, della coppia FF E0
    all'interno del file
    */

    fwrite(&ff,1,1,fileOut);
    fwrite(&d8,1,1,fileOut);
    fwrite(&ff,1,1,fileOut);
    while(fread(&buffer, 1, 1, fileIn) != EOF){
        fwrite(&buffer,1,1,fileOut);
        if(buffer==ff)
            flagFF=true;
        else    
            if(flagFF==true && buffer==d9)
                break;
            else
                flagFF=false;
    }

    fclose(fileIn);
    fclose(fileOut);

    return 0;
}
