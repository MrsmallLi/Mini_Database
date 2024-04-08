/*************************************************************************
	> File Name: students_table.c
	> Author: 
	> Mail: 
	> Created Time: Sat 06 Apr 2024 10:05:06 AM CST
 ************************************************************************/

#include <stdio.h>
#include <database.h>


static const char *table_name = "student table";
static const char *table_file = "./data/student_data.dat";
static const char *header_name[] = {"name", "age", "class", "height"};
static int header_len[] = {10, 6, 6, 6};
static void init_table(Database *);
static size_t getDataSize();
static void printData(void *);
static void scanData(void *);

typedef struct Student {
    char name[20];
    int age;
    int class;
    double height;
} table_data;

__attribute__((constructor)) 
static void __register_table() {
    register_table(table_name, init_table);
    return ;
}

static void init_table(Database *db) {
    db->table_name = table_name; 
    db->table_file = table_file;
    db->getDataSize = getDataSize;
    db->printData = printData;
    db->scanData = scanData;
    db->header_name = header_name;
    db->header_len = header_len;
    db->header_cnt = sizeof(header_len) / sizeof(header_len[0]);
    return ;
}

static void printData(void *__data) {
    table_data *data = (table_data *) __data; 
    char  frm[100];
    sprintf(frm, "%%%ds|%%%dd|%%%dd|%%%d.2lf|\n", header_len[0], header_len[1], header_len[2], header_len[3]);
    printf(frm, data->name, data->age, data->class, data->height);
}

size_t getDataSize() {
    return sizeof(table_data); 
}

static void scanData(void *__data) {
    table_data *data = (table_data*)(__data);
    scanf("%s%d%d%lf", 
            data->name, &data->age, 
            &data->class, &data->height
    );
    return ;
}
