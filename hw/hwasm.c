/*
 *    SPDX-FileCopyrightText: 2024 Matheus Pereira Dias <matheuspd07@gmail.com>
 *   
 *    SPDX-License-Identifier: GPL-3.0-or-later
 *
 *    This file is part of SYSeg, available at https://gitlab.com/monaco/syseg.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    char* label;
    uint16_t code;
    int num_bytes_opcode;
    int num_bytes;
} Opcode;

Opcode opcode_map[] = {
    {"mov_ah", 0xb4, 1, 2},
    {"mov_PTR_al", 0x8a84, 2, 4},
    {"mov_si", 0xbe00, 2, 3},
    {"cmp_al", 0x3c, 1, 2},
    {"je", 0x74, 1, 2},
    {"int", 0xcd, 1, 2},
    {"add_si", 0x83c6, 2, 3},
    {"jmp", 0xeb, 1, 2},
    {"hlt", 0xf4, 1, 1},
    {NULL, 0x00, 0, 0}
};

typedef struct {
    char label[20];
    uint16_t address;
    uint16_t to_resolve;
} Address;

typedef struct Addresses {
    Address data;
    struct Addresses* next;
} Addresses;

Addresses* createAddresses(const char* label, uint16_t address, int to_resolve) {
    Addresses* newAddresses = (Addresses*) malloc(sizeof(Addresses));
    if (newAddresses == NULL) {
        printf("Memory allocation error\n");
        return NULL;
    }
    strcpy(newAddresses->data.label, label);
    newAddresses->data.address = address;
    newAddresses->data.to_resolve = to_resolve;
    newAddresses->next = NULL;
    return newAddresses;
}

void addAddresses(Addresses** head, const char* label, uint16_t address, int to_resolve) {
    Addresses* newAddresses = createAddresses(label, address, to_resolve);
    if (newAddresses != NULL) {
        newAddresses->next = *head;
        *head = newAddresses;
    }
}

uint16_t isInList(Addresses* head, const char* label) {
    Addresses* current = head;
    while (current != NULL) {
        if (!strcmp(current->data.label, label)) {
            return current->data.address;
        }
        current = current->next;
    }
    return 0;
}

void printList(Addresses* head) {
    Addresses* current = head;
    while (current != NULL) {
        printf("Label: %s, Address: 0x%04x, To_resolve: %d\n", current->data.label, current->data.address, current->data.to_resolve);
        current = current->next;
    }
}

void updateAddress(Addresses* head, const char* label, uint16_t newAddress) {
    Addresses* current = head;
    while (current != NULL) {
        if (!strcmp(current->data.label, label)) {
            current->data.address = newAddress;
            return;
        }
        current = current->next;
    }
    printf("Label not found.\n");
}

void trim_left(char *str) {
    if (str == NULL) return;
    int index = 0, i = 0;
    /* Find the first non-whitespace character */
    while (str[index] == ' ' || str[index] == '\t') {
        index++;
    }
    /* Shift all characters to the beginning of the string */
    if (index > 0) {
        while (str[i + index] != '\0') {
            str[i] = str[i + index];
            i++;
        }
        str[i] = '\0';
    }
}

void trim_right(char *str) {
    if (str == NULL) return;
    int len = strlen(str);
    /* Find the last non-whitespace character */
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\n')) {
        len--;
    }
    str[len] = '\0';
}

void trim(char *str) {
    if (str == NULL) return;
    trim_right(str);
    trim_left(str);
}

void printHexBytes(uint16_t num, int x, FILE *out_file) {
    int i;

    for (i = x - 1; i >= 0; i--) {
        uint8_t byte = (num >> (8 * i)) & 0xFF;
        if(i == x-1 && num != 0x0 && (byte == 0x0 || byte == 0xff)) continue;
        fwrite(&byte, sizeof(uint8_t), 1, out_file);
    }
}

uint16_t diffHex(uint16_t num1, uint16_t num2) {
    uint16_t diff = num1 - num2;
    return diff;
}

void remove_comments(char *line) {
    char *comment = strchr(line, '#');
    if (comment) {
        *comment = '\0'; /* Terminate the string at the start of the comment */
    }
    /* Trim trailing whitespace after removing the comment */
    int len = strlen(line);
    while (len > 0 && (line[len - 1] == ' ' || line[len - 1] == '\t' || line[len - 1] == '\n')) {
        line[len - 1] = '\0';
        len--;
    }
}

int ends_with(const char *line, const char c) {
    int len = strlen(line);
    return (len > 0 && line[len - 1] == c);
}

