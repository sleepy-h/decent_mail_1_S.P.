#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include <ctype.h>

#define RAND_SEEK 0xDEADC0DE

#define swap(a,b) ({typeof(a) temp = a; a = b; b = temp;})
// struct for parameters 
typedef struct{
	uint64_t quantity_of_bilets;
	int64_t parameter_for_generator;
	FILE* file_with_students;
} parameters;

typedef struct{
	char* full_name;
	uint64_t pos_in_file;
	uint64_t bilet;
} student;

typedef struct{
	student** students;
	uint64_t quantity_of_students;
} students;

//how much now whitespaced strings in file
uint64_t fcheck_not_whitespace_str(FILE* fileptr){
	uint64_t result = 0;
	while(!feof(fileptr)){
		while(isspace(fgetc(fileptr)) && !feof(fileptr));
		if(!feof(fileptr)){
			++result;
			while(((fgetc(fileptr)) != '\n')  && !feof(fileptr));
		}
	}
	fseek(fileptr, 0L, SEEK_SET);
	return result;
}

//Initializator for parameters structure and processing parameters from command line
parameters* create_parameters(int argc, char const *argv[]){
	if (argc == 1){
        //Error: program without arguments
		return NULL;
	}
	parameters* parameter = (parameters *)malloc(sizeof(parameters));
	parameter->quantity_of_bilets = (uint64_t)0;
    parameter->parameter_for_generator = 0;
	for (int i = 1; i < (argc - 1); i+=2){
		if (argv[i][0] != '-'){
			// Error: Wrong command
			return NULL;
		} else {
			switch(argv[i][1]){
				case 'n':
					//if (strlen(argv[i]) == 2){
					if (argv[i][2] == '\0'){
						parameter->quantity_of_bilets = atoi(argv[i+1]);
					} else {
						// Error: Wrong command
						return NULL;
					}
					break;
				case 'g':
					//if (strlen(argv[i]) == 2){
					if (argv[i][2] == '\0'){
						parameter->parameter_for_generator = atoi(argv[i+1]);
					} else {
						// Error: Wrong command
						return NULL;
					}
					break;
				default:
					// Error: Wrong command
					return NULL;
					break;
			}
		}
	}
	parameter->file_with_students = fopen(argv[argc-1],"r");
	if (parameter->file_with_students == NULL)
		// Error: File is not created
		return NULL;
	return parameter;
}

student* create_student(char* full_name,uint64_t pos_in_file){
	student* new_student = (student*)malloc(sizeof(student));
	new_student->full_name = full_name;
	new_student->pos_in_file = pos_in_file;
	new_student->bilet = (uint64_t)0;
	return new_student;
}

students* create_students(uint64_t quantity){
	students* new_students = (students*)malloc(sizeof(students));
	new_students->students = (student**)malloc(quantity * sizeof(student*));
	new_students->quantity_of_students = quantity;
	return new_students;
}

void destruct_student(student* student){
	free(student->full_name);
	free(student);
}

void destructs_students(students* students){
	for (int i = 0; i < students->quantity_of_students; ++i)
		destruct_student(students->students[i]);
	free(students->students);
	free(students);
}

// clear all dynamitic memory  in parameters
void destruct_parameters(parameters* parameters){
	fclose(parameters->file_with_students);
	free(parameters);
}

// function for write help
void help(char const* name_of_programm){
	printf("Usage: %s [options] file...\n", name_of_programm);
    printf("Options:\n");
    printf("  -g                       Parameters for pseudo generation random.\n");
    printf("  -n                       Quantity of bilets.\n");
}

