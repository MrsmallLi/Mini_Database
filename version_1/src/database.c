/*************************************************************************
	> File Name: database.c
	> Author: 
	> Mail: 
	> Created Time: Thu 04 Apr 2024 03:01:15 PM CST
 ************************************************************************/

#include <stdio.h>
#include <database.h>
#include <string.h>
#include <stdlib.h>

struct Database db;

__attribute__((constructor)) 
void init_db() {
    db.table      = NULL;
    db.table_file = NULL;
    db.table_name = NULL;
    db.head.next  = NULL;
    return ;
}

TableInfo tables[100];
int table_cnt = 0;

void register_table(const char *table_name, InitTable_T init_table) {
    tables[table_cnt].table_name = table_name;
    tables[table_cnt].init_table = init_table;
    table_cnt += 1;
    return ;
}

static T_data *getNewTableData(void *data, long offset) {
    struct table_data *p = (struct table_data *)malloc(sizeof(struct table_data));
    p->data = (void *)malloc(db.getDataSize());
    memcpy(p->data, data, db.getDataSize());
    p->offset = offset;
    p->next = NULL;
    return p;
}

static void load_table_data() {
    char buff[db.getDataSize()];
    T_data *p = &(db.head);
    int data_cnt = 0;
    while (1) {
        long offset = ftell(db.table);
        if (fread(buff, db.getDataSize(), 1, db.table) == 0) break;
        p->next = getNewTableData(buff, offset);
        p = p->next;
        data_cnt += 1;
    }
    printf("load data success : %d item\n", data_cnt);
    return;
}

static void destroyTbaleData(struct table_data *p) {
    free(p->data);
    free(p);
    return ;
}

static void clear_table_data() {
    struct table_data *p = db.head.next, *q;
    while (p) {
        q = p->next;
        destroyTbaleData(p);
        p = q;
    }
    return ;
}

static void close_table() {
    clear_table_data();
    if (db.table == NULL) return ;
    fclose(db.table);
    return ;
}


static void open_table() {
    db.table = fopen(db.table_file, "rb+"); 
    if (db.table == NULL) {
        printf("can't open file : %s\n", db.table_file);
        exit(1);
    }
    load_table_data();
    fclose(db.table);
    return ;
}

static OP_TYPE choose_table() {
    printf("choose table\n");
    for (int i = 0; i < table_cnt; i++) {
        printf("%d : %s\n", i, tables[i].table_name);
    }
    printf("%d : quti\n", table_cnt);
    int x;
    do {
        printf("input Your choose : ");
        scanf("%d", &x);
        if (x < 0 || x > table_cnt) printf("input error, please input 1 or 2\n");
    } while (x < 0 || x > table_cnt);
    if (x < table_cnt)  {
        close_table();
        tables[x].init_table(&db);
        open_table();
        return TABLE_USAGE;
    }
    else return OP_END;
}

static OP_TYPE table_usage() {
    printf("1 : list %s\n", db.table_name);
    printf("2 : add an item to %s\n", db.table_name);
    printf("3 : modify an item in %s\n", db.table_name);
    printf("4 : delete an item from %s\n", db.table_name);
    printf("5 : back\n");
    int x;
    do {
        printf("input Your choose : ");
        scanf("%d", &x);
    } while (x < 1 || x > 5);
    switch (x) {
        case 1: return LIST_TABLE;
        case 2: return ADD_TABLE;
        case 3: return MODIFY_TABLE;
        case 4: return DELETE_TABLE;
        case 5: return CHOOSE_TABLE;
    }
    return TABLE_USAGE;
}

static void printTableHeader() {
    int len = 0;
    len += printf("%5s|", "id");
    char frm[100];
    for (int i = 0; i < db.header_cnt; i++) {
        sprintf(frm, "%%%ds|", db.header_len[i]);
        len += printf(frm, db.header_name[i]);
    }
    printf("\n");
    for (int i = 0; i < len; i++) printf("-");
    printf("\n");
    return ;
}

