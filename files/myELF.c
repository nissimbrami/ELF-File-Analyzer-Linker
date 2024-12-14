#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <elf.h>

#define FILENAME_SIZE 100

int debug_mode   = 0;
int fd1 = -1, fd2  = -1;
void *map_start1 = NULL, *map_start2 = NULL;
Elf32_Ehdr *header1 =  NULL, *header2 = NULL;

void debug_print(const char *msg) {
    if (debug_mode) {
        printf("DEBUG: %s\n", msg); 
    }
}

void *map_elf_file(const char *filename,  int *fd, Elf32_Ehdr **header) {
    *fd = open(filename, O_RDWR);
    if (*fd < 0) {
        perror("Error opening file ");
        return NULL;
    }
    
    off_t file_size = lseek(*fd, 0,  SEEK_END);
    lseek(*fd, 0, SEEK_SET);
    
    void *map_start = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, 0);
    if (map_start == MAP_FAILED) { 
        perror("Error mapping file ");
        close(*fd);
        *fd = -1;
        return NULL ;
    }

    *header  = (Elf32_Ehdr *)map_start;
    return  map_start;
}

void  examine_elf_file() {
    char filename[FILENAME_SIZE];
    printf(" enter ELF file name: ");
    scanf("%s" , filename);

    int fd;
    void *map_start ;
    Elf32_Ehdr *header;

    map_start =  map_elf_file(filename, &fd, &header);
    if (!map_start) return;

    // Check ELF magic number
    if (memcmp( header->e_ident, ELFMAG, SELFMAG) != 0) {
        printf("Error: Not a valid ELF file\n");
        munmap(map_start, lseek(fd, 0, SEEK_END));
        close(fd );
        return;
    }

    printf("magic num: %.3s\n" , header->e_ident + 1);  // Print ELF
    
    printf("data encoding: %s\n", 
           header->e_ident[EI_DATA] == ELFDATA2LSB ? "2's complement, little endian" :
           header->e_ident[EI_DATA] == ELFDATA2MSB ? "2's complement, big endian" : "Unknown");
    
    printf("entry point: 0x%x\n" , header->e_entry);
    printf("section header offset: %d\n", header->e_shoff);
    printf("number of section header entries: %d\n", header->e_shnum);
    printf("size of section header entry: %d\n", header->e_shentsize);
    printf("program header offset: %d\n" , header->e_phoff);
    printf("number of program header entries: %d\n", header->e_phnum);
    printf("size of program header entry: %d\n", header->e_phentsize);

    // Store the file information globally
    if (fd1 == -1 ) {
        fd1 = fd;
        map_start1 = map_start;
        header1 = header;
    } else if (fd2 ==  -1) {
        fd2 = fd;
        map_start2 =  map_start;
        header2 =  header;
    } else {
        printf("Error:  Cannot open more than two files.\n");
        munmap(map_start,  lseek(fd, 0, SEEK_END));
        close(fd);
    }
}

void print_section_names()  {
    char filename[FILENAME_SIZE];
    printf("Enter ELF file  name: ");
    scanf("%s",  filename);

    int fd;
    void *map_start;
    Elf32_Ehdr *header;

    map_start =  map_elf_file(filename, &fd, &header);
    if (!map_start) {
        printf("Error:  Failed to map ELF file.\n");
        return;
    }

    Elf32_Shdr *section_headers  = (Elf32_Shdr *)(map_start + header->e_shoff);
    const char *strtab = (char *)(map_start + section_headers[header->e_shstrndx].sh_offset);

    printf("File: %s\n", filename );
    for (int i = 0; i < header->e_shnum;  ++i) {
        const char *type_name;
        switch (section_headers[i].sh_type ) {
            case SHT_NULL: type_name = "NULL" ; break;
            case SHT_PROGBITS: type_name = "PROGBITS"; break;
            case SHT_SYMTAB: type_name = "SYMTAB" ; break;
            case SHT_STRTAB: type_name = "STRTAB"; break;
            case SHT_RELA: type_name = "RELA" ; break;
            case SHT_HASH: type_name = "HASH"; break;
            case SHT_DYNAMIC: type_name = "DYNAMIC"; break;
            case SHT_NOTE: type_name = "NOTE"; break;
            case SHT_NOBITS: type_name = "NOBITS"; break;
            case SHT_REL: type_name = "REL"; break;
            default: type_name = "UNKNOWN"; break;
        }

        printf("[%2d] %-17s 0x%08x %6d %6d   %s\n",
               i,
               &strtab[section_headers[i].sh_name],
               section_headers[i].sh_addr,
               section_headers[i].sh_offset, 
               section_headers[i].sh_size,
               type_name);
    }

    munmap(map_start, lseek(fd, 0, SEEK_END) );
    close(fd);
}

