#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>   
#include <string.h>
#include <stdlib.h>
#include <locale.h> 
#include <time.h>
#include <assert.h>
#include <ctype.h>

#define MAX_STR_SIZE 200

enum work_mode { all, start, end, change };
struct target {
	int count;
	int count_max;
	enum work_mode mode;
	char* word;
	char* replacement;
};


void help() {
	printf("Instruction:\n"
		"\tType -config or use key [ -с ] for passing the path to the config file.\n"
		"\tType -input or use key [ -i ] for passing the path to the input file.\n"
		"\tType -output or use key [ -o ] for passing the path to the output file.\n"
		"\tType -help or use key [ -h ] to open inctruction.\n");
	exit(1);
}
int cfgfile_lines_count(FILE* config) {
	int count = 0;
	char c = fgetc(config);
	while (c != EOF) {
		if (c == '\n') { count++; }
		c = fgetc(config);
	}
	count++;
	rewind(config);
	return count;
}
int size_of_inputfile(FILE* input) {
	int len = 0;
	fseek(input, 0, SEEK_END);
	len = ftell(input);
	rewind(input);
	return len;
}

struct target word_from_config(struct target curr_target, FILE* config) {		//заполняет структуру target словом из конфиг файла, где содержится само слово и как его надо удалить
	char* str = (char*)calloc(MAX_STR_SIZE, sizeof(char));
	assert(str != NULL);
	int len1 = 0;
	int i = 0;
	fgets(str, MAX_STR_SIZE, config);
	for (int j = 0; j < MAX_STR_SIZE; j++) {
		if (*(str + j) == '\n') {
			*(str + j) = '\0';
			break;
		}
	}
	len1 = (int)strlen(str);
	char* changePtr = strchr(str, '^');
	if (changePtr != NULL) {
		curr_target.mode = change;
		int wordLen = changePtr - str;
		curr_target.word = (char*)calloc(wordLen + 1, sizeof(char));
		strncpy(curr_target.word, str, wordLen);
		curr_target.word[wordLen] = '\0';
		int replacementLen = len1 - wordLen - 1;
		curr_target.replacement = (char*)calloc(replacementLen + 1, sizeof(char));
		strncpy(curr_target.replacement, changePtr + 1, replacementLen);
		curr_target.replacement[replacementLen] = '\0';
		return curr_target;
	}
	if (isdigit(str[0])) {
		curr_target.mode = start;
		curr_target.count_max = 0;
		curr_target.count = (int)atoi(str);
		if (curr_target.count == 0) {
			curr_target.count = 1;
		}
		for (i = 0; i < len1; i++) {
			if (!isdigit(*(str + i))) {
				break;
			}
		}
		*(str + len1 - 1) = '\0';
		len1 -= i;
		curr_target.word = (char*)calloc(len1 + 1, sizeof(char));
		strcpy(curr_target.word, str + 1 + i);
		free(str);
		return curr_target;
	}
	if (len1 > 0 && isdigit(*(str + len1 - 1))) { // Проверяем, что строка не пустая
		curr_target.mode = end;
		curr_target.count_max = 0;
		int i = len1 - 1;

		// Находим позицию первого нецифрового символа с конца
		while (i >= 0 && isdigit(*(str + i))) {
			i--;
		}

		// Преобразуем строку в число
		curr_target.count = atoi(str + i + 1);
		if (curr_target.count == 0) {
			curr_target.count = 1; // Если count равен 0, устанавливаем в 1
		}

		*(str + i) = '\0'; // Убираем символ % в конце
		len1 = i + 2; // Обновляем длину строки для выделения памяти

		// Выделяем память под curr_target.word и копируем строку
		curr_target.word = (char*)calloc(len1, sizeof(char));
		if (curr_target.word != NULL) {
			strcpy(curr_target.word, str + 1); // Копируем строку начиная со второго символа
		}

		free(str); // Освобождаем память для str
		return curr_target;
	}


	else {
		curr_target.mode = all;
		curr_target.count = 0;
		curr_target.count_max = 0;
		curr_target.word = (char*)calloc(len1 + 1, sizeof(char));
		strcpy(curr_target.word, str);
		free(str);
		return curr_target;
	}
}

void current_progress(int file_size, int file_where) {
	float file_persents = ((float)file_where / (float)file_size) * 100;
	system("cls");
	printf("Current processing: %d%%", (int)file_persents);
}

