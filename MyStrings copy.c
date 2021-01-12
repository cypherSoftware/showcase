#include <stdio.h>

// this function checks to see if a character is printable (space included)
int is_printable_char(char c){


    if (c >= 32 && c <= 126){       //if between 32 and 126 then its valid return 1 for true
        return 1;
    }
    else {                          //if not return 0 for false
        return 0;
    }
}

int main(int argc, char* argv[]) {

    FILE *file;
    char four_byte_chunk[4];
    char a_char;
    long position_in_file;
    short from_fread;



    if (argc != 2) {         //check for proper number of args
        printf("Did not provide proper number of ARGS\n");
        return (-1);
    }


    file = fopen(argv[1], "rb");        //open file in binary mode to be read



    if (file == NULL) {         //throw error if can't open file
        perror("Error opening file");
        return (-1);

    }


        do {
            //get seeker location each loop in case you don't get a 4 character run of printable characters
            position_in_file = ftell(file);

            //get 4 bytes from fread() and add it to the from_fread variable
            from_fread = fread(&four_byte_chunk, 1, 4, file);

            //check from_fread variable to make sure we got the minimum 4 bytes for a string if not then EOF
            if (from_fread != 4) {
                printf("\n end of file \n");
                return 0;                       //this will break while loop if EOF


            } else {           //this section checks 4bytes for printable characters
                                // examine each and determine if between interval [32,126]


                if (is_printable_char(four_byte_chunk[0]) && is_printable_char(four_byte_chunk[1])
                        && is_printable_char(four_byte_chunk[2]) && is_printable_char(four_byte_chunk[3])) {

                                //successfully found 4 byte chunk of printables characters
                                // let's rewind and print them out then continue until non printable character is found

                    fseek(file, position_in_file, SEEK_SET);    //rewind
                    while (!feof(file) && from_fread > 0) {     //print loop
                        from_fread = fread(&a_char, 1, 1, file); //read in a char
                        if (is_printable_char(a_char)) { //if printable, print it
                            printf("%c", a_char);
                        } else {
                            printf("\n"); //hit non printable character so new line for new string and break while loop.
                            break;
                        }
                    }
                } else { // first four bytes were not all printable charcters now advance one byte at a time until
                         //a printable character is found then try again

                    fseek(file, ftell(file) - 3, SEEK_SET);     //back up three instead of 4 to advance forward a char
                }

            }
        } while (1);

    }