void paste_fullname_from_file(students* students,FILE* file_with_students){
    while(!feof(file_with_students)){
		for(uint64_t i = 0;!feof(file_with_students); ++i){
			char current_symbol;
			bool previous_is_space = false;
			uint64_t lenght_of_full_name = 0;
			uint64_t lenght_in_fn_in_file = 0;
			while (isspace(current_symbol = fgetc(file_with_students)) && current_symbol != EOF);
			if (current_symbol == EOF) continue;
			++lenght_of_full_name;
			++lenght_in_fn_in_file;
			bool has_str = false;
			while (((current_symbol = fgetc(file_with_students)) != '\n')  && current_symbol != EOF){
				if (isspace(current_symbol)){
					previous_is_space = true;
				} else { 
					if (previous_is_space) {
						++lenght_of_full_name;
						previous_is_space = false;
					}
					++lenght_of_full_name;
					has_str = true;
				}
				++lenght_in_fn_in_file;
			}
            if (current_symbol == EOF){
                --lenght_of_full_name;
                --lenght_in_fn_in_file;
            }
            char* full_name = (char*)malloc((lenght_of_full_name + 1) * sizeof(char));
            uint64_t pos_in_fullname = 0;
			if(has_str){
				fseek(file_with_students, (-1) * (lenght_in_fn_in_file+1), SEEK_CUR);
				while(((current_symbol = fgetc(file_with_students)) != '\n')  && current_symbol != EOF){
					if (isspace(current_symbol)){
						previous_is_space = true;
					} else { 
						if (previous_is_space) {
							full_name[pos_in_fullname++] = ' ';
							previous_is_space = false;
						}
						full_name[pos_in_fullname++] = current_symbol;
						has_str = true;
					}
				}
			} else {
                free(full_name);
            }
			full_name[pos_in_fullname] = '\0';
			students->students[i] = create_student(full_name, i);
		}
	}
}

void sorting_students_for_fullname(students* students){
	for (uint64_t i = students->quantity_of_students / 2; i ; i >>= 1){
		for (uint64_t j = i; j < students->quantity_of_students; ++j){
			for (uint64_t k = j; k >= i && strcmp(students->students[k - i]->full_name, students->students[k]->full_name) > 0; k -= i){
				swap(students->students[k - i], students->students[k]);
			}
		}
	}
}

void sorting_students_for_filepos(students* students){
	for (uint64_t i = students->quantity_of_students / 2; i ; i >>= 1){
		for (uint64_t j = i; j < students->quantity_of_students; ++j){
			for (uint64_t k = j; k >= i && students->students[k - i]->pos_in_file >  students->students[k]->pos_in_file; k -= i){
				swap(students->students[k - i], students->students[k]);
			}
		}
	}
}

void gen_bilets(students* students, uint64_t quantity_of_bilets){
    //Create list numbers status
	bool* numbers_status = (bool*)calloc(quantity_of_bilets, sizeof(bool));

	// bool for optimization search not used number of bilets
	bool status_now = true;

	for (uint64_t i; i < students->quantity_of_students;){
        for (uint64_t j = 0; j < quantity_of_bilets; ++j, ++i){
            uint64_t k = rand() % quantity_of_bilets;
            for(; numbers_status[k] == status_now; k = (k + 1) %     quantity_of_bilets);
            students->students[i]->bilet = k + 1;
            numbers_status[k] = status_now;
		}
		status_now = !status_now;
	}
    free(numbers_status);
}

void printf_students_bilets(students* students){
    for(uint64_t i = 0; i < students->quantity_of_students; ++i){
		printf("%s: %lu\n", students->students[i]->full_name, students->students[i]->bilet);
	}
}

int main(int argc, char const *argv[]){
	//Processing parameters
	parameters* parameters = create_parameters(argc,argv);
	if (parameters == NULL){
		help(argv[0]);
		return 0;
	}

	// Create students struct
	students* students = create_students(fcheck_not_whitespace_str(parameters->file_with_students));
    //Check file for errors with content(empty file or whitespace)
	if (students->quantity_of_students == 0){
		//Error: File without students
		help(argv[0]);
		return 0;
	} else if (parameters->quantity_of_bilets == 0) {
        //If user don give quantity parameter set quantity_of_bilets = quantity_of_students
		parameters->quantity_of_bilets = students->quantity_of_students;
	}

	//Initialization random
	srand(RAND_SEEK + parameters->parameter_for_generator);

    //Processing file and make strings with fullname 
	paste_fullname_from_file(students,parameters->file_with_students);

    //sorting students for full name
	sorting_students_for_fullname(students);

    //generating bilets numbers
    gen_bilets(students, parameters->quantity_of_bilets);

    //return position of students how in file
    sorting_students_for_filepos(students);
    
    //print students fullname and their bilets
    printf_students_bilets(students);

    //clear dynamical memory
	destructs_students(students);
	destruct_parameters(parameters);
	return 0;
}