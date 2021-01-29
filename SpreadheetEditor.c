//AUTHOR : HAMORA HADI, YEHEZKIEL TATO

#include<stdio.h>
#include<string.h>
#include<stdlib.h>

struct text_32b {
    char value[32]; 
};

struct text_128b {
    char value[128]; 
};

struct text_1k {
    char value[1024]; 
};

struct text_64k {
    char value[65536]; 
};

const int VIEW = 0;
const int EDIT = 1;

typedef struct text_32b str32;
typedef struct text_128b str128;
typedef struct text_1k str1k;
typedef struct text_64k str64k;

int file_action();
int file_ask(int edit);
int file_exists(char file_name[128]);
int file_parse(char file_name[128], int mode);
int file_create();
int file_display(char file_name[128], int row_count, int column_count,int column_width[32], str32 columns[32], str32 row_data[512][32]);
int file_edit(char file_name[128], int row_count, int column_count,int column_width[32], str32 columns[32], str32 row_data[512][32]);
int file_save(char file_name[128], int row_count, int column_count,int column_width[32], str32 columns[32], str32 row_data[512][32]);

int main(){
    printf("Spreadsheet Manager\n\nBy : sammyon7,Yehezk34\n\n");
    while(file_action()){}
}

int file_action(){
    int action;

    printf("\nWhat Do You Want To Do?\nChoose by select the number!\n");
    printf("\n[ 1 ] Create Spreadsheet");
    printf("\n[ 2 ] Edit Spreadsheet");
    printf("\n[ 3 ] View Spreadsheet");
    printf("\n[ 4 ] Exit Program");
    printf("\n-> ");

    fflush(stdin);
    while(scanf("%d", &action)){
        if(action == 1 || action == 2 || action == 3 || action == 4){
            if(action == 1){
                file_create();
                break;
            }
            if(action == 2){
                file_ask(EDIT);
                break;
            }
            if(action == 3){
                file_ask(VIEW);
                break;
            }
            if(action == 4){
                printf("\nThank You for using Spreadsheet Manager Lite...");
                printf("\n\n");
                exit(EXIT_SUCCESS);
            }
        }
        printf("\nInvalid Input!");
        printf("\n> ");
        fflush(stdin);
    }
}

int file_ask(int mode){
    char f_name[128];

    reload_file:
    printf("\nEnter File Name: ");
    fflush(stdin); 
    scanf("%[^\n]s", f_name);
    if(!file_exists(f_name)){
        printf("\nError: File Does Not Exists!\n");
        goto reload_file;
    }
	else{
        if(mode == VIEW){
            file_parse(f_name, VIEW);
        }
        if(mode == EDIT){
            file_parse(f_name, EDIT);
        }
    }
}

int file_exists(char file_name[128]){
    FILE *fp;

    if((fp = fopen(file_name, "r")) != NULL){
        fclose(fp);
        return 1;
    }
	else{
        return 0;
    }
}

int file_parse(char file_name[128], int mode){
    FILE *fp;
    char buffer;
    int i = 0, j = 0;
    int file_size = 0;
    struct text_64k file_data;
    char *data;

    int line_count = 0;
    int row_count = 0;
    int column_count = 0;
    int c_num = 0; 
    int column_width[32]; 

    str1k data_pack[512];
    str32 columns[32]; 
    str32 row_data[512][32]; 

    if((fp = fopen(file_name, "r")) != NULL){
        while((buffer = getc(fp)) != EOF){
            file_data.value[file_size] = buffer;
            file_size++;
        }
        fclose(fp);
        
        data = strtok(file_data.value, "\n");
        while(data != NULL){
            strcpy(data_pack[line_count].value, data);
            data = strtok(NULL, "\r\n");
            line_count++;
        }

        for(i = 0; i < line_count; i++){
            data = strtok(data_pack[i].value, ",");
            c_num = 0;
            while(data != NULL){
                if(i == 0){
                    strcpy(columns[column_count].value, data);
                    column_width[column_count] = strlen(data);
                    column_count++;
                }
				else{
                    strcpy(row_data[i-1][c_num].value, data);
                    if(strlen(data) > column_width[c_num]){
                        column_width[c_num] = strlen(data);
                    }
                }

                data = strtok(NULL, ","); 
                c_num++;
            }
            row_count = i - 1;
        }
    }

    if(mode == VIEW || mode == EDIT){
        file_display(file_name, row_count, column_count,
                     column_width, columns, row_data);
    }

    if(mode == EDIT){
        file_edit(file_name, row_count, column_count,
                  column_width, columns, row_data);
    }

    return 1;
}