void process_line(char* line, FILE* out_file, int *num_bytes, Addresses **add, Addresses **add_to_resolve) {
    if (line == NULL) return;
    char *label = NULL;
    if(line[0] == '.') {
        char temp[20];
        printf("%s\n", line);
        sscanf(line, "%s", temp);
        if(!strcmp(temp, ".code16") || !strcmp(temp, ".global")) return;
        else if(!strcmp(temp, ".string")){
            char str[256];
            sscanf(line, "%s \"%[^\"]\"", temp, str);
            int i;
            for(i = 0; i < strlen(str); i++) {
                fwrite(&str[i], sizeof(char), 1, out_file);
                (*num_bytes)++;
            }
        }
        else if(!strcmp(temp, ".fill")){
            char temp1[20], temp2[20];
            int i,j;
            sscanf(line, "%s %d %[^,],%[^,], %d", temp, &i, temp1, temp2, &j);
            j = (uint8_t)j;
            int k;
            printf("%d", *num_bytes);
            for(k = 0; k < 510 - *num_bytes; k++) {
                fwrite(&j, sizeof(uint8_t), 1, out_file);
            }            
        }
        else if(!strcmp(temp, ".word")){
            uint16_t i;
            sscanf(line, "%s 0x%hx", temp, &i);
            i = (i >> 8) | ((i & 0xFF) << 8);
            printHexBytes(i, 2, out_file);
        }
    }
    else if (ends_with(line, ':')) {
        line[strlen(line)-1] = '\0';
        label = malloc(strlen(line) + 1);
        if(label == NULL) {
            printf("Error malloc label.\n");
            return;
        }
        strcpy(label, line);
        if(!strcmp(label, "_start")) {
            addAddresses(add, label, 0x7c00, 0);
        }
        else {
            uint16_t res = *num_bytes + 0x7c00;
            addAddresses(add, label, res, 0);
            if(isInList(*add_to_resolve, label)) {
                updateAddress(*add_to_resolve, label, res);
            }
        }        
        free(label);
        return;
    }
    else {
        char instruction[20], arg1[20], arg2[20], temp1[20], temp2[20];

        int num_args = sscanf(line, "%s %[^,], %s", instruction, arg1, arg2);

        switch (num_args) {
            case 1:
                /* HALT */
                if(!strcmp(instruction, "hlt")) {
                    printHexBytes(opcode_map[8].code, opcode_map[8].num_bytes_opcode, out_file);
                    (*num_bytes) += opcode_map[8].num_bytes;
                }
                break;
            case 2:
                /* INT */
                if(!strcmp(instruction, "int")) {
                    char *ptr;
                    sscanf(arg1, "$0x%s", temp1);
                    int len = strlen(temp1);
                    uint16_t t = strtol(temp1, &ptr, 16);
                    printHexBytes(opcode_map[5].code, opcode_map[5].num_bytes_opcode, out_file);
                    printHexBytes(t, (len+1)/2, out_file);
                    (*num_bytes) += opcode_map[5].num_bytes;
                }
                /* JE */
                else if(!strcmp(instruction, "je")) {
                    /* Find Label */
                    uint16_t t = isInList(*add, arg1);
                    uint16_t pos = 0x7c00 + *num_bytes + opcode_map[4].num_bytes;
                    if (t) {
                        printHexBytes(opcode_map[4].code, opcode_map[4].num_bytes_opcode, out_file);
                        uint16_t diff = diffHex(t, pos);
                        printHexBytes(diff, 2, out_file);
                    }
                    else {
                        printHexBytes(opcode_map[4].code, opcode_map[4].num_bytes_opcode, out_file);
                        addAddresses(add_to_resolve, arg1, 0x1, *(num_bytes) + opcode_map[4].num_bytes_opcode);
                        uint8_t x = 0x0;
                        fwrite(&x, sizeof(uint8_t), 1, out_file);
                    }
                    (*num_bytes) += opcode_map[4].num_bytes;
                }
                /* JMP */
                else if(!strcmp(instruction, "jmp")) {
                    /* Find Label */
                    uint16_t t = isInList(*add, arg1);
                    uint16_t pos = 0x7c00 + *num_bytes + opcode_map[7].num_bytes;
                    if (t) {
                        printHexBytes(opcode_map[7].code, opcode_map[7].num_bytes_opcode, out_file);
                        uint16_t diff = diffHex(t, pos);
                        printHexBytes(diff, 2,out_file);
                    }
                    else {
                        printHexBytes(opcode_map[7].code, opcode_map[7].num_bytes_opcode, out_file);
                        addAddresses(add_to_resolve, arg1, 0x1, *(num_bytes) + opcode_map[7].num_bytes_opcode);
                        uint8_t x = 0x0;
                        fwrite(&x, sizeof(uint8_t), 1, out_file);
                    }
                    (*num_bytes) += opcode_map[7].num_bytes;
                }                
                break;
            case 3:
                /* MOV */
                if(!strcmp(instruction, "mov")) {               
                    sscanf(arg2, "%%%s", temp2);
                    /* MOV AH */  
                    if(!strcmp(temp2, "ah")) {
                        char *ptr;     
                        sscanf(arg1, "$0x%s", temp1);
                        int len = strlen(temp1);
                        uint16_t t = strtol(temp1, &ptr, 16);
                        printHexBytes(opcode_map[0].code, opcode_map[0].num_bytes_opcode,out_file);
                        printHexBytes(t, (len+1)/2, out_file);
                        (*num_bytes) += opcode_map[0].num_bytes;              
                    }
                    /* MOV AL PTR SI */
                    else if(!strcmp(temp2, "al")) {
                        sscanf(arg1, "%[^(](%%%s)", temp2, temp1);
                        /* Find Label (temp2) */
                        uint16_t t = isInList(*add, arg1);
                        if (t) {
                            printHexBytes(opcode_map[1].code, opcode_map[1].num_bytes_opcode, out_file);
                            printHexBytes(t, 2, out_file);
                        }
                        else {
                            printHexBytes(opcode_map[1].code, opcode_map[1].num_bytes_opcode, out_file);
                            addAddresses(add_to_resolve, temp2, 0x1, *(num_bytes) +  + opcode_map[1].num_bytes_opcode);
                            uint8_t x = 0x0;
                            fwrite(&x, sizeof(uint8_t), 1, out_file);
                            fwrite(&x, sizeof(uint8_t), 1, out_file);
                        }
                        (*num_bytes) += opcode_map[1].num_bytes;
                    }
                    /* MOV SI */
                    else if(!strcmp(temp2, "si")) {
                        char *ptr;     
                        sscanf(arg1, "$0x%s", temp1);
                        int len = strlen(temp1);
                        uint16_t t = strtol(temp1, &ptr, 16);
                        printHexBytes(opcode_map[2].code, opcode_map[2].num_bytes_opcode, out_file);
                        printHexBytes(t, (len+1)/2, out_file);
                        (*num_bytes) += opcode_map[2].num_bytes;       
                    }           
                }
                /* ADD SI */
                if(!strcmp(instruction, "add")) {
                    char *ptr;     
                    sscanf(arg1, "$0x%s", temp1);
                    int len = strlen(temp1);
                    uint16_t t = strtol(temp1, &ptr, 16);
                    printHexBytes(opcode_map[6].code, opcode_map[6].num_bytes_opcode, out_file);
                    printHexBytes(t, (len+1)/2, out_file);
                    (*num_bytes) += opcode_map[6].num_bytes;                
                }
                /* CMP */
                else if(!strcmp(instruction, "cmp")) {
                    char *ptr;          
                    sscanf(arg1, "$0x%s", temp1);
                    int len = strlen(temp1);
                    uint16_t t = strtol(temp1, &ptr, 16);
                    printHexBytes(opcode_map[3].code, opcode_map[3].num_bytes_opcode, out_file);
                    printHexBytes(t, (len+1)/2, out_file);
                    (*num_bytes) += opcode_map[3].num_bytes;                
                }
                break;
            default:
                printf("Invalid or unrecognized line.\n");
        }
    }
}