void deleting(struct target* curr_target, int curr_target_count, FILE* input, FILE* output) {
	int file_size = size_of_inputfile(input);
	char* str = NULL;
	char** str_curr_targets = (char**)malloc(curr_target_count * sizeof(char*));
	if (str_curr_targets == NULL) { exit(0); }
	int flag = 0;
	int c = 0;
	int len = 0;
	int file_where = 0;
	for (int i = 0; i < curr_target_count; i++) {	//копируется из структуры curr_target в массив str_curr_targets
		*(str_curr_targets + i) = (char*)malloc((int)strlen(((*(curr_target + i)).word)) * sizeof(char));
		if (*(str_curr_targets + i) == NULL) {
			exit(0);
		}
		strcpy(*(str_curr_targets + i), ((*(curr_target + i)).word));
	}
	//подсчёт слов для удаления с конца
	c = fgetc(input);
	while (c != EOF) {					//необходимо для подсчета слов которые надо удалить с конца
		for (int j = 0; j < curr_target_count; j++) {
			if ((char)c == *(*(str_curr_targets + j)) && ((*(curr_target + j)).mode == end || (*(curr_target + j)).mode == start)) {
				len = (int)strlen(*(str_curr_targets + j));
				str = (char*)calloc(len + 1, sizeof(char));
				fseek(input, -1, SEEK_CUR);
				file_where = ftell(input);
				fgets(str, len + 1, input);
				if (strcmp(*(str_curr_targets + j), str) == 0) {
					(*(curr_target + j)).count_max += 1;
				}
				else {
					fseek(input, file_where + 1, SEEK_SET);
				}
				free(str);
			}
		}
		c = fgetc(input);
	}
	rewind(input);		//перемещает укащатель в начало 
	// если в тексте <= слов чем нужно удалить, то удаление с конца заменяется на удаление всех слов;
	for (int i = 0; i < curr_target_count; i++) {
		if ((*(curr_target + i)).mode == end && (*(curr_target + i)).count_max <= (*(curr_target + i)).count) {
			(*(curr_target + i)).mode = all;
		}
	}
	//удаление слов и вывод 
	c = fgetc(input);
	while (c != EOF) {
		for (int j = 0; j < curr_target_count; j++) {
			if ((char)c == *(*(str_curr_targets + j))) {
				len = (int)strlen(*(str_curr_targets + j));
				str = (char*)calloc(len + 1, sizeof(char));
				fseek(input, -1, SEEK_CUR);
				file_where = ftell(input);
				fgets(str, len + 1, input);
				current_progress(file_size, file_where);
				if (strcmp(*(str_curr_targets + j), str) == 0) {
					switch ((*(curr_target + j)).mode) {
					case start: {
						if ((*(curr_target + j)).count > 0) {
							(*(curr_target + j)).count--;
							flag = 1;
							free(str);
							break;
						}
						else {
							fseek(input, file_where + 1, SEEK_SET);
							free(str);
							break;
						}
					}
					case end: {
						if ((*(curr_target + j)).count_max == (*(curr_target + j)).count) {
							(*(curr_target + j)).mode = all;
							flag = 1;
							free(str);
							break;
						}
						else {
							(*(curr_target + j)).count_max -= 1;
							free(str);
							fseek(input, file_where + 1, SEEK_SET);
							break;
						}
					}
					case change: {
						fputs((*(curr_target + j)).replacement, output);
						flag = 1;
						free(str);
						break;
					}
					default: {
						flag = 1;
						free(str);
						break;
					}
					}
				}
				else {
					free(str);
					fseek(input, file_where + 1, SEEK_SET);
				}
			}
		}
		if (flag == 0) {			//символ не надо будет удалять
			fputc((char)c, output);
		}
		flag = 0;
		c = fgetc(input);
	}
	system("cls");
	free(str_curr_targets);
}

void cmd_argv(int argc, char** argv, FILE** input, FILE** config, FILE** output) {
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "-input") == 0) {
			i++;
			*input = fopen(argv[i], "r");
		}
		if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "-config") == 0) {
			i++;
			*config = fopen(argv[i], "r");
		}
		if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "-output") == 0) {
			i++;
			*output = fopen(argv[i], "w");
		}
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0) {
			help();
		}
	}
}

int main(int argc, char** argv) {
	char* locale = setlocale(LC_ALL, "Russian");
	int lines = 0;
	clock_t begin, end;
	double time_spent = 0;
	struct target* cfg_instruction;
	FILE* input = fopen("input.txt", "r");
	FILE* config = fopen("config.txt", "r");
	FILE* output = fopen("output.txt", "w");

	cmd_argv(argc, argv, &input, &config, &output);

	if (input == NULL || config == NULL || output == NULL) {
		printf("Files hadn't found. Type [ -help ] to see instruction.\n");
		exit(0);
	}
	begin = clock();
	lines = cfgfile_lines_count(config);

	cfg_instruction = (struct target*)malloc(lines * sizeof(struct target));
	assert(cfg_instruction != NULL);			//проверка условия
	for (int i = 0; i < lines; i++) {
		*(cfg_instruction + i) = word_from_config(*(cfg_instruction + i), config); //заполняет структуру target словом из конфиг файла, где содержится само слово и как его надо удалить

		deleting(cfg_instruction, lines, input, output);

		free(cfg_instruction);
		fclose(config);
		fclose(input);
		end = clock();
		time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		printf("Time spent:%lf", time_spent);

		return 0;
	}
}