static int __list_table() {
    T_data *p = db.head.next;
    int id = 0;
    printTableHeader();
    while (p) {
        printf("%5d|", id++);
        db.printData(p->data);
        p = p->next;
    }
    return id;
}

static OP_TYPE list_table() {
    __list_table();
    return TABLE_USAGE;
}

static long add_one_table_data(void *buff) {
    fseek(db.table, 0, SEEK_END);
    long offset = ftell(db.table);
    struct table_data *p = &(db.head);
    while (p->next) p = p->next;
    p->next = getNewTableData(buff, offset);
    fwrite(buff, db.getDataSize(), 1, db.table);
    fflush(db.table);
    return offset;
}

static OP_TYPE add_table() {
    printf("add new item : (");
    for (int i = 0; i < db.header_cnt; i++) {
        i && printf(", ");
        printf("%s", db.header_name[i]);
    }
    printf(")\n");
    printf("input : ");
    char buff[db.getDataSize()];
    db.scanData(buff);
    add_one_table_data(buff);
    printf("add one item to %s table : success\n", db.table_name);
    return TABLE_USAGE;
}

static void modify_one_table_data(void *buff, int id) {
    struct table_data *p = db.head.next; 
    for (int i = 0; i < id; i++) p = p->next;
    memcpy(p->data, buff, db.getDataSize());
    fseek(db.table, p->offset, SEEK_SET);
    fwrite(buff, db.getDataSize(), 1, db.table);
    fflush(db.table);
    printf("modify one item from %s table : success\n", db.table_name);
    return ;
}

static OP_TYPE modify_table() {
    int n = __list_table();
    int id;
    do {
        printf("modify id (input %d back): ", n);
        scanf("%d", &id);
    } while (id < 0 || id > n);
    if (id == n) return TABLE_USAGE;
    printf("modify item (id = %d) : (", id);
    for (int i = 0; i < db.header_cnt; i++) {
        i && printf(", ");
        printf("%s", db.header_name[i]);
    }
    printf(")\n");
    printf("input new item: ");
    char buff[db.getDataSize()];
    db.scanData(buff);
    modify_one_table_data(buff, id);
    return TABLE_USAGE;
}

static void clearTableData() {
    fclose(db.table);
    db.table = fopen(db.table_file, "w");
    fclose(db.table);
    db.table = fopen(db.table_file, "rb+");
    struct table_data *p = db.head.next, *q;
    while (p) {
        p = p->next;
        destroyTbaleData(p);
        p = q;
    }
    db.head.next = NULL;
    return ;
}

static void restoreTableData() {
    struct table_data *p = db.head.next, *q;
    db.head.next = NULL;
    clearTableData();
    while (p) {
        q = p->next;
        add_one_table_data(p->data);
        destroyTbaleData(p);
        p = q;
    }
    return ;
}

static OP_TYPE delete_table() {
    int n = __list_table();
    int id;
    do {
        printf("delete id (input %d back): ", n);
        scanf("%d", &id);
    } while (id < 0 || id > n);
    if (id == n) return TABLE_USAGE;
    struct table_data *p = &(db.head), *q;
    for (int i = 0; i < id; i++) p = p->next;
    q = p->next;
    p->next = q->next;
    destroyTbaleData(q);
    restoreTableData();
    return TABLE_USAGE;
}
OP_TYPE run(OP_TYPE status) {
    switch (status)  {
        case CHOOSE_TABLE : {
            return choose_table();
        } break;
        case TABLE_USAGE : {
            return table_usage();
        } break;
        case LIST_TABLE : {
            return list_table();
        } break;
        case ADD_TABLE : {
            return add_table();
        } break;
        case MODIFY_TABLE : {
            return modify_table();
        } break;
        case DELETE_TABLE : {
            return delete_table();
        } break;
        default : {
            printf("unkown status : %d\n", status);
        } break;
    }
    return OP_END;
}

void run_database() {
    OP_TYPE status = CHOOSE_TABLE;
    while (1) {
        status = run(status);
        if (status == OP_END) break;
    }
    printf("Thanks your used~\n");
    return ;
}