int main() {
    FILE *in_file = fopen("hw.S", "r");
    FILE *out_file = fopen("hw.bin", "w+b");
    char line[256];
    Addresses *add = NULL;
    Addresses *add_to_resolve = NULL;

    if (in_file == NULL || out_file == NULL) {
        perror("Error opening file");
        return 1;
    }

    int num_bytes = 0;

    while (fgets(line, sizeof(line), in_file)) {
        remove_comments(line);
        trim(line);
        if (strlen(line) == 0) continue;  /* Skip empty lines */
        process_line(line, out_file, &num_bytes, &add, &add_to_resolve);
    }

    /*printList(add);
    printList(add_to_resolve);*/

    while (add != NULL) {
        Addresses* temp = add;
        add = add->next;
        free(temp);
    }

    while (add_to_resolve != NULL) {
        Addresses* temp = add_to_resolve;
        if(!strcmp(add_to_resolve->data.label, "msg")) {
            /* FSEEK */
            uint16_t pos = add_to_resolve->data.to_resolve;
            if (fseek(out_file, pos, SEEK_SET) != 0) {
                fprintf(stderr, "Error when moving the cursor in the file.\n");
                fclose(out_file);
                exit(1);
            }
            uint16_t temp = add_to_resolve->data.address;
            temp = (temp >> 8) | ((temp & 0xFF) << 8);
            printHexBytes(temp, 2, out_file);
        }
        else{
            /* FSEEK */
            uint16_t pos = add_to_resolve->data.to_resolve;
            if (fseek(out_file, pos, SEEK_SET) != 0) {
                fprintf(stderr, "Error when moving the cursor in the file.\n");
                fclose(out_file);
                exit(1);
            }
            uint16_t temp = 0x7c00 + add_to_resolve->data.to_resolve + 1;
            uint16_t diff = diffHex(add_to_resolve->data.address, temp);
            printHexBytes(diff, 2, out_file);
        }
        
        add_to_resolve = add_to_resolve->next;
        free(temp);
    }

    fclose(in_file);
    fclose(out_file);
    return 0;
}