void print_symbols() {
    if (fd1 == -1 && fd2 == -1) {
        printf("Error: No ELF file loaded.\n" );
        return;
    }

    for (int file_num = 1; file_num <= 2;  file_num++) {
        void *map_start = (file_num == 1) ? map_start1 : map_start2;
        Elf32_Ehdr *header = (file_num == 1) ? header1 : header2;
        int fd = (file_num == 1) ? fd1 : fd2;

        if (fd == -1) continue;

        Elf32_Shdr *section_headers = (Elf32_Shdr  *)(map_start + header->e_shoff);
        
        Elf32_Shdr *symtab = NULL;
        Elf32_Shdr *strtab = NULL;

        for (int i = 0; i < header->e_shnum; i++ ) {
            if (section_headers[i].sh_type == SHT_SYMTAB) {
                symtab = &section_headers[i];
            } else if (section_headers[i].sh_type == SHT_STRTAB && 
                       strcmp((char*)(map_start + section_headers[header->e_shstrndx].sh_offset + section_headers[i].sh_name), ".strtab") == 0) {
                strtab = &section_headers[i];
            }
        }

        if (!symtab || !strtab) {
            printf("Error: Symbol table or string table not found in file  %d.\n", file_num);
            continue;
        }

        Elf32_Sym *symbols = (Elf32_Sym *)(map_start + symtab->sh_offset);
        const char *str_table = (const char *)(map_start + strtab->sh_offset);
        int symbol_count = symtab->sh_size / sizeof(Elf32_Sym);

        printf("file %s\n", (file_num == 1) ? "F1a.o" : "F2a.o" );  // Replace with actual filenames
        for (int i = 0; i < symbol_count; i++) {
            const char *section_name = "UND";
            if (symbols[i].st_shndx < header->e_shnum && symbols[i].st_shndx != SHN_UNDEF) {
                section_name = (char*)(map_start + section_headers[header->e_shstrndx].sh_offset + section_headers[symbols[i].st_shndx].sh_name);
            } else if (symbols[i].st_shndx == SHN_ABS) {
                section_name = "ABS";
            }

            printf("[%2d] 0x%08x %3d %-8s %s\n ", 
                   i, 
                   symbols[i].st_value, 
                   symbols[i].st_shndx,
                   section_name,
                   &str_table[symbols[i].st_name]);
        }
    }
}

void check_files_for_merge() {
    if (fd1 == -1 || fd2 == -1) {
        printf("Error: Two ELF files need to be open for this operation.\n");
        return;
    }

    Elf32_Shdr *section_headers1 = (Elf32_Shdr *)(map_start1 + header1->e_shoff);
    Elf32_Shdr *section_headers2 = (Elf32_Shdr *)(map_start2 + header2->e_shoff);

    Elf32_Shdr *symtab1 = NULL;
    Elf32_Shdr *symtab2 = NULL;

    for (int i = 0; i < header1->e_shnum; ++i) {
        if (section_headers1[i].sh_type == SHT_SYMTAB) {
            symtab1 = &section_headers1[i];
            break;
        }
    }

    for (int i = 0; i < header2->e_shnum; ++i) {
        if (section_headers2[i].sh_type == SHT_SYMTAB) {
            symtab2 = &section_headers2[i];
            break;
        }
    }

    if (!symtab1 || !symtab2) {
        printf("Error: One or both files do not contain a symbol table.\n");
        return;
    }

    Elf32_Sym *symbols1 = (Elf32_Sym *)(map_start1 + symtab1->sh_offset);
    Elf32_Sym *symbols2 = (Elf32_Sym *)(map_start2 + symtab2->sh_offset);

    const char *strtab1 = (char *)(map_start1 + section_headers1[symtab1->sh_link].sh_offset);
    const char *strtab2 = (char *)(map_start2 + section_headers2[symtab2->sh_link].sh_offset);

    int sym_count1 = symtab1->sh_size / sizeof(Elf32_Sym);
    int sym_count2 = symtab2->sh_size / sizeof(Elf32_Sym);

    int errors = 0;

    // Check symbols in first file
    for (int i = 1; i < sym_count1; ++i) {
        if (ELF32_ST_BIND(symbols1[i].st_info) == STB_LOCAL) continue;

        const char *sym_name = &strtab1[symbols1[i].st_name];
        if (strlen(sym_name) == 0) continue;

        if (symbols1[i].st_shndx == SHN_UNDEF) {
            // Symbol undefined in first file
            int found = 0;
            for (int j = 1; j < sym_count2; ++j) {
                if (strcmp(sym_name, &strtab2[symbols2[j].st_name]) == 0) {
                    if (symbols2[j].st_shndx != SHN_UNDEF) {
                        found = 1;
                        break;
                    }
                }
            }
            if (!found) {
                printf("Symbol %s undefined.\n", sym_name);
                errors++;
            }
        } else {
            // Symbol defined in first file
            for (int j = 1; j < sym_count2; ++j) {
                if (strcmp(sym_name, &strtab2[symbols2[j].st_name]) == 0) {
                    if (symbols2[j].st_shndx != SHN_UNDEF) {
                        printf("Symbol %s multiply defined.\n", sym_name);
                        errors++;
                    }
                    break;
                }
            }
        }
    }

    if (errors == 0) {
        printf("Files can be merged :-)\n");
    } else {
        printf("Found %d issues with files preventing merge.\n", errors);
    }
}

