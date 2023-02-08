#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char *output_path = "a.out";
int *input_path;
int input_count = 0;
unsigned long offset = 0;



ELF *out_elf;
Code_sec *out_code;
Name_sec *out_name;
Addr_sec *out_addr;

char *code_data = NULL;
unsigned long code_size = 0;

Name_sec_elem *names;
unsigned long names_count = 0;

Addr_sec_elem *addrs;
unsigned long addrs_count = 0;


void print_help();


int main(int argc, char *argv[]) {
	input_path = malloc(0);
	code_data = malloc(0);
	names = malloc(0);
	addrs = malloc(0);

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h\0") == 0 || strcmp(argv[i], "--help\0") == 0) {
			print_help();
		} else if (strcmp(argv[i], "-o\0") == 0 || strcmp(argv[i], "--output\0") == 0) {
			output_path = argv[++i];
		} else if (strcmp(argv[i], "-t\0") == 0 || strcmp(argv[i], "--help\0") == 0) {
			offset = scanf("%d", argv[++i]);
		} else {
			input_path = realloc(input_path, sizeof(input_path) + 8);
			input_path[input_count++] = i;
		}
	}

	for (int i = 0; i < input_count; i++) {
		FILE *f = fopen(argv[input_path[i]], "rb");

		if (f == NULL) {
			printf("Файл %s не найден!\n", argv[input_path[i]]);
			return 2;
		}


		ELF *elf = malloc(sizeof(ELF));
		fread(elf, sizeof(ELF), 1, f);

		if (strcmp(elf->magic, "ELF\0\0\0") != 0) {
			printf("Ошибка чтения заголовка!\n");
			fclose(f);
			return 1;
		}

		if (elf->type != TYPE_STAT) {
			printf("Файл %s не предназначен для статичного связывания!\n", argv[input_path[i]]);
			fclose(f);
			return 1;
		}

		unsigned long size;
		unsigned long ncount = 0;
		unsigned long acount = 0;


		Code_sec *code;
		Name_sec *name;
		Addr_sec *addr;

		if (elf->name_entry != 0) {
			fseek(f, elf->name_entry, SEEK_SET);

			fread(&size, sizeof(long), 1, f);

			ncount = (size - sizeof(long)) / sizeof(Name_sec_elem);

			name = malloc(size);
			name->size = size;
			fread(name->elems, sizeof(Name_sec_elem), ncount, f);

			names = realloc(names, (names_count + ncount) * sizeof(Name_sec_elem));

			memcpy(&names[names_count], name->elems, ncount * sizeof(Name_sec_elem));

			for (int j = 0; j < ncount; j++) {
				names[j + names_count].offset += code_size;

				for (int k = 0; k < names_count; k++) {
					if (strcmp(names[k].name, names[j + names_count].name) == 0) {
						printf("Ссылка на \"%s\" уже существует!\n", names[k].name);
						return 1;
					}
				}
			}

			names_count += ncount;

			free(name);
		}

		if (elf->addr_entry != 0) {
			fseek(f, elf->addr_entry, SEEK_SET);

			fread(&size, sizeof(long), 1, f);

			acount = (size - sizeof(long)) / sizeof(Addr_sec_elem);

			addr = malloc(size);
			addr->size = size;
			fread(addr->elems, sizeof(Addr_sec_elem), acount, f);

			addrs = realloc(addrs, (addrs_count + acount) * sizeof(Addr_sec_elem));

			memcpy(&addrs[addrs_count], addr->elems, acount * sizeof(Addr_sec_elem));

			for (int j = 0; j < acount; j++) {
				addrs[j + addrs_count].offset += code_size;
			}

			addrs_count += acount;

			free(addr);
		}

		if (elf->code_entry != 0) {
			fseek(f, elf->code_entry, SEEK_SET);

			fread(&size, sizeof(long), 1, f);

			code = malloc(size);
			code->size = size;
			unsigned long csize = size - sizeof(long);
			fread(code->data, csize, 1, f);

			code_data = realloc(code_data, code_size + csize);

			memcpy(&code_data[code_size], code->data, csize);

			code_size += csize;

			free(code);
		}

		fclose(f);

		free(elf);
	}


	#ifdef DEBUG
	putc('\n', stdout);

	for (int i = 0; i < code_size; i++) {
		printf("%0x ", code_data[i]);
	}

	putc('\n', stdout);
	putc('\n', stdout);

	for (int i = 0; i < names_count; i++) {
		printf("%s: %d\n", names[i].name, names[i].offset);
	}

	putc('\n', stdout);

	for (int i = 0; i < addrs_count; i++) {
		printf("%d: %s\n", addrs[i].offset, addrs[i].name);
	}
	#endif


	for (int i = 0; i < addrs_count; i++) {
		int nid = -1;

		for (int j = 0; j < names_count; j++) {
			if (strcmp(addrs[i].name, names[j].name) == 0) {
				nid = j;
				break;
			}
		}

		if (nid == -1) {
			printf("Неопределённая ссылка на \"%s\"\n", addrs[i].name);
			return 1;
		}

		for (int j = 0; j < 8; j++) {
			code_data[j + addrs[i].offset] = (names[nid].offset + offset) >> (j << 3) & 0xff;
		}
	}


	out_elf = calloc(sizeof(ELF), 1);
	out_code = malloc(sizeof(Code_sec) + code_size);
	out_name = malloc(sizeof(Name_sec) + names_count * sizeof(Name_sec_elem));
	out_addr = malloc(sizeof(Addr_sec) + addrs_count * sizeof(Addr_sec_elem));

	out_code->size = code_size + sizeof(long);
	memcpy(out_code->data, code_data, code_size);

	out_name->size = sizeof(long) + names_count * sizeof(Name_sec_elem);
	memcpy(out_name->elems, names, names_count * sizeof(Name_sec_elem));

	out_addr->size = sizeof(long) + addrs_count * sizeof(Addr_sec_elem);
	memcpy(out_addr->elems, addrs, addrs_count * sizeof(Addr_sec_elem));

	strcpy(out_elf->magic, "ELF\0\0\0");
	out_elf->version = 1;
	out_elf->type = TYPE_EXEC;
	out_elf->entry = 0;
	if (out_code->size > 8)
		out_elf->code_entry = sizeof(ELF);
	if (out_name->size > 8)
		out_elf->name_entry = out_elf->code_entry + out_code->size;
	if (out_addr->size > 8)
		out_elf->addr_entry = out_elf->name_entry + out_name->size;

	char start_find = 0;

	for (int i = 0; i < names_count; i++) {
		if (strcmp(names[i].name, "start") == 0) {
			out_elf->entry = names[i].offset;
			start_find = 1;
			break;
		}
	}

	if (start_find == 0) {
		printf("Функция \"start\" не найдена, точка входа установлена на 0\n");
	}


	FILE *f = fopen(output_path, "wb");

	if (f == NULL) {
		printf("Файл %s невозможно создать!\n", output_path);
		return 1;
	}

	fwrite(out_elf, sizeof(ELF), 1, f);
	if (out_code->size > 8)
		fwrite(out_code, sizeof(Code_sec) + code_size, 1, f);
	if (out_name->size > 8)
		fwrite(out_name, sizeof(Name_sec) + names_count * sizeof(Name_sec_elem), 1, f);
	if (out_addr->size > 8)
		fwrite(out_addr, sizeof(Addr_sec) + addrs_count * sizeof(Addr_sec_elem), 1, f);

	fclose(f);


	free(code_data);
	free(input_path);
	free(out_elf);
}


void print_help() {
	printf("Композтор для elf файлов.\n\t-h --help\tвывод этого сообщения.\n\t-o --output\tпуть к итоговому файлу.\n\t-t --offset\tсмещение кода.\n");
}