int file_display(char file_name[128], int row_count, int column_count,int column_width[32], str32 columns[32], str32 row_data[512][32]){
    int i, j;

    printf("\n");
    for(i = 0; i < row_count + 2; i++){ 
        if(i == 0){
            printf("\n     |");
            for(j = 0; j < column_count; j++){
                printf("-%.*s-|", column_width[j], "-------------------------------");
            }

            printf("\n Col |");
            for(j = 0; j < column_count; j++){
                printf(" %c", j + 65);
                printf("%.*s |", column_width[j] - 1, "                                ");
            }

            printf("\n     |");
            for(j = 0; j < column_count; j++){
                printf("-%.*s-|", column_width[j], "-------------------------------");
            }
            printf("\n Row |");
            for(j = 0; j < column_count; j++){
                printf(" %s ", columns[j].value);
                printf("%.*s|", column_width[j] - strlen(columns[j].value), "                                ");
            }
            printf("\n     |");
            for(j = 0; j < column_count; j++){
                printf("-%.*s-|", column_width[j], "-------------------------------");
            }
        }
		else{

            printf("\n");
            
            if(i > 0 && i < 10){printf(" %d   ", i);}
            if(i > 9 && i < 100){printf(" %d  ", i);}
            if(i > 99 && i < 1000){printf(" %d ", i);}
            if(i > 999 && i < 10000){printf(" %d", i);}

            printf("|");
            
            for(j = 0; j < column_count; j++){
                printf(" %s ", row_data[i-1][j].value);
                printf("%.*s|", column_width[j] - strlen(row_data[i-1][j].value), "                                ");
            }
            printf("\n     |");
            for(j = 0; j < column_count; j++){
                printf("-%.*s-|", column_width[j], "-------------------------------");
            }
        }
    }

    printf("\n\n");
    return 1;
}

int file_create(){
	
    int i;
    char y_n[8];
    char file_name[128];
    char column_name[32];
    int row_count;
    int column_count;
    int column_width[32];
    str32 columns[32];
    str32 row_data[512][32];

    file_create_reload:
    printf("\nFile Name: ");
    fflush(stdin); 
    scanf("%[^\n]s", file_name);
    if(file_exists(file_name)){
        printf("\nFile Already Exists. Override it?");
        printf("\ny / n > ");
        fflush(stdin);
        scanf("%[^\n]s", y_n);
        if(!strcmp(y_n, "y") || !strcmp(y_n, "Y")){
            goto file_create_reload;
        }
    }

    repeat_column_count:
    printf("\nNumber Of Columns: ");
    fflush(stdin);
    scanf("%d", &column_count);
    if(column_count < 1){
        printf("\nError: Invalid Column Count!\n");
        goto repeat_column_count;
    }

    printf("\nEnter Column Names: \n");
    for(i = 0; i < column_count; i++){
        repeat_column_name:
        printf("%c: ", i + 65);
        fflush(stdin);
        scanf("%[^\n]s", column_name);
        if(strlen(column_name) > 0){
            strcpy(columns[i].value, column_name);
            column_width[i] = strlen(column_name);
        }
		else{
            printf("\nError: Invalid Column Name\n");
            goto repeat_column_name;
        }
    }
    row_count = -1;

    file_edit(file_name, row_count, column_count,
              column_width, columns, row_data);

    return 1;
}