void merge_elf_files() {
    if (fd1 == -1 || fd2 == -1 ||  map_start1 == NULL || map_start2 == NULL || header1 == NULL || header2 == NULL) {
        printf("Error: Two valid ELF files need to be open for this operation.\n");
        return;
    }

    int fd_out = open("out.ro",  O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out == -1) {
        perror("Error creating output file");
        return;
    }

    Elf32_Ehdr new_header = *header1;
    if (write(fd_out, &new_header, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        perror("Error writing ELF header");
        close(fd_out);
        return;
    }

    Elf32_Shdr *new_section_headers =  malloc(header1->e_shnum * sizeof(Elf32_Shdr));
    if (new_section_headers == NULL) {
        perror("Error allocating memory for new section headers");
        close(fd_out);
        return;
    }
    memcpy(new_section_headers, (void*)map_start1 + header1->e_shoff, header1->e_shnum * sizeof(Elf32_Shdr));

    off_t current_offset = sizeof(Elf32_Ehdr);
    for (int i = 0; i < header1->e_shnum; i++) {
        Elf32_Shdr *sh1 =  &new_section_headers[i];
        if (sh1->sh_name >= new_section_headers[header1->e_shstrndx].sh_size) {
            printf("Error: Invalid section name offset\n");
            close(fd_out);
            free(new_section_headers);
            return;
        }
        const char *name = (char*)map_start1 + new_section_headers[header1->e_shstrndx].sh_offset + sh1->sh_name;

        if (strcmp(name, ".text") == 0 || strcmp(name, ".data") == 0 || strcmp(name, ".rodata") == 0) {
            if (write( fd_out, map_start1 + sh1->sh_offset, sh1->sh_size) != sh1->sh_size) {
                perror("Error writing section");
                close(fd_out);
                free(new_section_headers);
                return;
            }
            
            Elf32_Shdr  *section_headers2 = (Elf32_Shdr*)(map_start2 + header2->e_shoff);
            for (int j = 0; j < header2->e_shnum; j++) {
                Elf32_Shdr *sh2 = &section_headers2[j];
                if (sh2->sh_name >= section_headers2[header2->e_shstrndx].sh_size) {
                    printf("Error: Invalid section name offset in second file\n");
                    close(fd_out);
                    free(new_section_headers);
                    return;
                }
                const char  *name2 = (char*)map_start2 + section_headers2[header2->e_shstrndx].sh_offset + sh2->sh_name;
                if (strcmp(name, name2) == 0) {
                    if (write(fd_out, map_start2 + sh2->sh_offset, sh2->sh_size) != sh2->sh_size) {
                        perror("Error writing section from second file");
                        close(fd_out);
                        free(new_section_headers);
                        return;
                    }
                    sh1->sh_size += sh2->sh_size;
                    break;
                }
            }
        } else {
            if (write( fd_out, map_start1 + sh1->sh_offset, sh1->sh_size) != sh1->sh_size) {
                perror("Error writing section");
                close(fd_out);
                free(new_section_headers);
                return;
            }
        }

        sh1->sh_offset  = current_offset;
        current_offset += sh1->sh_size;
    }

    Elf32_Shdr *symtab1 = NULL, *symtab2 = NULL;
    for (int i = 0; i < header1->e_shnum; i++) {
        if (new_section_headers[i].sh_type == SHT_SYMTAB) {
            symtab1 = &new_section_headers[i];
            break;
        }
    }

    Elf32_Shdr *section_headers2  = (Elf32_Shdr*)(map_start2 + header2->e_shoff);
    for (int i = 0; i < header2->e_shnum; i++) {
        if (section_headers2[i].sh_type == SHT_SYMTAB) {
            symtab2 = &section_headers2[i];
            break;
        }
    }

    if (symtab1 && symtab2) {
        Elf32_Sym *symbols1 = ( Elf32_Sym*)(map_start1 + symtab1->sh_offset);
        Elf32_Sym *symbols2 = (Elf32_Sym*)(map_start2 + symtab2->sh_offset);
        int sym_count1 = symtab1->sh_size / sizeof(Elf32_Sym);
        int sym_count2 = symtab2->sh_size / sizeof(Elf32_Sym);

        for (int i = 1; i < sym_count1; i++) {
            if (symbols1[i].st_shndx == SHN_UNDEF) {
                for (int j = 1; j < sym_count2; j++) {
                    if (symbols1[i].st_name >= new_section_headers[symtab1->sh_link].sh_size ||
                        symbols2[j].st_name >= section_headers2[symtab2->sh_link].sh_size) {
                        printf("Error: Invalid symbol name offset\n");
                        close(fd_out);
                        free(new_section_headers);
                        return;
                    }
                    if (strcmp(( char*)map_start1 + new_section_headers[symtab1->sh_link].sh_offset + symbols1[i].st_name,
                               (char*)map_start2 + section_headers2[symtab2->sh_link].sh_offset + symbols2[j].st_name) == 0) {
                        if (symbols2[j].st_shndx != SHN_UNDEF) {
                            symbols1[i] = symbols2[j];
                            break;
                        }
                    }
                }
            }
        }
        if (write(fd_out, symbols1 , symtab1->sh_size) != symtab1->sh_size) {
            perror("Error writing symbol table");
            close(fd_out);
            free(new_section_headers);
            return;
        }
    }

    new_header.e_shoff = current_offset;
    if (write(fd_out, new_section_headers,  header1->e_shnum * sizeof(Elf32_Shdr)) != header1->e_shnum * sizeof(Elf32_Shdr)) {
        perror("Error writing section headers");
        close(fd_out);
        free(new_section_headers);
        return;
    }

    lseek(fd_out, 0, SEEK_SET);
    if (write(fd_out, &new_header,  sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        perror("Error updating ELF header");
        close(fd_out);
        free(new_section_headers);
        return;
    }

    close(fd_out);
    free(new_section_headers  );

    printf("Merged ELF file created: out.ro\n");
}

int main() {
    int choice;
    while (1) { 
        printf("Choose action:\n");
        printf("0-Toggle Debug Mode\n");
        printf("1-Examine ELF File\n");
        printf("2-Print Section Names\n");
        printf("3-Print Symbols\n");
        printf("4-Check Files for Merge\n");
        printf("5-Merge ELF Files\n");
        printf("6-Quit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 0:
                debug_mode = !debug_mode;
                printf("Debug mode %s\n" , debug_mode ? "on" : "off");
                break;
            case 1:
                examine_elf_file();
                break;
            case 2:
                print_section_names ();
                break;
            case 3:
                print_symbols();
                break;
            case 4:
                check_files_for_merge();
                break;
            case 5:
                merge_elf_files();
                break;
            case 6:
                if (map_start1) munmap (map_start1, lseek(fd1, 0, SEEK_END));
                if (map_start2) munmap(map_start2, lseek(fd2, 0, SEEK_END));
                if (fd1 != -1) close(fd1);
                if (fd2 != -1) close(fd2);
                exit(0);
            default:
                printf("Invalid choice.\n");
                break;
        }
    }
    return 0;
}