int file_edit(char file_name[128], int row_count, int column_count,int column_width[32], str32 columns[32], str32 row_data[512][32]){

    int i, j;
    char u_inp1[8];
    char u_inp2[32];
    char row_inp[4];
    char save_as_file[128];
    char y_n[8];
    int column_id;
    int row_id;
    str32 field_data;
    str32 blank_row[32];
    str32 temp_row[32];

    show_edit_help:
    printf("\nEnter Field Name [ e.g: A1, B17, D52 ] to edit");
    printf("\nType \"addrow\" to add new row.");
    printf("\nType \"remrow\" to remove a row.");
    printf("\nType \"remcol\" to remove a column.");
    printf("\nType \"save\" to save to file.");
    printf("\nType \"saveas\" to save as a new file.");
    printf("\nType \"show\" to show current data table.");
    printf("\nType \"help\" to show this command list.");
    printf("\nType \"exit\" to go back.");

    printf("\n\n> ");
    fflush(stdin);
    while(scanf("%s", u_inp1)){
        if(strcmp(u_inp1, "addrow") == 0){
            add_new_row:
            	
            fflush(stdin);
            printf("\nRow Position: ");
            scanf("%d", &row_id);
            printf("\n");

            if(row_id >= 1 && row_id <= row_count + 2){
                for(i = 0; i < column_count; i++){
                    printf("[ %c ] %s: ", i + 65, columns[i].value);
                    fflush(stdin);
                    scanf("%[^\n]s", u_inp2);
                    strcpy(blank_row[i].value, u_inp2);
                }

                if(row_id == row_count + 2){
                    for(i = 0; i < column_count; i++){
                        row_data[row_id - 1][i] = blank_row[i];
                        if(strlen(blank_row[i].value) > column_width[i]){
                            column_width[i] = strlen(blank_row[i].value);
                        }
                    }
                }
				else{
                    for(i = row_count; i > row_id - 2; i--){
                        for(j = 0; j < column_count; j++){
                            row_data[i+1][j] = row_data[i][j];
                        }
                    }
                    for(i = 0; i < column_count; i++){
                        row_data[row_id - 1][i] = blank_row[i];
                        if(strlen(blank_row[i].value) > column_width[i]){
                            column_width[i] = strlen(blank_row[i].value);
                        }
                    }
                }

                row_count++;
                printf("\nNew Row Added At [ %d ]\n", row_id);
            }
			else{
                printf("\nError: Invalid Row Number!\n");
                goto add_new_row;
            }
        }else if(strcmp(u_inp1, "remrow") == 0){
            remove_row:
            	
            fflush(stdin);
            printf("\nRow Position: ");
            scanf("%d", &row_id);

            if(row_id >= 1 && row_id <= row_count + 1){
            	
                for(i = row_id; i < row_count + 1; i++){
                    for(j = 0; j < column_count; j++){
                        row_data[i-1][j] = row_data[i][j];
                    }
                }

                row_count--;
                printf("Row Deleted [ %d ]\n", row_id);
            }
			else{
                printf("\nError: Invalid Row Number!\n");
                goto remove_row;
            }
        }
		else if(strcmp(u_inp1, "remcol") == 0){
            remove_col:
            	
            fflush(stdin);
            printf("\nColumn Alphabet: ");
            scanf("%c", &u_inp2);

            if(u_inp2[0] > 96 && u_inp2[0] < 123){ 
                u_inp2[0] = u_inp2[0] - 32;
            }
            if(u_inp2[0] > 64 && u_inp2[0] < 91){ // A - Z
                column_id = u_inp2[0] - 65;
            }
			else{
                printf("\nError: Invalid Column Alphabet!\n");
                goto remove_col;
            }


            if(column_id >= 0 && column_id < column_count){
            	
                if(column_id < column_count - 1){
                    for(j = column_id; j < column_count - 1; j++){
                        columns[j] = columns[j+1];
                    }
                }
                
                for(i = 0; i < row_count; i++){
                    if(column_id < column_count - 1){
                        for(j = column_id; j < column_count - 1; j++){
                            row_data[i][j] = row_data[i][j+1];
                        }
                    }
                }

                column_count--;
                printf("Column Deleted [ %c ]\n", column_id + 65);
            }
			else{
                printf("\nError: Invalid Column Alphabet!\n");
                goto remove_col;
            }
        }
		else if(strcmp(u_inp1, "save") == 0){
            printf("\nSaving File...\n");
            file_save(file_name, row_count, column_count,
                      column_width, columns, row_data);
            break;
        }
		else if(strcmp(u_inp1, "saveas") == 0){
            file_save_as:
            printf("\nSave File As: ");
            fflush(stdin); 
            scanf("%[^\n]s", save_as_file);
            if(file_exists(save_as_file)){
                printf("\nFile Already Exists. Override it?");
                printf("\ny / n > ");
                fflush(stdin);
                
                scanf("%[^\n]s", y_n);
                if(strcmp(y_n, "y") || strcmp(y_n, "Y")){
                    file_save(save_as_file, row_count, column_count,
                              column_width, columns, row_data);
                }
				else{
                    goto file_save_as;
                }
            }
			else{
                file_save(save_as_file, row_count, column_count,
                          column_width, columns, row_data);
            }
            break;
        }
		else if(strcmp(u_inp1, "exit") == 0){
            printf("\nExiting Edit Mode...\n");
            break;
        }
		else if(strcmp(u_inp1, "show") == 0){
            file_display(file_name, row_count, column_count,
                     column_width, columns, row_data);
        }
		else if(strcmp(u_inp1, "help") == 0){
            goto show_edit_help;
        }
		else{
            if(u_inp1[0] > 96 && u_inp1[0] < 123){ 
                u_inp1[0] = u_inp1[0] - 32; 
            }
            
            if(u_inp1[0] > 64 && u_inp1[0] < 91){ 
                column_id = u_inp1[0] - 65;
            }
			else{
                printf("\nError: Invalid Command!\n");
                goto skip_edit_command;
            }

            if(column_id > column_count){
                printf("\nError: Column Does Not Exists!\n");
                goto skip_edit_command;
            }
            
            u_inp1[0] = 48;
            row_id = atoi(u_inp1) - 1;

            if(atoi(u_inp1) == 0){
                printf("\nError: Invalid Command!\n");
                goto skip_edit_command;
            }
            if(row_id > row_count){
                printf("\nError: Row Does Not Exists!\n");
                goto skip_edit_command;
            }

            field_data = row_data[row_id][column_id];
            printf("\n%c%d: %s", column_id + 65, row_id + 1, field_data.value);
            printf("\nEnter New Value: ");
            fflush(stdin); 
            scanf("%[^\n]s", u_inp2);

            if(strcpy(row_data[row_id][column_id].value, u_inp2)){
                printf("%c%d Value Modified.\n", column_id + 65, row_id + 1);
                if(strlen(u_inp2) > column_width[column_id]){
                    column_width[column_id] = strlen(u_inp2);
                }
            }
        }

        skip_edit_command:
        	
        fflush(stdin);
        printf("\n> ");
    }
    return 1;
}

int file_save(char file_name[128], int row_count, int column_count,int column_width[32], str32 columns[32], str32 row_data[512][32]){

    FILE *fp;
    int i, j;
    str64k file_data;
    
    strcpy(file_data.value, "");
    for(i = 0; i < column_count; i++){
        if(i > 0){
            strcat(file_data.value, ",");
        }
        strcat(file_data.value, columns[i].value);
    }

    for(i = 0; i < row_count + 1; i++){
        strcat(file_data.value, "\r\n"); 
        for(j = 0; j < column_count; j++){
            if(j > 0){
                strcat(file_data.value, ",");
            }
            strcat(file_data.value, row_data[i][j].value);
        }
    }

    printf("\n\nFile Name: %s", file_name);

    if((fp = fopen(file_name, "wb")) != NULL){
        fprintf(fp, file_data.value);
        fclose(fp);
        printf("\nSuccessfully Saved.");
    }
	else{
        printf("\nError Saving File.");
    }

    printf("\n");
    return 1;
}
